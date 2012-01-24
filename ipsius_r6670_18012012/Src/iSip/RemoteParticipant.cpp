#include "stdafx.h"
#include "iSip/undeferrno.h"

#include "RemoteParticipant.h"
#include "RemoteParticipantDialogSet.h"
#include "ConversationManager.h"
#include "SdpUtils.h"
#include "SipUtils.h"
#include "SipMessageHelper.h"

#include "Sdp/SdpHelperResip.h"
#include "Sdp/Sdp.h"

#include <rutil/DnsUtil.hxx>
#include <rutil/Random.hxx>
#include <resip/stack/SipFrag.hxx>
#include <resip/stack/ExtensionHeader.hxx>
#include <resip/dum/DialogUsageManager.hxx>
#include <resip/dum/ClientInviteSession.hxx>
#include <resip/dum/ServerInviteSession.hxx>
#include <resip/dum/ClientSubscription.hxx>
#include <resip/dum/ServerOutOfDialogReq.hxx>
#include <resip/dum/ServerSubscription.hxx>

#include "Utils/AtomicTypes.h"
#include "Utils/IntToString.h"
#include "Utils/ErrorsSubsystem.h"

namespace
{
	std::string GenerateRemoteParticipantName()
	{
		static Utils::AtomicInt GNumber;

		return "RemoteParticipant_" + Utils::IntToString(GNumber.FetchAndAdd(1));
	}

	resip::NameAddr UpdateDestinationAddr(const resip::NameAddr &destination,
		resip::UserProfile &profile)
	{
		if (!destination.uri().host().empty()) return destination;

		resip::NameAddr res;

		resip::Data user =  destination.uri().user();
			
		res.uri() = profile.getDefaultFrom().uri();

		res.uri().user() = user;

		return res;
	}

	iSip::ISipMedia* Media(Utils::SafeRef<iSip::IRemoteParticipantUser> &user)
	{
		ESS_ASSERT (!user.IsEmpty());

		iSip::ISipMedia *media = user->Media();
		ESS_ASSERT (media != 0);

		return media;
	}

	const iSip::ISipMedia* Media(const Utils::SafeRef<iSip::IRemoteParticipantUser> &user)
	{
		ESS_ASSERT (!user.IsEmpty());

		const iSip::ISipMedia *media = user->Media();
		ESS_ASSERT (media != 0);
		return media;
	}

	boost::shared_ptr<iReg::CallRecordWrapper>
		CreateCallRecordWrapper(boost::shared_ptr<iReg::CallRecordWrapper> ptr,
			iReg::CallRecordWrapper::ObserverFn fn)
	{
		if (ptr == 0) ptr.reset(new iReg::CallRecordWrapper(fn));
		return ptr;
	}

}

namespace iSip
{

	// UAC
	RemoteParticipant::RemoteParticipant(
		ConversationManager& conversationManager,
		resip::DialogUsageManager& dum,
		RemoteParticipantDialogSet& remoteParticipantDialogSet,
		iLogW::ILogSessionCreator &logCreator,
		boost::function<void (RemoteParticipant*, bool)> registrator,
		boost::shared_ptr<ConversationProfile> conversationProfile,
		Utils::SafeRef<IRemoteParticipantUser> user,
		const resip::NameAddr &destination)	:
		AppDialog(dum),
		m_log(logCreator.CreateSession(GenerateRemoteParticipantName(), true)),
		m_infoTag(m_log->RegisterRecordKind(L"Info", true)),
		m_warningTag(m_log->RegisterRecordKind(L"Warning", true)),
		m_debugTag(m_log->RegisterRecordKind(L"Debug", true)),
		m_registrator(registrator),
		m_conversationManager(conversationManager),
		m_dum(dum),
		m_dialogSet(remoteParticipantDialogSet),
		m_user(user),
		m_conversationProfile(conversationProfile),
		m_callRecord(
            CreateCallRecordWrapper(m_user->CallRecordByUser(), m_conversationProfile->Misc.CallRecordFn())),
		m_regRecord(m_callRecord->Record().Dst()),
		m_dialogId(resip::Data::Empty, resip::Data::Empty, resip::Data::Empty),
		m_state(Connecting),
		m_offerRequired(false)
	{

		ESS_ASSERT (m_conversationProfile != 0);
		ESS_ASSERT (!m_user.IsEmpty());

		if(m_log->LogActive(m_infoTag))
		{
			*m_log << m_infoTag << "Created (UAC). User: "
				<< m_user->CallUserName()
				<<  iLogW::EndRecord;
		}
		
		m_regRecord.SetName(NameParticipant().c_str());	

		m_registrator(this, true);

		InitiateRemoteCall(destination);
	}

	// -------------------------------------------------------------------------------------
	// UAS - or forked leg

	RemoteParticipant::RemoteParticipant(ConversationManager& conversationManager,
		resip::DialogUsageManager& dum,
		RemoteParticipantDialogSet& remoteParticipantDialogSet,
		iLogW::ILogSessionCreator &logCreator,
		boost::function<void (RemoteParticipant*, bool)> registrator,
		boost::shared_ptr<ConversationProfile> conversationProfile) :
		AppDialog(dum),
		m_log(logCreator.CreateSession(GenerateRemoteParticipantName(), true)),
		m_infoTag(m_log->RegisterRecordKind(L"Info", true)),
		m_warningTag(m_log->RegisterRecordKind(L"Warning", true)),
		m_debugTag(m_log->RegisterRecordKind(L"Debug", true)),
		m_registrator(registrator),
		m_conversationManager(conversationManager),
		m_dum(dum),
		m_dialogSet(remoteParticipantDialogSet),
		m_conversationProfile(conversationProfile),
		m_callRecord(
            new iReg::CallRecordWrapper(m_conversationProfile->Misc.CallRecordFn())),
		m_regRecord(m_callRecord->Record().Src()),
		m_dialogId(resip::Data::Empty, resip::Data::Empty, resip::Data::Empty),
		m_state(Connecting),
		m_offerRequired(false)
	{
		ESS_ASSERT(m_conversationProfile != 0);

		if(m_log->LogActive(m_infoTag))
		{
			*m_log << m_infoTag << "Created (UAS or forked leg)" << iLogW::EndRecord;
		}

		m_regRecord.SetName(NameParticipant().c_str());
		
		m_registrator(this, true);
	}

	// -------------------------------------------------------------------------------------

	RemoteParticipant::~RemoteParticipant()
	{
		ESS_ASSERT (m_user.IsEmpty());

		if(m_log->LogActive(m_infoTag))
		{
			*m_log << m_infoTag  << "Deleted" << iLogW::EndRecord;
		}

		m_registrator(this, false);
	}

	// -------------------------------------------------------------------------------------
/*
	std::string RemoteParticipant::Name() const
	{
		return m_log->NameStr();
	}
*/
	// -------------------------------------------------------------------------------------

	void RemoteParticipant::destroyParticipant()
	{
		try
		{
			if (m_state == Terminating) return;

			stateTransition(Terminating);

			if (m_inviteSessionHandle.isValid())
			{
				m_inviteSessionHandle->end();
			}
			else
			{
				m_dialogSet.end();
			}

		}
		catch(resip::BaseException &e)
		{
			if(m_log->LogActive(m_warningTag))
			{
				*m_log << m_warningTag << "RemoteParticipant::destroyParticipant exception: "
					<< SipUtils::ToString(e.getMessage())
					<< iLogW::EndRecord;
			}
		}
	}

	// -------------------------------------------------------------------------------------

	std::string RemoteParticipant::StateToString(State state)
	{
		switch(state)
		{
			case Connecting:		return "Connecting";
			case Accepted:			return "Accepted";
			case Connected:			return "Connected";
			case Redirecting:		return "Redirecting";
			case Reinviting:		return "Offering";
			case Replacing:			return "Replacing";
			case PendingOODRefer:	return "PendingOODRefer";
			case Terminating:		
				return "Terminating";
		}
			
		ESS_HALT("Unknown state");
		return "";
	}

	// -------------------------------------------------------------------------------------

	void RemoteParticipant::stateTransition(State state)
	{
		if(m_log->LogActive(m_infoTag))
		{
			*m_log  << m_infoTag << "State transition to state " << StateToString(state) << iLogW::EndRecord;
		}

		m_state = state;

		if (m_state == Connected && !m_pendingRequest.empty())
		{
			boost::function<void ()> pendingRequest = m_pendingRequest;
			m_pendingRequest.clear();
			pendingRequest();
		}
	}

	// -------------------------------------------------------------------------------------

	void RemoteParticipant::setPendingOODReferInfo(resip::ServerOutOfDialogReqHandle ood, const resip::SipMessage& referMsg)
	{
		stateTransition(PendingOODRefer);
		mPendingOODReferMsg = referMsg;
		mPendingOODReferNoSubHandle = ood;
	}

	// -------------------------------------------------------------------------------------

	void RemoteParticipant::setPendingOODReferInfo(resip::ServerSubscriptionHandle ss, const resip::SipMessage& referMsg)
	{
		stateTransition(PendingOODRefer);
		mPendingOODReferMsg = referMsg;
		mPendingOODReferSubHandle = ss;
	}

	// -------------------------------------------------------------------------------------

	void RemoteParticipant::acceptPendingOODRefer()
	{
		ESS_ASSERT(m_state == PendingOODRefer);

		resip::SharedPtr<resip::UserProfile> profile;
		bool accepted = false;
		if (mPendingOODReferNoSubHandle.isValid())
		{
			mPendingOODReferNoSubHandle->send(mPendingOODReferNoSubHandle->accept(202));  // Accept OOD Refer
			profile = mPendingOODReferNoSubHandle->getUserProfile();
			accepted = true;
		}
		else if (mPendingOODReferSubHandle.isValid())
		{
			mPendingOODReferSubHandle->send(mPendingOODReferSubHandle->accept(202));  // Accept OOD Refer
			profile = mPendingOODReferSubHandle->getUserProfile();
			accepted = true;
		}

		if (!accepted)
		{
			if(m_log->LogActive(m_warningTag))
			{
				*m_log << m_warningTag << "acceptPendingOODRefer - no valid handles" << iLogW::EndRecord;
			}

			m_user.Clear()->OnParticipantTerminated(this, 500);
			delete this;
			return;
		}

		// Create offer
		resip::SdpContents offer;
		
		Media(m_user)->GenerateOffer(offer);

		// Build the Invite
		resip::SharedPtr<resip::SipMessage> invitemsg = m_dum.makeInviteSessionFromRefer(
			mPendingOODReferMsg,
			profile,
			mPendingOODReferSubHandle,  // Note will be invalid if refer no-sub, which is fine
			&offer,
			resip::DialogUsageManager::None,  //EncryptionLevel
			0,     //Aleternative Contents
			&m_dialogSet);

		m_dum.send(invitemsg);
		
		Media(m_user)->AdjustRTPStreams(true);

		stateTransition(Connecting);

	}

	// -------------------------------------------------------------------------------------

	void RemoteParticipant::rejectPendingOODRefer(unsigned int statusCode)
	{
		ESS_ASSERT(m_state == PendingOODRefer);

		if (mPendingOODReferNoSubHandle.isValid())
		{
			mPendingOODReferNoSubHandle->send(mPendingOODReferNoSubHandle->reject(statusCode));
		}
		else if (mPendingOODReferSubHandle.isValid())
		{
			mPendingOODReferSubHandle->send(mPendingOODReferSubHandle->reject(statusCode));
		}
		else
		{
			if(m_log->LogActive(m_warningTag))
			{
				*m_log << m_warningTag << "rejectPendingOODRefer - no valid handles" << iLogW::EndRecord;
			}
			statusCode = 500;
		}

		m_user.Clear()->OnParticipantTerminated(this, statusCode);

		m_dialogSet.destroy();  // Will also cause "this" to be deleted
	}

	// -------------------------------------------------------------------------------------

	void RemoteParticipant::redirectPendingOODRefer(const resip::NameAddr& destination)
	{
		ESS_ASSERT(m_state == PendingOODRefer);

		int statusCode = 0;

		if (mPendingOODReferNoSubHandle.isValid())
		{
			statusCode = 302; // Moved Temporarily
			resip::SharedPtr<resip::SipMessage> redirect = mPendingOODReferNoSubHandle->reject(statusCode);
			redirect->header(resip::h_Contacts).clear();
			redirect->header(resip::h_Contacts).push_back(destination);
			mPendingOODReferNoSubHandle->send(redirect);
		}
		else if (mPendingOODReferSubHandle.isValid())
		{
			statusCode = 302; // Moved Temporarily
			resip::SharedPtr<resip::SipMessage> redirect = mPendingOODReferSubHandle->reject(statusCode);
			redirect->header(resip::h_Contacts).clear();
			redirect->header(resip::h_Contacts).push_back(destination);
			mPendingOODReferSubHandle->send(redirect);
		}
		else
		{
			if(m_log->LogActive(m_warningTag))
			{
				*m_log << m_warningTag << "rejectPendingOODRefer - no valid handles" << iLogW::EndRecord;
			}
			statusCode = 500;
		}

		m_user.Clear()->OnParticipantTerminated(this, statusCode);
		m_dialogSet.destroy();  // Will also cause "this" to be deleted
	}

	// -------------------------------------------------------------------------------------

	void RemoteParticipant::processReferNotify(const resip::SipMessage& notify)
	{
		if (m_state != Redirecting) return;

		unsigned int code = 400;  // Bad Request - default if for some reason a valid resip::SipFrag is not present

		resip::SipFrag* frag  = dynamic_cast<resip::SipFrag*>(notify.getContents());
		if (frag != 0)
		{
			// Get StatusCode from resip::SipFrag
			if (frag->message().isResponse())
			{
				code = frag->message().header(resip::h_StatusLine).statusCode();
			}
		}

		// Check if success or failure response code was in resip::SipFrag
		if (code >= 200 && code < 300)
		{
			m_user->OnParticipantRedirectSuccess(this);
			stateTransition(Connected);
		}

		if (code >= 300)
		{
			m_user->OnParticipantRedirectFailure(this, code);
			stateTransition(Connected);
		}
	}

	// -------------------------------------------------------------------------------------

	void RemoteParticipant::OnNonDialogCreatingProvisional(const resip::SipMessage &msg)
	{
		if(m_log->LogActive(m_infoTag))
		{
			*m_log  << m_infoTag << "onNonDialogCreatingProvisional, "
				<< MsgToString(msg)
				<< iLogW::EndRecord;
		}

		if (!m_user.IsEmpty()) m_user->OnParticipantAlerting(this, msg);
	}

	// -------------------------------------------------------------------------------------

	void RemoteParticipant::OnForkingOccured(const IRemoteParticipant *src,
		Utils::SafeRef<IRemoteParticipant> newCall,
		const resip::SipMessage &msg)
	{
		ESS_ASSERT (src == this);

		if (!m_user.IsEmpty()) m_user->OnForkingOccured(this, newCall, msg);
	}

	// -------------------------------------------------------------------------------------

	void RemoteParticipant::OnReplacedBy(Utils::SafeRef<IRemoteParticipant> newCall)
	{
		if (!m_user.IsEmpty()) m_user.Clear()->OnParticipantReplacedBy(this, newCall);

		destroyParticipant();
	}

	// -------------------------------------------------------------------------------------

	void RemoteParticipant::provideOffer(bool postOfferAccept)
	{
	
		ESS_ASSERT(m_inviteSessionHandle.isValid());
		
		resip::SdpContents offer;
		
		Media(m_user)->GenerateOffer(offer);

		doProvideOfferAnswer(true,
			offer,
			postOfferAccept,
			false);

		m_offerRequired = false;
	}

	// -------------------------------------------------------------------------------------

	bool RemoteParticipant::ProvideAnswer(bool postAnswerAccept, bool postAnswerAlert)
	{
		ESS_ASSERT(m_inviteSessionHandle.isValid());

		resip::SdpContents answer;
		
		bool answerOk = Media(m_user)->GenerateAnswer(answer);
		
		if(!answerOk)
		{
			m_inviteSessionHandle->reject(488);
			return false;
		}

		doProvideOfferAnswer(false,
			answer,
			postAnswerAccept,
			postAnswerAlert);

		return true;
	}

	// -------------------------------------------------------------------------------------

	void RemoteParticipant::onNewSession(resip::ClientInviteSessionHandle h, resip::InviteSession::OfferAnswerType oat, const resip::SipMessage& msg)
	{
		if(m_log->LogActive(m_infoTag))
		{
			*m_log  << m_infoTag << "onNewSession(Client): " << MsgToString(msg) << iLogW::EndRecord;
		}

		m_inviteSessionHandle = h->getSessionHandle();
		m_dialogId = getDialogId();
	}

	// -------------------------------------------------------------------------------------

	void RemoteParticipant::onNewSession(resip::ServerInviteSessionHandle h, resip::InviteSession::OfferAnswerType oat, const resip::SipMessage& msg)
	{
		if(m_log->LogActive(m_infoTag))
		{
			*m_log  << m_infoTag << "onNewSession(Server): " << MsgToString(msg) << iLogW::EndRecord;
		}

		m_inviteSessionHandle = h->getSessionHandle();
		m_dialogId = getDialogId();

		// First check if this INVITE is to replace an existing session
		if (msg.exists(resip::h_Replaces))
		{
			std::pair<resip::InviteSessionHandle, int> presult;
			presult = m_dum.findInviteSession(msg.header(resip::h_Replaces));
			if (!(presult.first == resip::InviteSessionHandle::NotValid()))
			{
				RemoteParticipant *participantToReplace = dynamic_cast<RemoteParticipant *>(presult.first->getAppDialog().get());

				ESS_ASSERT(participantToReplace != 0);

				if(m_log->LogActive(m_infoTag))
				{
					*m_log  << m_infoTag << "onNewSession(Server), to replace participant "
						<< participantToReplace->NameParticipant()
						<< ", msg: "
						<< MsgToString(msg)
						<< iLogW::EndRecord;
				}

				// We are about to adjust the participant handle of the replacing participant to ours
				// ensure that the mapping is also adjusted in the replacing participants dialog set
				if (m_dialogSet.IsActiveRemoteParticipant(*participantToReplace))
				{
					m_dialogSet.SetActiveRemoteParticipant(SelfRef());
				}

				// Session to replace was found - end old session and flag to auto-answer this session after SDP offer-answer is complete
				participantToReplace->OnReplacedBy(this);

				stateTransition(Replacing);
				return;
			}
		}

		// Check for Auto-Answer indication - support draft-ietf-answer-mode-01
		// and Answer-After parameter of Call-Info header
		
		ESS_ASSERT(m_conversationProfile != 0);
		bool autoAnswerRequired = false;
		bool autoAnswer = m_conversationProfile->
			GetAutoAnswerProfile().ShouldAutoAnswer(msg, &autoAnswerRequired);

		if (!autoAnswer && autoAnswerRequired)  // If we can't autoAnswer but it was required, we must reject the call
		{
			resip::WarningCategory warning;
			warning.hostname() = resip::DnsUtil::getLocalHostName();
			warning.code() = 399; /* Misc. */
			warning.text() = "automatic answer forbidden";
			// m_participantHandle = 0; // Don't generate any callbacks for this rejected invite
			h->reject(403 /* Forbidden */, &warning);
			return;
		}

		// notify of new participant
		ESS_ASSERT(m_user.IsEmpty());

		iSip::SipMessageHelper msgHelper(msg);
		m_regRecord.SetCalledInfo(msgHelper.To().uri().ToString().c_str());
		m_regRecord.SetCallingInfo(msgHelper.From().uri().ToString().c_str());

		m_conversationManager.OnIncomingParticipant(0, /* src ptr */
			this,
			msg,
			autoAnswer,
			*m_conversationProfile);

		ESS_ASSERT(!m_user.IsEmpty());
	}

	// -------------------------------------------------------------------------------------

	void RemoteParticipant::onFailure(resip::ClientInviteSessionHandle h, const resip::SipMessage& msg)
	{
	   if(m_log->LogActive(m_infoTag))
	   {
		   *m_log  << m_infoTag << "onFailure: " << MsgToString(msg) << iLogW::EndRecord;
	   }

	   stateTransition(Terminating);

	   // If ForkSelectMode is automatic, then ensure we destory any participant, except the original
	   if (m_dialogSet.getForkSelectMode() == ParticipantForkSelectMode::Automatic &&
		  !m_dialogSet.IsActiveRemoteParticipant(*this))
	   {
		  destroyParticipant();
	   }
	}

	// -------------------------------------------------------------------------------------

	void RemoteParticipant::onEarlyMedia(resip::ClientInviteSessionHandle h, const resip::SipMessage &msg, const resip::SdpContents &sdp)
	{
		if(m_log->LogActive(m_infoTag))
		{
			*m_log  << m_infoTag << "onEarlyMedia: " << MsgToString(msg) << iLogW::EndRecord;
		}

		if (m_dialogSet.isStaleFork(*this)) return;

		InitRemoteSdp(sdp);
		
		Media(m_user)->AdjustRTPStreams(false);
	}

	// -------------------------------------------------------------------------------------

	void RemoteParticipant::onProvisional(resip::ClientInviteSessionHandle h, const resip::SipMessage& msg)
	{
		if(m_log->LogActive(m_infoTag))
		{
			*m_log  << m_infoTag << "onProvisional: " << MsgToString(msg) << iLogW::EndRecord;
		}

		ESS_ASSERT(msg.header(resip::h_StatusLine).responseCode() != 100);

		if (!m_dialogSet.isStaleFork(*this))
		{
			if (!m_user.IsEmpty()) m_user->OnParticipantAlerting(this, msg);
		}
	}

	// -------------------------------------------------------------------------------------

	void RemoteParticipant::onConnected(resip::ClientInviteSessionHandle h, const resip::SipMessage& msg)
	{
		if(m_log->LogActive(m_infoTag))
		{
			*m_log  << m_infoTag << "onConnected(Client): " << MsgToString(msg) << iLogW::EndRecord;
		}

		// Check if this is the first leg in a potentially forked call to send a 200
		if (m_dialogSet.isUACConnected())
		{
			// We already have a 200 response - send a BYE to this leg
			h->end();
			return;
		}

		if (!m_user.IsEmpty()) m_user->OnParticipantConnected(this, msg);

		m_dialogSet.SetUACConnected(getDialogId(), SelfRef());
		stateTransition(Connected);
	}

	// -------------------------------------------------------------------------------------

	void RemoteParticipant::onConnected(resip::InviteSessionHandle, const resip::SipMessage& msg)
	{
		if(m_log->LogActive(m_infoTag))
		{
			*m_log  << m_infoTag << "onConnected: " << MsgToString(msg) << iLogW::EndRecord;
		}

		stateTransition(Connected);
	}

	// -------------------------------------------------------------------------------------

	void RemoteParticipant::onConnectedConfirmed(resip::InviteSessionHandle, const resip::SipMessage& msg)
	{
		if(m_log->LogActive(m_infoTag))
		{
			*m_log  << m_infoTag << "onConnectedConfirmed: " << MsgToString(msg) << iLogW::EndRecord;
		}

		stateTransition(Connected);
	}

	// -------------------------------------------------------------------------------------

	void RemoteParticipant::onStaleCallTimeout(resip::ClientInviteSessionHandle)
	{
		if(m_log->LogActive(m_warningTag))
		{
			*m_log << m_warningTag << "onStaleCallTimeout: " << iLogW::EndRecord;
		}
	}

	// -------------------------------------------------------------------------------------

	std::string ReasonToString(resip::InviteSessionHandler::TerminatedReason reason)
	{
		switch(reason)
		{
		case resip::InviteSessionHandler::RemoteBye:
			return "received a BYE from peer";
		case resip::InviteSessionHandler::RemoteCancel:
			return "received a CANCEL from peer";
		case resip::InviteSessionHandler::Rejected:
			return "received a rejection from peer";
		case resip::InviteSessionHandler::LocalBye:
			return "ended locally via BYE";
		case resip::InviteSessionHandler::LocalCancel:
			return "ended locally via CANCEL";
		case resip::InviteSessionHandler::Replaced:
			return "ended due to being replaced";
		case resip::InviteSessionHandler::Referred:
			return "ended due to being reffered";
		case resip::InviteSessionHandler::Error:
			return "ended due to an error";
		case resip::InviteSessionHandler::Timeout:
			return "ended due to a timeout";
		}
		return "Wrong reason";
	}

	// -------------------------------------------------------------------------------------

	void RemoteParticipant::onTerminated(resip::InviteSessionHandle h, resip::InviteSessionHandler::TerminatedReason reason, const resip::SipMessage* msg)
	{
		stateTransition(Terminating);
		
		unsigned int statusCode = 0;

		if (msg != 0 && msg->isResponse())	statusCode = msg->header(resip::h_StatusLine).responseCode();

		// Ensure terminating party is from answered fork before generating event
		if (!m_dialogSet.isStaleFork(*this))
		{
			if (!m_user.IsEmpty()) m_user.Clear()->OnParticipantTerminated(this, statusCode);
		}

		{
			std::string str = ReasonToString(reason);
			if(m_log->LogActive(m_infoTag))
			{
				*m_log << m_infoTag << "onTerminated. ";
				if (statusCode != 0)
				{
					*m_log << "{ Response code : " << statusCode << ", "
						<< SipUtils::ResponseCodeToString(statusCode) << "} ";
				}
				*m_log << str << iLogW::EndRecord;
			}

			m_regRecord.SetEndCode(statusCode, str.c_str());
		}

	}

	// -------------------------------------------------------------------------------------

	void RemoteParticipant::onRedirected(resip::ClientInviteSessionHandle, const resip::SipMessage& msg)
	{
		if(m_log->LogActive(m_infoTag))
		{
			*m_log  << m_infoTag << "onRedirected: " << MsgToString(msg) << iLogW::EndRecord;
		}

		RemoteParticipant *newParticipant =
			m_dialogSet.CreateNewParticipantForRedirected(*this);

		if (!m_user.IsEmpty()) m_user.Clear()->OnParticipantReplacedBy(this, newParticipant);
	}

	// -------------------------------------------------------------------------------------

	void RemoteParticipant::onAnswer(resip::InviteSessionHandle h, const resip::SipMessage& msg, const resip::SdpContents& sdp)
	{
		if(m_log->LogActive(m_infoTag))
		{
			*m_log  << m_infoTag << "onAnswer: " << MsgToString(msg) << iLogW::EndRecord;
		}


		// Ensure answering party is from answered fork before generating event
		if (!m_dialogSet.isStaleFork(*this))
		{
			InitRemoteSdp(sdp);
		
			Media(m_user)->AdjustRTPStreams(false);
		}

		stateTransition(Connected);  // This is valid until PRACK is implemented
	}

	// -------------------------------------------------------------------------------------

	void RemoteParticipant::onOffer(resip::InviteSessionHandle h, const resip::SipMessage& msg, const resip::SdpContents &offer)
	{
		if(m_log->LogActive(m_infoTag))
		{
			*m_log  << m_infoTag << "onOffer: " << MsgToString(msg) << iLogW::EndRecord;
		}

		InitRemoteSdp(offer);
		
		if (m_state == Connecting && m_inviteSessionHandle.isValid())
		{
			resip::ServerInviteSession* sis = dynamic_cast<resip::ServerInviteSession*>(m_inviteSessionHandle.get());
			if (sis != 0 && !sis->isAccepted()) return;
		}

		bool answerOk = ProvideAnswer(m_state == Replacing /* postAnswerAccept */, false /* postAnswerAlert */);

		if (!answerOk) return;

		if (m_state == Replacing) stateTransition(Connecting);
	}

	// -------------------------------------------------------------------------------------

	void RemoteParticipant::onOfferRequired(resip::InviteSessionHandle h, const resip::SipMessage& msg)
	{
		if(m_log->LogActive(m_infoTag))
		{
			*m_log  << m_infoTag << "onOfferRequired: " << MsgToString(msg) << iLogW::EndRecord;
		}

		// We are being asked to provide SDP to the remote end - we should no longer be considering that
		// remote end wants us to be on hold

		if (m_state == Connecting && !h->isAccepted())
		{
			// If we haven't accepted yet - delay providing the offer until accept is called (this allows time
			// for a localParticipant to be added before generating the offer)
			m_offerRequired = true;
			return;
		}

		provideOffer(m_state == Replacing /* postOfferAccept */);
		if (m_state == Replacing) stateTransition(Connecting);
	}

	// -------------------------------------------------------------------------------------

	void RemoteParticipant::onOfferRejected(resip::InviteSessionHandle, const resip::SipMessage *msg)
	{
		if(m_log->LogActive(m_infoTag))
		{
			*m_log  << m_infoTag << "onOfferRejected: ";
			if (msg != 0) *m_log << MsgToString(*msg);
			*m_log << iLogW::EndRecord;
		}
	}

	// -------------------------------------------------------------------------------------

	void RemoteParticipant::onOfferRequestRejected(resip::InviteSessionHandle h, const resip::SipMessage &msg)
	{
		// InfoLog(<< "onOfferRequestRejected: handle=" << m_base->getParticipantHandle() << ", " << msg.brief());
		ESS_HALT("We never send a request for an offer (ie. Invite with no SDP)");
	}

	// -------------------------------------------------------------------------------------

	void RemoteParticipant::onRemoteSdpChanged(resip::InviteSessionHandle h, const resip::SipMessage &msg, const resip::SdpContents &sdp)
	{
		if(m_log->LogActive(m_infoTag))
		{
			*m_log  << m_infoTag << "onRemoteSdpChanged: " << MsgToString(msg) << iLogW::EndRecord;
		}

		InitRemoteSdp(sdp);
		
		Media(m_user)->AdjustRTPStreams(false);
	}

	// -------------------------------------------------------------------------------------

	void RemoteParticipant::onInfo(resip::InviteSessionHandle, const resip::SipMessage &msg)
	{
		if(m_log->LogActive(m_infoTag))
		{
			*m_log  << m_infoTag << "onInfo: " << MsgToString(msg) << iLogW::EndRecord;
		}

		//assert(0);
	}

	// -------------------------------------------------------------------------------------

	void RemoteParticipant::onInfoSuccess(resip::InviteSessionHandle, const resip::SipMessage &msg)
	{
		// InfoLog(<< "onInfoSuccess: handle=" << m_base->getParticipantHandle() << ", " << msg.brief());
		ESS_HALT("We never send an info request");
	}

	// -------------------------------------------------------------------------------------

	void RemoteParticipant::onInfoFailure(resip::InviteSessionHandle, const resip::SipMessage &msg)
	{
		// InfoLog(<< "onInfoFailure: handle=" << m_base->getParticipantHandle() << ", " << msg.brief());
		ESS_HALT("We never send an info request");
	}

	// -------------------------------------------------------------------------------------

	void RemoteParticipant::Refer(const resip::SipMessage &msg,
		resip::ServerSubscriptionHandle ss)
	{
		m_user->OnRefer(this, msg);

		// Create offer
		resip::SdpContents offer;
		
		Media(m_user)->GenerateOffer(offer);

		// Build the Invite

		resip::SharedPtr<resip::SipMessage> newInviteMsg;
		if(ss.isValid())
			newInviteMsg = m_dum.makeInviteSessionFromRefer(msg, ss->getHandle(), &offer, &m_dialogSet);
		else
		{
			ESS_ASSERT (m_conversationProfile != 0);
			newInviteMsg = m_dum.makeInviteSessionFromRefer(msg, m_conversationProfile->UserProfilePtr(), &offer, &m_dialogSet);
		}

		m_dum.send(newInviteMsg);

		// Set RTP stack to listen
		Media(m_user)->AdjustRTPStreams(true);

	}

	// -------------------------------------------------------------------------------------

	void RemoteParticipant::onRefer(resip::InviteSessionHandle is, resip::ServerSubscriptionHandle ss, const resip::SipMessage &msg)
	{
		if(m_log->LogActive(m_infoTag))
		{
			*m_log  << m_infoTag << "onRefer: " << MsgToString(msg) << iLogW::EndRecord;
		}

		// Accept the Refer
		ss->send(ss->accept(202 /* Refer Accepted */));

		// Create new Participant - but use same participant handle
		RemoteParticipant *participant =
			m_conversationManager.CreateUACOriginalRemoteParticipant(m_conversationProfile,
			m_dialogSet.getForkSelectMode());

		m_user.Clear()->OnParticipantReplacedBy(this, participant);

		participant->Refer(msg, ss);
	}

	// -------------------------------------------------------------------------------------

	void RemoteParticipant::doReferNoSub(const resip::SipMessage& msg)
	{
		// Create new Participant - but use same participant handle
		RemoteParticipant *participant =
			m_conversationManager.CreateUACOriginalRemoteParticipant(m_conversationProfile,
			m_dialogSet.getForkSelectMode());

		m_user.Clear()->OnParticipantReplacedBy(this, participant);

		participant->Refer(msg);
	}

	// -------------------------------------------------------------------------------------

	void RemoteParticipant::onReferNoSub(resip::InviteSessionHandle is, const resip::SipMessage& msg)
	{
		if(m_log->LogActive(m_infoTag))
		{
			*m_log  << m_infoTag << "onReferNoSub: " << MsgToString(msg) << iLogW::EndRecord;
		}

		try
		{
			// Accept the Refer
			is->acceptReferNoSub(202 /* Refer Accepted */);

			doReferNoSub(msg);
		}
		catch(resip::BaseException &e)
		{
			if(m_log->LogActive(m_warningTag))
			{
				*m_log << m_warningTag << "onReferNoSub exception:" << SipUtils::ToString(e.getMessage()) << iLogW::EndRecord;
			}
		}
	}

	// -------------------------------------------------------------------------------------

	void RemoteParticipant::onReferAccepted(resip::InviteSessionHandle, resip::ClientSubscriptionHandle, const resip::SipMessage& msg)
	{
		if(m_log->LogActive(m_infoTag))
		{
			*m_log  << m_infoTag << "onReferAccepted: " << MsgToString(msg) << iLogW::EndRecord;
		}
	}

	// -------------------------------------------------------------------------------------

	void RemoteParticipant::onReferRejected(resip::InviteSessionHandle, const resip::SipMessage& msg)
	{
		if(m_log->LogActive(m_infoTag))
		{
			*m_log  << m_infoTag << "onReferRejected: " << MsgToString(msg) << iLogW::EndRecord;
		}

		if (msg.isResponse() && m_state == Redirecting)
		{
			m_user->OnParticipantRedirectFailure(this, msg.header(resip::h_StatusLine).responseCode());
			stateTransition(Connected);
		}
	}

	// -------------------------------------------------------------------------------------

	void RemoteParticipant::onMessage(resip::InviteSessionHandle, const resip::SipMessage& msg)
	{
		if(m_log->LogActive(m_infoTag))
		{
			*m_log  << m_infoTag << "onMessage: " << MsgToString(msg) << iLogW::EndRecord;
		}
	}

	// -------------------------------------------------------------------------------------

	void RemoteParticipant::onMessageSuccess(resip::InviteSessionHandle, const resip::SipMessage& msg)
	{
		if(m_log->LogActive(m_infoTag))
		{
			*m_log  << m_infoTag << "onMessageSuccess: " << MsgToString(msg) << iLogW::EndRecord;
		}
	}

	// -------------------------------------------------------------------------------------

	void RemoteParticipant::onMessageFailure(resip::InviteSessionHandle, const resip::SipMessage& msg)
	{
		if(m_log->LogActive(m_infoTag))
		{
			*m_log  << m_infoTag << "onMessageFailure: " << MsgToString(msg) << iLogW::EndRecord;
		}
	}

	// -------------------------------------------------------------------------------------

	void RemoteParticipant::onForkDestroyed(resip::ClientInviteSessionHandle)
	{
		if(m_log->LogActive(m_infoTag))
		{
			*m_log  << m_infoTag << "onForkDestroyed"<< iLogW::EndRecord;
		}
	}

	// -------------------------------------------------------------------------------------
	// IClientSubscriptionHandler

	void RemoteParticipant::onUpdatePending(resip::ClientSubscriptionHandle h, const resip::SipMessage& notify, bool outOfOrder)
	{
		if(m_log->LogActive(m_infoTag))
		{
			*m_log  << m_infoTag << "onUpdatePending(ClientSub): " << MsgToString(notify) << iLogW::EndRecord;
		}

		if (notify.exists(resip::h_Event) && notify.header(resip::h_Event).value() == "refer")
		{
			h->acceptUpdate();
			processReferNotify(notify);
		}
		else
		{
			h->rejectUpdate(400, resip::Data("Only notifies for refers are allowed."));
		}
	}

	// -------------------------------------------------------------------------------------

	void RemoteParticipant::onUpdateActive(resip::ClientSubscriptionHandle h, const resip::SipMessage& notify, bool outOfOrder)
	{
		if(m_log->LogActive(m_infoTag))
		{
			*m_log << m_infoTag << "onUpdateActive(ClientSub): " << MsgToString(notify) << iLogW::EndRecord;
		}

		if (notify.exists(resip::h_Event) && notify.header(resip::h_Event).value() == "refer")
		{
			h->acceptUpdate();
			processReferNotify(notify);
		}
		else
		{
			h->rejectUpdate(400, resip::Data("Only notifies for refers are allowed."));
		}
	}

	// -------------------------------------------------------------------------------------

	void RemoteParticipant::onUpdateExtension(resip::ClientSubscriptionHandle h, const resip::SipMessage& notify, bool outOfOrder)
	{
		if(m_log->LogActive(m_infoTag))
		{
			*m_log  << m_infoTag << "onUpdateExtension(ClientSub): " << MsgToString(notify) << iLogW::EndRecord;
		}

		if (notify.exists(resip::h_Event) && notify.header(resip::h_Event).value() == "refer")
		{
			h->acceptUpdate();
			processReferNotify(notify);
		}
		else
		{
			h->rejectUpdate(400, resip::Data("Only notifies for refers are allowed."));
		}
	}

	// -------------------------------------------------------------------------------------

	void RemoteParticipant::onTerminated(resip::ClientSubscriptionHandle h, const resip::SipMessage* notify)
	{
		if (notify == 0)
		{
			// Timed out waiting for notify
			if(m_log->LogActive(m_infoTag))
			{
				*m_log  << m_infoTag << "onTerminated(ClientSub)" << iLogW::EndRecord;
			}

			if (m_state == Redirecting)
			{
				m_user->OnParticipantRedirectFailure(this, 408);
				stateTransition(Connected);
			}
			return;
		}

		if(m_log->LogActive(m_infoTag))
		{
			*m_log  << m_infoTag << "onTerminated(ClientSub): " << MsgToString(*notify) << iLogW::EndRecord;
		}

		if (notify->isRequest() && notify->exists(resip::h_Event) && notify->header(resip::h_Event).value() == "refer")
		{
			// Note:  Final notify is sometimes only passed in the onTerminated callback
			processReferNotify(*notify);
		}
		else if (notify->isResponse() && m_state == Redirecting)
		{
			m_user->OnParticipantRedirectFailure(this, notify->header(resip::h_StatusLine).responseCode());
			stateTransition(Connected);
		}

	}

	// -------------------------------------------------------------------------------------

	void RemoteParticipant::onNewSubscription(resip::ClientSubscriptionHandle h, const resip::SipMessage& notify)
	{
		if(m_log->LogActive(m_infoTag))
		{
			*m_log  << m_infoTag << "onNewSubscription(ClientSub): " << MsgToString(notify) << iLogW::EndRecord;
		}
	}

	// -------------------------------------------------------------------------------------

	int RemoteParticipant::onRequestRetry(resip::ClientSubscriptionHandle h, int retryMinimum, const resip::SipMessage& notify)
	{
		if(m_log->LogActive(m_infoTag))
		{
			*m_log  << m_infoTag << "onRequestRetry(ClientSub): " << MsgToString(notify) << iLogW::EndRecord;
		}

		return -1;
	}

	// -------------------------------------------------------------------------------------
	// IRemoteParticipantExt

	std::string RemoteParticipant::NameParticipant() const
	{
		return m_log->NameStr();
	}

	// -------------------------------------------------------------------------------------

	void RemoteParticipant::LinkUser(Utils::SafeRef<IRemoteParticipantUser> user)
	{
		ESS_ASSERT(m_user.IsEmpty() || m_user == user );

		m_user = user;
		
		if(m_log->LogActive(m_infoTag) && !m_user->CallUserName().empty())
		{
			*m_log << m_infoTag << "Linked to user: "
				<< m_user->CallUserName()
				<<  iLogW::EndRecord;
		}
	}

	// -------------------------------------------------------------------------------------

	void RemoteParticipant::RelatedParticipants(RelatedParticipantList &list)
	{
		ESS_HALT ("Unexpected");
	}

	// -------------------------------------------------------------------------------------

	boost::shared_ptr<ConversationProfile> RemoteParticipant::GetConversationProfile() const
	{
		return m_conversationProfile;
	}

	// -------------------------------------------------------------------------------------

	void RemoteParticipant::MediaObjectIsReady()
	{
		if(m_log->LogActive(m_warningTag))
		{
			*m_log << m_infoTag << "Media ready" << iLogW::EndRecord;
		}
	}

	// -------------------------------------------------------------------------------------

	boost::shared_ptr<iReg::CallRecordWrapper> RemoteParticipant::CallRecord()
	{
		return m_callRecord;
	}

	// -------------------------------------------------------------------------------------

	void RemoteParticipant::AlertParticipant(bool earlyFlag)
	{
		if (m_state != Connecting || !m_inviteSessionHandle.isValid())
		{
			if(m_log->LogActive(m_warningTag))
			{
				*m_log << m_warningTag<< "alert called in invalid state: " << StateToString(m_state) << iLogW::EndRecord;
			}
			return;
		}

		resip::ServerInviteSession* sis =
			dynamic_cast<resip::ServerInviteSession*>(m_inviteSessionHandle.get());

		if (sis == 0 || sis->isAccepted()) return;

		if (!earlyFlag)
		{
			sis->provisional(180, earlyFlag);
			return;
		}

		bool answerOk = ProvideAnswer(false /* postAnswerAccept */, true /* postAnswerAlert */);

		// if (!answerOk) return;
	}

	// -------------------------------------------------------------------------------------

	void RemoteParticipant::AnswerParticipant()
	{

		// Accept Pending OOD Refer if required
		if (m_state == PendingOODRefer)
		{
			acceptPendingOODRefer();
			return;
		}

		if (m_state != Connecting || !m_inviteSessionHandle.isValid())
		{
			if(m_log->LogActive(m_warningTag))
			{
				*m_log << m_warningTag << "accept called in invalid state: " << StateToString(m_state) << iLogW::EndRecord;
			}

			return;
		}

		// Accept SIP call if required
		resip::ServerInviteSession* sis =
			dynamic_cast<resip::ServerInviteSession*>(m_inviteSessionHandle.get());

		if (sis == 0 || sis->isAccepted()) return;

		if (m_offerRequired)
		{
			provideOffer(true /* postOfferAccept */);
		}
		else
		{
			bool answerOk = ProvideAnswer(true /* postAnswerAccept */, false /* postAnswerAlert */);
			
			if (!answerOk) return;
		}

		stateTransition(Accepted);
	}

	// -------------------------------------------------------------------------------------

	void RemoteParticipant::UpdateMedia()
	{
		if (m_state != Connected) return;

		if(m_inviteSessionHandle.isValid())
		{
			provideOffer(false /* postOfferAccept */);
			stateTransition(Reinviting);
		}
	}

	// -------------------------------------------------------------------------------------

	void RemoteParticipant::DestroyParticipant(unsigned int rejectCode,
		const std::string &description)
	{
		if (!m_user.IsEmpty()) m_user.Clear();

		if (rejectCode == 0) rejectCode = 480; // 480 - Temporarily Unavailable

		{
			if(m_log->LogActive(m_infoTag))
			{
				*m_log << m_infoTag << "Destroy call. Cause " << rejectCode;
				if (!description.empty()) *m_log << " " << description;
				*m_log << iLogW::EndRecord;
			}
			m_regRecord.SetEndCode(rejectCode, description.c_str());
		}
		
		try
		{
			if (m_state == Terminating) return;

			// Reject UAS Invite Session if required
			if (m_state == Connecting && m_inviteSessionHandle.isValid())
			{
				resip::ServerInviteSession* sis =
					dynamic_cast<resip::ServerInviteSession*>(m_inviteSessionHandle.get());

				if (sis != 0 && !sis->isAccepted())
				{
					sis->reject(rejectCode);
					return;	
				}
			}

			// Reject Pending OOD Refer request if required
			if (m_state == PendingOODRefer)
			{
				rejectPendingOODRefer(rejectCode);
				return;
			}

			stateTransition(Terminating);
			if (m_inviteSessionHandle.isValid())
			{
				m_inviteSessionHandle->end();
			}
			else
			{
				m_dialogSet.end();
			}
		}
		catch(resip::BaseException &e)
		{
			if(m_log->LogActive(m_warningTag))
			{
				*m_log << m_warningTag << "destroy exception: " << SipUtils::ToString(e.getMessage()) << iLogW::EndRecord;
			}
		}
	}

	// -------------------------------------------------------------------------------------

	void RemoteParticipant::RedirectParticipant(const resip::NameAddr &par)
	{
		ESS_ASSERT(m_pendingRequest.empty());

		resip::NameAddr destination =
			UpdateDestinationAddr(par, m_conversationProfile->UserProfile());

		if (m_state == PendingOODRefer)
		{
			redirectPendingOODRefer(destination);
			return;
		}

		if (!m_inviteSessionHandle.isValid())
		{
			m_pendingRequest = boost::bind(&T::RedirectParticipant, this, destination);
			return;
		}

		if (m_state != Connecting &&
			m_state != Accepted &&
			m_state != Connected &&
			m_state != Reinviting)
		{
			m_pendingRequest = boost::bind(&T::RedirectParticipant, this, destination);
			return;
		}

		resip::ServerInviteSession* sis =
			dynamic_cast<resip::ServerInviteSession*>(m_inviteSessionHandle.get());
				
		// If this is a UAS session and we haven't sent a final response yet - then redirect via 302 response
		if (sis !=0 && !sis->isAccepted() && ( m_state == Connecting || m_state == Reinviting))
		{
			resip::NameAddrs destinations;
			destinations.push_back(destination);

			m_user->OnParticipantRedirectSuccess(this);

			sis->redirect(destinations);

			return;
		}

		if (m_inviteSessionHandle->isConnected()) // redirect via blind transfer
		{
			m_inviteSessionHandle->refer(destination, true /* refersub */);
			stateTransition(Redirecting);
			return;
		}

		m_pendingRequest = boost::bind(&T::RedirectParticipant, this, destination);
	}

	// -------------------------------------------------------------------------------------

	void RemoteParticipant::RedirectToParticipant(IRemoteParticipant &dst)
	{
		ESS_ASSERT(m_pendingRequest.empty());

		RemoteParticipant *destParticipant = dynamic_cast<RemoteParticipant*>(&dst);

		ESS_ASSERT(destParticipant != 0 && destParticipant != this);

		ESS_ASSERT(&m_conversationManager == &destParticipant->m_conversationManager);

		RedirectToParticipantImpl(destParticipant->getInviteSessionHandle());
	}

	// -------------------------------------------------------------------------------------

	void RemoteParticipant::RedirectToParticipantImpl(resip::InviteSessionHandle destHandle)
	{
		if (!destHandle.isValid())
		{
			if(m_log->LogActive(m_warningTag))
			{
				*m_log << "redirectToParticipant error: destParticipant has no valid resip::InviteSession" << iLogW::EndRecord;
			}

			m_user->OnParticipantRedirectFailure(this, 406 /* Not Acceptable */);
			return;
		}

		if (!m_inviteSessionHandle.isValid())
		{
			m_pendingRequest = boost::bind(&T::RedirectToParticipantImpl, this, destHandle);
			return;
		}

		if (m_state != Connecting &&
			m_state != Accepted &&
			m_state != Connected &&
			m_state != Reinviting)
		{
			m_pendingRequest = boost::bind(&T::RedirectToParticipantImpl, this, destHandle);
			return;
		}


		resip::ServerInviteSession* sis =
			dynamic_cast<resip::ServerInviteSession*>(m_inviteSessionHandle.get());

		// If this is a UAS session and we haven't sent a final response yet - then redirect via 302 response

		if (sis != 0 && !sis->isAccepted() && (m_state == Connecting || m_state ==  Reinviting))
		{
			resip::NameAddrs destinations;
			destinations.push_back(destHandle->peerAddr());
			m_user->OnParticipantRedirectSuccess(this);
			sis->redirect(destinations);
			return;
		}

		if (m_inviteSessionHandle->isConnected()) // redirect via attended transfer (with replaces)
		{
			m_inviteSessionHandle->refer(resip::NameAddr(destHandle->peerAddr().uri()) /* remove tags */, destHandle /* session to replace)  */, true /* refersub */);
			stateTransition(Redirecting);
			return;
		}

		m_pendingRequest = boost::bind(&T::RedirectToParticipantImpl, this, destHandle);
	}

	// -------------------------------------------------------------------------------------

	void RemoteParticipant::InitiateRemoteCall(const resip::NameAddr &destination)
	{
		resip::SdpContents offer;

		Media(m_user)->GenerateOffer(offer);

		resip::NameAddr dstAddr = UpdateDestinationAddr(destination,
			m_conversationProfile->UserProfile());

		m_regRecord.SetCalledInfo(SipUtils::ToQString(dstAddr));

		m_regRecord.SetCallingInfo(
			SipUtils::ToQString(m_conversationProfile->UserProfilePtr()->getDefaultFrom()));

		resip::SharedPtr<resip::SipMessage> invitemsg = m_dum.makeInviteSession(
			dstAddr,
			m_conversationProfile->UserProfilePtr(),
			&offer,
			&m_dialogSet);

		m_dum.send(invitemsg);

		// Adjust RTP streams
		Media(m_user)->AdjustRTPStreams(true);

	}

	// -------------------------------------------------------------------------------------

	void RemoteParticipant::doProvideOfferAnswer(bool offer,
		const resip::SdpContents &sdp,
		bool postOfferAnswerAccept,
		bool postAnswerAlert)
	{
		ESS_ASSERT(m_inviteSessionHandle.isValid());

		if (offer)
		{
			m_inviteSessionHandle->provideOffer(sdp);
		}
		else
		{
			m_inviteSessionHandle->provideAnswer(sdp);
		}

		// Adjust RTP Streams
		Media(m_user)->AdjustRTPStreams(offer);

		// Do Post Answer Operations
		resip::ServerInviteSession* sis =
			dynamic_cast<resip::ServerInviteSession*>(m_inviteSessionHandle.get());

		if (sis == 0) return;

		if (postAnswerAlert)
		{
			sis->provisional(180,true);
		}
		if (postOfferAnswerAccept)
		{
			sis->accept();
		}
	}

	// -------------------------------------------------------------------------------------

	void RemoteParticipant::InitRemoteSdp(const resip::SdpContents &offer)
	{
		boost::shared_ptr<resip::SdpContents>
			val(new resip::SdpContents(offer));

		Media(m_user)->InitRemoteSdp(val);
	}

	// -------------------------------------------------------------------------------------

	std::string RemoteParticipant::MsgToString(const resip::SipMessage &msg)
	{
		return SipUtils::ToString(m_conversationProfile->Misc.ResipLogMode(), msg);
	}
}


