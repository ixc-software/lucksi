#include "stdafx.h"
#include "iSip/undeferrno.h"

#include "ConversationManager.h"
#include "MsgRemoteParticipant.h"
#include "RemoteParticipant.h"
#include "RemoteParticipantDialogSet.h"
#include "SdpUtils.h"
#include "SipTrustedNode.h"

// resip includes
#include <resip/dum/DialogUsageManager.hxx>
#include <resip/dum/ClientInviteSession.hxx>
#include <resip/dum/ServerInviteSession.hxx>
#include <resip/dum/ClientSubscription.hxx>
#include <resip/dum/ServerOutOfDialogReq.hxx>

namespace iSip
{

	ConversationManager::ConversationManager(iCore::MsgThread &thread,
		iLogW::LogSession  &logSession,
		boost::shared_ptr<ConversationProfile> defaultConversationProfile,
		const SipTrustedNodeList &trustedNodeList,
		resip::DialogUsageManager &dum,
		CallReceiver callReceiver,
		ConversationProfileFinder conversationProfileFinder) :
		iCore::MsgObject(thread),
		m_log(logSession),
		m_infoTag(m_log.RegisterRecordKind(L"CallManagerInfo", true)),
		m_warningTag(m_log.RegisterRecordKind(L"CallManagerWarning", true)),
		m_defaultConversationProfile(defaultConversationProfile),
		m_trustedNodeList(trustedNodeList),
		m_dum(dum),
		m_callReceiver(callReceiver),
		m_conversationProfileFinder(conversationProfileFinder),
		m_clientSubscriptionHandler(CreateClientSubscriptionHandler())
	{

		ESS_ASSERT (!callReceiver.empty());
		ESS_ASSERT (!m_conversationProfileFinder.empty());

		m_dum.setRedirectHandler(this);
		m_dum.setInviteSessionHandler(&m_inviteSessionHandler);
		m_dum.setDialogSetHandler(this);

		m_dum.addOutOfDialogHandler(resip::OPTIONS, this);
		m_dum.addOutOfDialogHandler(resip::REFER, this);

		m_dum.addClientSubscriptionHandler("refer", m_clientSubscriptionHandler.get());

		m_serverSubscriptionHandler.reset(new iSip::ServerSubscriptionHandler(
			m_log,
			m_dum,
			boost::bind(&T::NewSubscriptionFromRefer, this, _1, _2)));
	}

	// -------------------------------------------------------------------------------------

	ConversationManager::~ConversationManager()
	{
		ESS_ASSERT(m_participants.empty());
	}

	// -------------------------------------------------------------------------------------

	RemoteParticipant* ConversationManager::CreateUACOriginalRemoteParticipant(
		boost::shared_ptr<ConversationProfile>	conversationProfile,
		ParticipantForkSelectMode::Value forkMode)
	{
		// Create new Participant - but use same participant handle
		RemoteParticipantDialogSet* participantDialogSet = new RemoteParticipantDialogSet(
			m_dum,
			*this,
			m_log,
			forkMode);

		return participantDialogSet->CreateUACOriginalRemoteParticipant(conversationProfile);

	}

	// -------------------------------------------------------------------------------------

	boost::shared_ptr<ConversationProfile>
		ConversationManager::GetIncomingConversationProfile(const resip::SipMessage& msg)
	{
		return m_conversationProfileFinder(msg);
	}

	// -------------------------------------------------------------------------------------

	void ConversationManager::PostMsg(boost::function<void ()> fn)
	{
		ESS_ASSERT (!fn.empty());
		PutFunctor(fn);
	}

	// -------------------------------------------------------------------------------------

	void ConversationManager::Shutdown()
	{
		// End each Participant
		m_participants.clear();
	}

	// -------------------------------------------------------------------------------------
	// IRemoteParticipantCreator

	Utils::SafeRef<IRemoteParticipant> ConversationManager::CreateOutgoingParticipant(
		Utils::SafeRef<IRemoteParticipantUser> user,
		boost::shared_ptr<Parameters> parameters)
	{
		ESS_ASSERT (parameters->ConversationProfile->IsValid());

		return new MsgRemoteParticipant(getMsgThread(),
			*this,
			boost::bind(&T::RemoteParticipantRegistrator, this, _1, _2),
			user,
			parameters);
	}

	// -------------------------------------------------------------------------------------
	// ISipCallManagerEvents

	void ConversationManager::OnIncomingParticipant(
		const ISipAccessPoint *src,
		Utils::SafeRef<IRemoteParticipant> newParticipant,
		const resip::SipMessage &msg,
		bool autoAnswer,
		const iSip::ConversationProfile &conversationProfile)
	{
		ESS_ASSERT (src == 0); // only one src - RemoteParticipant
		ESS_ASSERT (conversationProfile.IsValid());

		Utils::SafeRef<IRemoteParticipant> msgParticipant =
			CreateIncomingParticipant(newParticipant);

		ISipAccessPointEvents *callReceiver = m_callReceiver(conversationProfile);
		if (callReceiver == 0)
		{
			msgParticipant->DestroyParticipant(403,
				"Call receiver is missing. Destroy call. Message: " + MsgToString(msg));
			return;
		}

		if (!m_trustedNodeList.IsTrustedNode(msg.getSource()))
		{
			msgParticipant->DestroyParticipant(403,
				"Call source isn't in trusted list. Destroy call " + MsgToString(msg));
			return;
		}

		callReceiver->OnIncomingParticipant(0, msgParticipant, msg, autoAnswer, conversationProfile);
	}

	// -------------------------------------------------------------------------------------

	void ConversationManager::OnRequestOutgoingParticipant(
		const ISipAccessPoint *src,
		Utils::SafeRef<IRemoteParticipant> newParticipant,
		const resip::SipMessage &msg,
		const iSip::ConversationProfile &conversationProfile)
	{
		ESS_ASSERT (src == 0); // only one src - RemoteParticipant

		RequestOutgoingParticipant(newParticipant, msg, conversationProfile);
	}


	// -------------------------------------------------------------------------------------
	// OutOfDialogHandler

	void ConversationManager::onSuccess(resip::ClientOutOfDialogReqHandle, const resip::SipMessage &msg)
	{
		if(m_log.LogActive(m_infoTag))
		{
			m_log << m_infoTag << "onSuccess(ClientOutOfDialogReqHandle): "
				<< MsgToString(msg)
				<< iLogW::EndRecord;
		}
	}

	// -------------------------------------------------------------------------------------

	void ConversationManager::onFailure(resip::ClientOutOfDialogReqHandle, const resip::SipMessage &msg)
	{
		if(m_log.LogActive(m_infoTag))
		{
			m_log << m_infoTag << "onFailure(ClientOutOfDialogReqHandle): "
				<< MsgToString(msg)
				<< iLogW::EndRecord;
		}
	}

	// -------------------------------------------------------------------------------------

	void ConversationManager::onReceivedRequest(resip::ServerOutOfDialogReqHandle ood, const resip::SipMessage &msg)
	{
		if(m_log.LogActive(m_infoTag))
		{
			m_log << m_infoTag << "onReceivedRequest(ServerOutOfDialogReqHandle): "
				<< MsgToString(msg)
				<< iLogW::EndRecord;
		}

		if (msg.method() == resip::OPTIONS)
		{
			resip::SharedPtr<resip::SipMessage> optionsAnswer = ood->answerOptions();

			// Attach an offer to the options request
			resip::SdpContents sdp = m_conversationProfileFinder(msg)->sessionCaps();

			SdpUtils::BuildSdpOffer(sdp);
			optionsAnswer->setContents(&sdp);
			ood->send(optionsAnswer);
			return;
		}

		if (msg.method() == resip::REFER)
		{
			if (!msg.exists(resip::h_ReferTo))
			{
				if(m_log.LogActive(m_warningTag))
				{
					m_log << m_warningTag << "onReceivedRequest(ServerOutOfDialogReqHandle): Received refer w/out a Refer-To: "
						<< MsgToString(msg)
						<< iLogW::EndRecord;
				}
				ood->send(ood->reject(400));
				return;
			}

			// Received an OOD refer request with no refer subscription
			try
			{
				// Check if TargetDialog header is present
				if (msg.exists(resip::h_TargetDialog))
				{
					std::pair<resip::InviteSessionHandle, int> presult;
					presult = GetDialogUsageManager().findInviteSession(msg.header(resip::h_TargetDialog));

					if (!(presult.first == resip::InviteSessionHandle::NotValid()))
					{
						RemoteParticipant *participantToRefer = dynamic_cast<RemoteParticipant*>(presult.first->getAppDialog().get());

						ESS_ASSERT (participantToRefer != 0);

						// Accept the Refer
						ood->send(ood->accept(202 /* Refer Accepted */));

						participantToRefer->doReferNoSub(msg);
						return;
					}
	            }

				// Create new Participant
				RemoteParticipantDialogSet *participantDialogSet =
					new RemoteParticipantDialogSet(GetDialogUsageManager(), *this, m_log);

				RemoteParticipant *participant =
					participantDialogSet->CreateUACOriginalRemoteParticipant(GetIncomingConversationProfile(msg)); // refer_profile

				// Set pending OOD info in Participant - causes accept or reject to be called later
				participant->setPendingOODReferInfo(ood, msg);

				// Notify application
				ConversationProfile *profile = dynamic_cast<ConversationProfile*>(ood->getUserProfile().get());

				ESS_ASSERT(profile != 0);

				RequestOutgoingParticipant(participant, msg, *profile);
			}
			catch(resip::BaseException &e)
			{
				if(m_log.LogActive(m_warningTag))
				{
					m_log << m_warningTag << "onReceivedRequest(ServerOutOfDialogReqHandle): exception "
						<< SipUtils::ToString(e.getMessage())
						<< iLogW::EndRecord;
				}
			}
			return;
		} // if (msg.method() == resip::REFER)
	}

	// -------------------------------------------------------------------------------------
	////////////////////////////////////////////////////////////////////////////////
	// RedirectHandler /////////////////////////////////////////////////////////////
	////////////////////////////////////////////////////////////////////////////////

	void ConversationManager::onRedirectReceived(resip::AppDialogSetHandle, const resip::SipMessage &msg)
	{
		if(m_log.LogActive(m_infoTag))
		{
			m_log << m_infoTag << "onRedirectReceived(AppDialogSetHandle): "
				<< MsgToString(msg)
				<< iLogW::EndRecord;
		}
	}

	// -------------------------------------------------------------------------------------

	bool ConversationManager::onTryingNextTarget(resip::AppDialogSetHandle, const resip::SipMessage &msg)
	{
	   if(m_log.LogActive(m_infoTag))
	   {
		   m_log << m_infoTag << "onTryingNextTarget(AppDialogSetHandle): "
			   << MsgToString(msg)
			   << iLogW::EndRecord;
	   }

	   // Always allow redirection for now
	   return true;
	}

	// -------------------------------------------------------------------------------------
	// DialogSetHandler

	void ConversationManager::onTrying(
		resip::AppDialogSetHandle h, const resip::SipMessage& msg)
	{
		resip::DialogSetHandler *handler = dynamic_cast<resip::DialogSetHandler*>(h.get());

		if (handler != 0)
		{
			handler->onTrying(h, msg);
		}
		else
		{
			if(m_log.LogActive(m_infoTag))
			{
				m_log << m_infoTag << "onTrying(AppDialogSetHandle): "
					<< MsgToString(msg)
					<< iLogW::EndRecord;
			}
		}
	}

	// -------------------------------------------------------------------------------------

	void ConversationManager::onNonDialogCreatingProvisional(
		resip::AppDialogSetHandle h, const resip::SipMessage& msg)
	{
		resip::DialogSetHandler *handler = dynamic_cast<resip::DialogSetHandler*>(h.get());
		if (handler != 0)
		{
			handler->onNonDialogCreatingProvisional(h, msg);
		}
		else
		{
			if(m_log.LogActive(m_infoTag))
			{
				m_log << m_infoTag << "onNonDialogCreatingProvisional(AppDialogSetHandle): "
					<< MsgToString(msg)
					<< iLogW::EndRecord;
			}
		}
	}

	// -------------------------------------------------------------------------------------

	void ConversationManager::NewSubscriptionFromRefer(resip::ServerSubscriptionHandle ss, const resip::SipMessage &msg)
	{
		RemoteParticipantDialogSet *participantDialogSet =
			new RemoteParticipantDialogSet(GetDialogUsageManager(), *this, m_log);

		RemoteParticipant *participant =
			participantDialogSet->CreateUACOriginalRemoteParticipant(GetIncomingConversationProfile(msg));  // refer_profile

		// Set pending OOD info in Participant - causes accept or reject to be called later
		participant->setPendingOODReferInfo(ss, msg);

		// Notify application
		ConversationProfile* profile = dynamic_cast<ConversationProfile*>(ss->getUserProfile().get());
		
		ESS_ASSERT(profile != 0);

		RequestOutgoingParticipant(participant, msg, *profile);
	}

	// -------------------------------------------------------------------------------------

	void ConversationManager::RemoteParticipantRegistrator(IRemoteParticipant *item, bool addToList)
	{
		if (addToList)
		{
			m_participants.push_back(item);
			return;
		}

		ListOfParticipant::iterator i = m_participants.begin();
		while(i != m_participants.end())
		{
			if (&(*i) == item) 	break;
			++i;
		}
		ESS_ASSERT(i != m_participants.end());

		PutMsg(this, &T::OnDeleteParticipants, m_participants.release(i).release());
	}

	// -------------------------------------------------------------------------------------

	MsgRemoteParticipant* ConversationManager::CreateIncomingParticipant(
		Utils::SafeRef<IRemoteParticipant> newParticipant)
	{
		return new MsgRemoteParticipant(getMsgThread(),
			*this,
			boost::bind(&T::RemoteParticipantRegistrator, this, _1, _2),
			newParticipant);
	}

	// -------------------------------------------------------------------------------------

	void ConversationManager::RequestOutgoingParticipant(
		Utils::SafeRef<IRemoteParticipant> newParticipant,
		const resip::SipMessage &msg,
		const iSip::ConversationProfile &conversationProfile)
	{

		Utils::SafeRef<IRemoteParticipant> msgParticipant =
			CreateIncomingParticipant(newParticipant);

		ISipAccessPointEvents *callReceiver = m_callReceiver(conversationProfile);

		if (callReceiver == 0)
		{
			msgParticipant->DestroyParticipant(403);
			return;
		}

		callReceiver->OnRequestOutgoingParticipant(0, msgParticipant, msg, conversationProfile);
	}

	// -------------------------------------------------------------------------------------

	std::string ConversationManager::MsgToString(const resip::SipMessage &msg)
	{
		return SipUtils::ToString(m_defaultConversationProfile->Misc.ResipLogMode(), msg);
	}

}
