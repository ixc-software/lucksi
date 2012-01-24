#include "stdafx.h"
#include "iSip/undeferrno.h"

#include "UserAgentClientSubscription.h"
#include "SipUtils.h"

#include "iLog/LogWrapper.h"
#include "Utils/AtomicTypes.h"
#include "Utils/IntToString.h"

#include <resip/dum/DialogUsageManager.hxx>
#include <resip/dum/ClientSubscription.hxx>

namespace {

	using namespace iSip;

	resip::ClientSubscriptionHandler& SafeCast(resip::ClientSubscriptionHandle h)
	{
		resip::ClientSubscriptionHandler *item =
			dynamic_cast<resip::ClientSubscriptionHandler*>(h->getAppDialogSet().get());

		ESS_ASSERT(item != 0);

		return *item;
	}

	// -------------------------------------------------------------------------------------

	class ClientSubscriptionWrapper : boost::noncopyable,
		public IUserAgentSubscription
	{
	public:

		ClientSubscriptionWrapper(IUserAgentSubscriptionEvent &user) :
			User(user)
		{}

		~ClientSubscriptionWrapper()
		{
			Destructor();
		}

		Utils::WeakRef<ClientSubscriptionWrapper&> SelfRef()
		{
			return m_selfHost.Create<ClientSubscriptionWrapper&>(*this);
		}

		IUserAgentSubscriptionEvent &User;

		boost::function<void ()> Destructor;
		
	private:
		Utils::WeakRefHost m_selfHost;
	};

	std::string GetSubscriptionName()
	{
		static Utils::AtomicInt GSubscriptionCount;

		return "ClientSubscription_" + Utils::IntToString(GSubscriptionCount.FetchAndAdd(1));
	}

}

namespace iSip
{

	class UserAgentClientSubscription::ClientSubscription : boost::noncopyable,
		public resip::AppDialogSet,
		public IClientSubscriptionHandler
	{
		typedef ClientSubscription T;

	public:

		ClientSubscription(iLogW::ILogSessionCreator &logCreator,
			resip::DialogUsageManager& dum,
			ClientSubscriptionWrapper &wrapp) :
			resip::AppDialogSet(dum),
			m_log(logCreator.CreateSession(GetSubscriptionName(), true)),
			m_infoTag(m_log->RegisterRecordKind(L"Info", true)),
			m_wrapp(wrapp.SelfRef())
		{
			m_lastNotifyHash = 0;
			m_ended = false;

			Utils::WeakRef<ClientSubscription&> item = m_selfHost.Create<ClientSubscription&>(*this);

			wrapp.Destructor = boost::bind(&T::StaticEnd, item);

			if(m_log->LogActive(m_infoTag))
			{
				*m_log << m_infoTag << "Created" << iLogW::EndRecord;
			}

		}

		~ClientSubscription()
		{
			if(m_log->LogActive(m_infoTag))
			{
				*m_log << m_infoTag << "Deleted" << iLogW::EndRecord;
			}
		}

		void End()
		{
			if (m_ended) return;

			m_ended = true;
			AppDialogSet::end();
		}

		Utils::WeakRef<ClientSubscription&> SelfRef()
		{
			return m_selfHost.Create<ClientSubscription&>(*this);
		}

	// IClientSubscriptionHandler
	private:

		void onUpdatePending(resip::ClientSubscriptionHandle h, const resip::SipMessage &notify, bool outOfOrder)
		{
			LogMsg("onUpdatePending(ClientSubscriptionHandle), ", &notify);

			AcceptUpdate(h, notify);
		}

		void onUpdateActive(resip::ClientSubscriptionHandle h, const resip::SipMessage &notify, bool outOfOrder)
		{
			LogMsg("onUpdateActive(ClientSubscriptionHandle), ", &notify);

			AcceptUpdate(h, notify);
		}

		void onUpdateExtension(resip::ClientSubscriptionHandle h, const resip::SipMessage& notify, bool outOfOrder)
		{
			LogMsg("onUpdateExtension(ClientSubscriptionHandle), ", &notify);

			AcceptUpdate(h, notify);
		}

		void onTerminated(resip::ClientSubscriptionHandle h, const resip::SipMessage *notify)
		{

			LogMsg("onTerminated(ClientSubscriptionHandle), ", notify);

			if (notify == 0)
			{
				SubscriptionTerminated(408);// timeout waiting for notify after subscribe
				return;
			}

			unsigned int statusCode = 0;

			if (notify->isResponse())
			{
				statusCode = notify->header(resip::h_StatusLine).responseCode();
			}
			else
			{
				if (notify->getContents() != 0)
				{
					NotifyReceived(notify->getContents()->getBodyData());
				}
			}

			SubscriptionTerminated(statusCode);
		}

		void onNewSubscription(resip::ClientSubscriptionHandle h, const resip::SipMessage& notify)
		{
			LogMsg("onNewSubscription(ClientSubscriptionHandle), ", &notify);

			// Note:  The notify here, will also be passed in an onUpdateXXXX callback, so no need to do anything with this callback
		}

		int  onRequestRetry(resip::ClientSubscriptionHandle h, int retryMinimum, const resip::SipMessage& notify)
		{
			return -1;
		}

	private:

		static void StaticEnd(Utils::WeakRef<ClientSubscription&> item)
		{
			if (!item.Empty()) item.Value().End();
		}

		void AcceptUpdate(resip::ClientSubscriptionHandle h, const resip::SipMessage &notify)
		{
			h->acceptUpdate();

			if (m_ended)
			{
				h->end();
				return;
			}

			if (notify.getContents() != 0)
			{
				const resip::Data& bodyData = notify.getContents()->getBodyData();
				NotifyReceived(bodyData);
			}
		}

		void NotifyReceived(const resip::Data& notifyData)
		{
			if (m_ended) return;

			if (m_wrapp.Empty()) return;

			size_t hash = notifyData.hash();

			if (hash == m_lastNotifyHash)  return;

			// only call callback if body changed from last time

			m_lastNotifyHash = hash;
			m_wrapp.Value().User.SubscriptionNotify(m_wrapp.Value(), notifyData);
		}

		void SubscriptionTerminated(int statusCode)
		{
			if (m_ended) return;

			if (m_wrapp.Empty()) return;
				
			m_wrapp.Value().User.SubscriptionTerminated(m_wrapp.Value(), statusCode);
		}

		void LogMsg(const std::string &event, const resip::SipMessage *msg)
		{
			if(m_log->LogActive(m_infoTag))
			{
				*m_log << m_infoTag << event;
				
				if (msg != 0) *m_log << SipUtils::ToString(msg);
				
				*m_log << iLogW::EndRecord;
			}
		}

	private:
		boost::scoped_ptr<iLogW::LogSession>  m_log;
		iLogW::LogRecordTag m_infoTag;

		Utils::WeakRefHost m_selfHost;
		Utils::WeakRef<ClientSubscriptionWrapper&> m_wrapp;

		size_t m_lastNotifyHash;
		bool m_ended;
	};
}

namespace iSip
{

	UserAgentClientSubscription::UserAgentClientSubscription(resip::DialogUsageManager &dum) :
		m_dum(dum),
		m_clientSubscriptionHandler(CreateClientSubscriptionHandler())
	{}

	// -------------------------------------------------------------------------------------

	UserAgentClientSubscription::~UserAgentClientSubscription()
	{
		for(Subscriptions::const_iterator i = m_subscriptions.begin();
			i != m_subscriptions.end(); ++i)
		{
			ESS_ASSERT(i->Empty());
		}
	}

	// -------------------------------------------------------------------------------------

	void UserAgentClientSubscription::Shutdown()
	{
		for(Subscriptions::iterator i = m_subscriptions.begin();
			i != m_subscriptions.end(); ++i)
		{
			if (!i->Empty()) i->Value().End();
		}
	}

	// -------------------------------------------------------------------------------------

	IUserAgentSubscription* UserAgentClientSubscription::CreateSubscription(
		iLogW::ILogSessionCreator &logCreator,
		IUserAgentSubscriptionEvent &user,
		resip::SharedPtr<resip::UserProfile> profile,
		const resip::Data &eventType,
		const resip::NameAddr &target,
		unsigned int subscriptionTime)
	{
		if (!m_dum.getClientSubscriptionHandler(eventType))
		{
			m_dum.addClientSubscriptionHandler(eventType, m_clientSubscriptionHandler.get());
		}

		ClientSubscriptionWrapper *item = new ClientSubscriptionWrapper(user);

		ClientSubscription *subscription = new ClientSubscription(logCreator, m_dum, *item);

		m_dum.send(m_dum.makeSubscription(target, profile, eventType, subscriptionTime, subscription));
		
		// add new subccription to list

		Subscriptions::iterator i = m_subscriptions.begin();

		while(i != m_subscriptions.end())
		{
			if (i->Empty()) break;
			
			++i;
		}

		if (i == m_subscriptions.end())
			m_subscriptions.push_back(subscription->SelfRef());
		else
			*i = subscription->SelfRef();

		return item;
	}

}
