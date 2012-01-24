#include "stdafx.h"
#include "iSip/undeferrno.h"

#include "UserAgentRegistrator.h"
#include "ConversationProfile.h"
#include "SipUtils.h"
#include "ISipAccessPoint.h"

#include "Utils/ErrorsSubsystem.h"

#include "Utils/AtomicTypes.h"
#include "Utils/IntToString.h"

#include <resip/dum/AppDialogSet.hxx>
#include <resip/dum/DialogUsageManager.hxx>
#include <resip/dum/ClientRegistration.hxx>


namespace
{
	std::string GetRegistrationName()
	{
		static Utils::AtomicInt GRegistrationCount;
		return "Item_" + Utils::IntToString(GRegistrationCount.FetchAndAdd(1));
	}
}

namespace
{
	class IRegistrationUser : public Utils::IBasicInterface
	{
	public:

		virtual void StateChanged(iSip::SipRegistrationState::Value state,
			const std::string &event,
			const resip::SipMessage *msg = 0) = 0;
	};


	class Registration : boost::noncopyable,
		public resip::AppDialogSet,
		public resip::ClientRegistrationHandler
	{
		typedef Registration T;

	public:

		Registration(
			Utils::WeakRef<IRegistrationUser&> user,
			resip::DialogUsageManager& dum,
			boost::shared_ptr<iSip::ConversationProfile> conversationProfile) :
			resip::AppDialogSet(dum),
			m_user(user),
			m_ended(false)
		{
			// Register new profile
			ESS_ASSERT (conversationProfile->GetRegistrationProfile().EnableRegistration());

			ESS_ASSERT (conversationProfile->UserProfile().getDefaultRegistrationTime() != 0);

			m_state = iSip::SipRegistrationState::RsProgress;

			dum.send(mDum.makeRegistration(conversationProfile->UserProfile().getDefaultFrom(),
				conversationProfile->UserProfilePtr(), this));
		}

		// -------------------------------------------------------------------------------------

		Utils::WeakRef<Registration&> SelfRef()
		{
			return m_selfHost.Create<Registration&>(*this);
		}

		// -------------------------------------------------------------------------------------

		const resip::NameAddrs& getContactAddresses() const
		{
			static resip::NameAddrs empty;

			if (!m_registrationHandle.isValid()) return empty;

			return m_registrationHandle->allContacts();  // .slg. note:  myContacts is not sufficient, since they do not contain the stack populated transport address
		}

		// -------------------------------------------------------------------------------------

		iSip::SipRegistrationState::Value GetState() const
		{
			return m_state;
		}

		// -------------------------------------------------------------------------------------

		void RemoveRegistration()
		{
			end();
		}

	// AppDialogSet
	private:

		void end()
		{
			if (m_ended) return;

			m_ended = true;

			try
			{
				// If ended - then just shutdown registration - likely due to shutdown
				if (m_registrationHandle.isValid()) m_registrationHandle->end();

				AppDialogSet::end();
			}
			catch(resip::BaseException&)
			{
				// If end() call is nested - it will throw - catch here so that processing continues normally
			}
		}

	// ClientRegistrationHandler
	public:
		
		void onSuccess(resip::ClientRegistrationHandle h,
			const resip::SipMessage &response)
		{
			SetNewState(h, iSip::SipRegistrationState::RsSuccess, "", &response);
		}

		// -------------------------------------------------------------------------------------

		void onFailure(resip::ClientRegistrationHandle h, const resip::SipMessage &response)
		{
			SetNewState(h, iSip::SipRegistrationState::RsProgress, "", &response);
		}

		// -------------------------------------------------------------------------------------

		void onRemoved(resip::ClientRegistrationHandle h, const resip::SipMessage &response)
		{
			SetNewState(h, iSip::SipRegistrationState::RsRemoved, "", &response);
		}

		// -------------------------------------------------------------------------------------

		int onRequestRetry(resip::ClientRegistrationHandle h, int retryMinimum, const resip::SipMessage &msg)
		{
			return -1;  // Let Profile retry setting take effect
		}
	
	private:

		void SetNewState(resip::ClientRegistrationHandle h,
			iSip::SipRegistrationState::Value state,
			const std::string &event,
			const resip::SipMessage *msg)
		{
			
			m_state = state;
			
			if (!m_user.Empty()) m_user.Value().StateChanged(m_state, event, msg);

			if (!m_ended)
			{
				m_registrationHandle = h;				
				return;
			}

			try
			{
				// If we don't have a handle - then just shutdown registration - likely due to shutdown
				h->end();
			}
			catch(resip::BaseException&)
			{
				// If end() call is nested - it will throw - catch here so that processing continues normally
			}
		}

	private:
		Utils::WeakRefHost m_selfHost;

		Utils::WeakRef<IRegistrationUser&> m_user;

		bool m_ended;

		iSip::SipRegistrationState::Value m_state;
        mutable resip::ClientRegistrationHandle m_registrationHandle;
	};

}

namespace iSip
{

	class UserAgentRegistrator::Item : boost::noncopyable,
		IRegistrationUser,
		public IRegistrationItem
	{
		typedef Item T;

	public:

		Item(
			boost::function<void (T*, bool)> registrator,
			iLogW::ILogSessionCreator &logCreator,
			resip::DialogUsageManager& dum,
			const std::string &accessPointName,
			ISipAccessPointEventsExt &user,
			boost::shared_ptr<ConversationProfile> conversationProfile) :
			m_registrator(registrator),
			m_log(logCreator.CreateSession(GetRegistrationName(), true)),
			m_infoTag(m_log->RegisterRecordKind(L"Info", true)),
			m_user(user),
			m_conversationProfile(conversationProfile)
		{
			ESS_ASSERT (!m_registrator.empty());

			m_registrator(this, true);

			// Register new profile
			if (m_conversationProfile->GetRegistrationProfile().EnableRegistration())
			{
				ESS_ASSERT (m_conversationProfile->UserProfile().getDefaultRegistrationTime() != 0);

				Registration *registration(new Registration(
					m_selfHost.Create<IRegistrationUser&>(*this),
					dum,
					conversationProfile));

				m_registration = registration->SelfRef();
			}

			StateChanged(GetState(),
				"Created. Access point: '" + accessPointName +
				"' From: " +
				iSip::SipUtils::ToString(conversationProfile->UserProfile().getDefaultFrom()));
		}

		// -------------------------------------------------------------------------------------

		~Item()
		{
			if (!m_registration.Empty()) m_registration.Value().RemoveRegistration();
		}

		// -------------------------------------------------------------------------------------

		Utils::WeakRef<Item&> SelfRef()
		{
			return m_selfHost.Create<Item&>(*this);
		}

		Utils::WeakRef<IRegistrationItem&> RegistrationRef()
		{
			return m_selfHost.Create<IRegistrationItem&>(*this);
		}

		// -------------------------------------------------------------------------------------

		ISipAccessPointEvents* CallReceiver() {	return &m_user;	}

		// -------------------------------------------------------------------------------------

		boost::shared_ptr<ConversationProfile> GetConversationProfile() const
		{
			return m_conversationProfile;
		}

		// -------------------------------------------------------------------------------------

		const resip::NameAddrs& getContactAddresses() const
		{
			static resip::NameAddrs empty;

			if (m_registration.Empty()) return empty;

			return m_registration.Value().getContactAddresses();  // .slg. note:  myContacts is not sufficient, since they do not contain the stack populated transport address
		}


		// IRegistrationItem
	public:

		SipRegistrationState::Value GetState() const
		{
			return m_registration.Empty() ?
				SipRegistrationState::RsNoRegistration :
				m_registration.Value().GetState();
		}

		void RemoveRegistration()
		{
			if (!m_registration.Empty()) m_registration.Value().RemoveRegistration();

			m_registrator(this, false);
		}

	//
	private:

		void StateChanged(iSip::SipRegistrationState::Value state,
				const std::string &event,
				const resip::SipMessage *msg = 0)
		{
			if(m_log->LogActive(m_infoTag))
			{
				*m_log << m_infoTag
					<< "State: " << SipRegistrationState::ToString(state)
					<< "; "
					<< event;

				if (msg != 0) *m_log << MsgToString(*msg);
				*m_log << iLogW::EndRecord;
			}

			m_user.RegistrationState(state, event);
		}

		// -------------------------------------------------------------------------------------

		std::string MsgToString(const resip::SipMessage &msg)
		{
			return iSip::SipUtils::ToString(m_conversationProfile->Misc.ResipLogMode(), msg);
		}

	private:
		boost::function<void (T*, bool)> m_registrator;
		Utils::WeakRefHost m_selfHost;

		boost::scoped_ptr<iLogW::LogSession>  m_log;
		iLogW::LogRecordTag m_infoTag;
		Utils::WeakRef<Registration&> m_registration;
		ISipAccessPointEventsExt &m_user;
		boost::shared_ptr<ConversationProfile> m_conversationProfile;
	};

}

namespace iSip
{
	UserAgentRegistrator::UserAgentRegistrator(
		resip::DialogUsageManager& dum,
		iLogW::LogSession  &logSession,
		boost::shared_ptr<ConversationProfile> defaultConversationProfile) :
		m_dum(dum),
		m_registrationHandler(dum),
		m_log(logSession),
		m_infoTag(m_log.RegisterRecordKind(L"RegistrationInfo", true)),
		m_defaultConversationProfile(defaultConversationProfile)
	{
		ESS_ASSERT(m_defaultConversationProfile != 0);
	}

	// -------------------------------------------------------------------------------------

	UserAgentRegistrator::~UserAgentRegistrator()
	{
	}

	// -------------------------------------------------------------------------------------

	void UserAgentRegistrator::Shutdown()
	{
		m_items.clear();
	}

	// -------------------------------------------------------------------------------------

	boost::shared_ptr<ConversationProfile> UserAgentRegistrator::
		GetIncomingConversationProfile(const resip::SipMessage& msg) const
	{
		ESS_ASSERT(msg.isRequest());

		// Examine the sip message, and select the most appropriate conversation profile

		// Check if request uri matches registration contact
		const resip::Uri& requestUri = msg.header(resip::h_RequestLine).uri();

		for (ListItem::const_iterator i = m_items.begin();
			i != m_items.end(); ++i)
		{
			const resip::NameAddrs &contacts = i->getContactAddresses();

			for(resip::NameAddrs::const_iterator naIt = contacts.begin(); naIt != contacts.end(); naIt++)
			{
				if(m_log.LogActive(m_infoTag))
				{
					m_log << m_infoTag << "GetIncomingConversationProfile: comparing requestUri = "
						<< SipUtils::ToString(requestUri)
						<< " to contactUri = "
						<< SipUtils::ToString((*naIt).uri())
						<< iLogW::EndRecord;
				}
				
				if (naIt->uri() == requestUri) return i->GetConversationProfile();
			}
		}

		// Check if To header matches default from
		resip::Data toAor = msg.header(resip::h_To).uri().getAor();

		for (ListItem::const_iterator i = m_items.begin();
			i != m_items.end(); ++i)
		{
			boost::shared_ptr<ConversationProfile> profile(i->GetConversationProfile());
			
			if (isEqualNoCase(toAor, profile->UserProfile().getDefaultFrom().uri().getAor())) return profile;
		}

		// If can't find any matches, then return the default outgoing profile
		if(m_log.LogActive(m_infoTag))
		{
			m_log << m_infoTag
				<< "GetIncomingConversationProfile: no matching profile found, falling back to default outgoing profile"
				<< iLogW::EndRecord;
		}
		
		return DefIncomingConversationProfile();
	}

	// -------------------------------------------------------------------------------------

	ISipAccessPointEvents* UserAgentRegistrator::CallsReceiver(const ConversationProfile &conversationProfile)
	{
		Item *item = FindRegistrationItem(conversationProfile);

		if (item == 0)
		{
			item = (m_defcallReceiver.Empty()) ? 0 : &m_defcallReceiver.Value();
		}
		return (item == 0) ? 0 : item->CallReceiver();
	}

	// -------------------------------------------------------------------------------------
	// IUserAgentRegistrator

	Utils::WeakRef<IRegistrationItem&>
		UserAgentRegistrator::AddRegistration(const std::string &accessPointName,
			ISipAccessPointEventsExt &user,
			boost::shared_ptr<ConversationProfile> conversationProfile)
	{
		Item *item = FindRegistrationItem(*conversationProfile);

		ESS_ASSERT(item == 0);

		item = new Item(
			boost::bind(&T::PointRegistrator, this, _1, _2),
			m_log,
			m_dum,
			accessPointName,
			user,
			conversationProfile);

		return item->RegistrationRef();
	}

	// -------------------------------------------------------------------------------------

	void UserAgentRegistrator::SetDefIncomingCallReceiver(
		const IRegistrationItem &callReceiver)
	{
		ListItem::iterator i = m_items.begin();
		while (i != m_items.end())
		{
			if (&(*i) == &callReceiver) break;
			++i;
		}
		
		ESS_ASSERT (i != m_items.end());

		m_defcallReceiver = i->SelfRef();
	}

	// -------------------------------------------------------------------------------------

	UserAgentRegistrator::Item* UserAgentRegistrator::FindRegistrationItem(
		const ConversationProfile &conversationProfile)
	{
		for (ListItem::iterator i = m_items.begin();
			i != m_items.end(); ++i)
		{
			if (i->GetConversationProfile().get() == &conversationProfile) return &(*i);
		}

		return 0;
	}

	// -------------------------------------------------------------------------------------

	boost::shared_ptr<ConversationProfile>
		UserAgentRegistrator::DefIncomingConversationProfile() const
	{
		return (m_defcallReceiver.Empty()) ?
			m_defaultConversationProfile :
			m_defcallReceiver.Value().GetConversationProfile();
	}

	// -------------------------------------------------------------------------------------

	void UserAgentRegistrator::PointRegistrator(Item *item, bool addToList)
	{
		if (addToList)
		{
			m_items.push_back(item);
			return;
		}
		
		ListItem::iterator i = m_items.begin();
		while (i != m_items.end())
		{
			if (&(*i) == item) break;
			++i;
		}

		ESS_ASSERT (i != m_items.end());

		m_items.erase(i);
	}

}
