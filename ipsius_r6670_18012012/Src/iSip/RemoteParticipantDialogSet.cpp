#include "stdafx.h"
#include "iSip/undeferrno.h"

#include "RemoteParticipantDialogSet.h"
#include "ConversationManager.h"
#include "RemoteParticipant.h"
#include "SipUtils.h"
#include "Sdp/SdpHelperResip.h"
#include "Sdp/Sdp.h"

#include <rutil/Random.hxx>
#include <rutil/DnsUtil.hxx>
#include <resip/stack/SipFrag.hxx>
#include <resip/dum/DialogUsageManager.hxx>
#include <resip/dum/ServerInviteSession.hxx>

#include "Utils/AtomicTypes.h"
#include "Utils/IntToString.h"
#include "Utils/ErrorsSubsystem.h"


namespace
{
	std::string GenerateDialogSetName()
	{
		static Utils::AtomicInt GNumber;

		return "InviteDialogSet_" + Utils::IntToString(GNumber.FetchAndAdd(1));
	}
}

namespace iSip
{

	RemoteParticipantDialogSet::RemoteParticipantDialogSet(resip::DialogUsageManager &dum,
		ConversationManager& conversationManager,
		iLogW::ILogSessionCreator &logCreator,
		ParticipantForkSelectMode::Value forkSelectMode) :
		AppDialogSet(dum),
		m_log(logCreator.CreateSession(GenerateDialogSetName(), true)),
		m_infoTag(m_log->RegisterRecordKind(L"Info", true)),
		m_warningTag(m_log->RegisterRecordKind(L"Warning", true)),
		m_conversationManager(conversationManager),
		mForkSelectMode(forkSelectMode)
	{
		if(m_log->LogActive(m_infoTag))
		{
			*m_log << m_infoTag << "Created" << iLogW::EndRecord;
		}
	}

	// -------------------------------------------------------------------------------------

	RemoteParticipantDialogSet::~RemoteParticipantDialogSet()
	{
		if(m_log->LogActive(m_infoTag))
		{
			*m_log << m_infoTag << "Deleted" << iLogW::EndRecord;
		}
	}

	// -------------------------------------------------------------------------------------

	Utils::SafeRef<IRemoteParticipant> RemoteParticipantDialogSet::CreateOutgoingParticipant(
		Utils::SafeRef<IRemoteParticipantUser> user,
		boost::shared_ptr<IRemoteParticipantCreator::Parameters> parameters)
	{
		ESS_ASSERT(m_uacOriginalRemoteParticipant.get() == 0);

		ESS_ASSERT(m_dialogs.empty());

		ESS_ASSERT(!user.IsEmpty());

		ESS_ASSERT(parameters->ConversationProfile != 0);

		m_conversationProfile = parameters->ConversationProfile;

		RemoteParticipant *participant = new RemoteParticipant(
			m_conversationManager,
			mDum,
			*this,
			*m_log,
			boost::bind(&T::RemoteParticipantRegistrator, this, _1, _2),
			m_conversationProfile,
			user,
			parameters->GetDestination());

		m_uacOriginalRemoteParticipant.reset(participant);

		m_activeRemoteParticipant = participant->SelfRef();

		return participant;
	}

	// -------------------------------------------------------------------------------------

	RemoteParticipant* RemoteParticipantDialogSet::CreateUACOriginalRemoteParticipant(
		boost::shared_ptr<ConversationProfile>	conversationProfile)
	{
		ESS_ASSERT(m_uacOriginalRemoteParticipant.get() == 0);
		ESS_ASSERT(m_conversationProfile == 0);

		m_conversationProfile = conversationProfile;

		RemoteParticipant *participant = CreateUasOrForkParticipant();

		m_uacOriginalRemoteParticipant.reset(participant);

		m_activeRemoteParticipant = participant->SelfRef();

		return participant;
	}

	// -------------------------------------------------------------------------------------

	void RemoteParticipantDialogSet::SetUACConnected(const resip::DialogId &dialogId,
		Utils::WeakRef<RemoteParticipant&> ref)
	{
		ESS_ASSERT(m_connectedRemoteParticipant.Empty());

		ESS_ASSERT(!ref.Empty());

		m_connectedRemoteParticipant = ref;
		m_activeRemoteParticipant	 = ref;

		if (mForkSelectMode != ParticipantForkSelectMode::Automatic) return;

		for(Dialogs::iterator i = m_dialogs.begin();
			i != m_dialogs.end(); ++i)
		{
			ESS_ASSERT (*i != 0);
			if(*i == &m_activeRemoteParticipant.Value()) continue;

			if(m_log->LogActive(m_infoTag))
			{
				*m_log << m_infoTag  << "Connected to forked leg "
					<< SipUtils::ToString(dialogId)
					<< " - stale dialog "
					<< SipUtils::ToString((*i)->DialogId())
					<< " and related conversation(s) will be ended."
					<< iLogW::EndRecord;
			}

			(*i)->destroyParticipant();
		}
	}

	// -------------------------------------------------------------------------------------

	bool RemoteParticipantDialogSet::IsActiveRemoteParticipant(const IRemoteParticipant &val)  const
	{
		return !m_activeRemoteParticipant.Empty() &&
			&m_activeRemoteParticipant.Value() == &val;
	}

	// -------------------------------------------------------------------------------------

	void RemoteParticipantDialogSet::SetActiveRemoteParticipant(
		Utils::WeakRef<RemoteParticipant&> activeRemoteParticipant)
	{
		m_activeRemoteParticipant = activeRemoteParticipant;
	}

	// -------------------------------------------------------------------------------------

	RemoteParticipant *RemoteParticipantDialogSet::CreateNewParticipantForRedirected(
		const RemoteParticipant &redirectedParticipant)
	{
		RemoteParticipant *participant = CreateUasOrForkParticipant();

		if (m_uacOriginalRemoteParticipant.get() == &redirectedParticipant ||
			m_uacOriginalRemoteParticipant.get() == 0) m_uacOriginalRemoteParticipant.reset(participant);

		return participant;
	}

	// -------------------------------------------------------------------------------------
	// resip::AppDialogSet

	resip::SharedPtr<resip::UserProfile>
		RemoteParticipantDialogSet::selectUASUserProfile(const resip::SipMessage& msg)
	{
		if (m_conversationProfile == 0) m_conversationProfile = m_conversationManager.GetIncomingConversationProfile(msg);

		return (m_conversationProfile == 0) ?
			resip::SharedPtr<resip::UserProfile>() :
			m_conversationProfile->UserProfilePtr();
	}

	// -------------------------------------------------------------------------------------

	resip::AppDialog* RemoteParticipantDialogSet::createAppDialog(const resip::SipMessage &msg)
	{

		if (m_uacOriginalRemoteParticipant.get() == 0 &&
			m_dialogs.empty())
		{
			// no original participant and no dialog
			// UAS DialogSet
			// Note:  !slg! DialogId is not quite right here, since there is no To Tag on the INVITE

			RemoteParticipant *participant = CreateUasOrForkParticipant();

			m_activeRemoteParticipant = participant->SelfRef();

			return participant;
		}

		if (m_uacOriginalRemoteParticipant.get() != 0)
		{
			// move participant ownership to dum
			return m_uacOriginalRemoteParticipant.release();
		}
		
		// forking occured and we now have multiple dialogs in this dialog set

		RemoteParticipant *newParticipant = CreateUasOrForkParticipant();

		if(m_log->LogActive(m_infoTag))
		{
			*m_log << m_infoTag << "Forking occurred for original UAC participant '"
				<< m_uacOriginalRemoteParticipant->NameParticipant()
				<< "' this is leg number " << m_dialogs.size()
				<< " new participant " << newParticipant->NameParticipant()
				<< iLogW::EndRecord;
		}

		RemoteParticipant *participant = (!m_activeRemoteParticipant.Empty()) ?
			&m_activeRemoteParticipant.Value() :
			m_dialogs.front();

		ESS_ASSERT (participant != 0);

		participant->OnForkingOccured(participant, newParticipant, msg);

		return newParticipant;
	}

	// -------------------------------------------------------------------------------------
	// resip::DialogSetHandler

	void RemoteParticipantDialogSet::onTrying(resip::AppDialogSetHandle, const resip::SipMessage& msg)
	{
		if (isUACConnected() || m_activeRemoteParticipant.Empty()) return;
		
		if(m_log->LogActive(m_infoTag))
		{
			*m_log << m_infoTag << "onTrying: '" << m_activeRemoteParticipant.Value().NameParticipant()
				<< "' " << MsgToString(msg)
				<< iLogW::EndRecord;
		}
	}

	// -------------------------------------------------------------------------------------

	void RemoteParticipantDialogSet::onNonDialogCreatingProvisional(resip::AppDialogSetHandle,
		const resip::SipMessage& msg)
	{
		ESS_ASSERT(msg.header(resip::h_StatusLine).responseCode() != 100);

		// It possible to get a provisional from another fork after receiving a 200 - if so, don't generate an event

		if (!isUACConnected() && !m_activeRemoteParticipant.Empty())
		{
			m_activeRemoteParticipant.Value().OnNonDialogCreatingProvisional(msg);
		}
	}


	// -------------------------------------------------------------------------------------
/*
	int RemoteParticipantDialogSet::getLocalRTPPort()
	{
		if (mLocalRTPPort != 0 || mAllocateLocalRTPPortFailed) return mLocalRTPPort;

		bool isUAC = false;

		// mLocalRTPPort = mConversationManager.allocateRTPPort();

		if (mLocalRTPPort == 0)
		{
			WarningLog(<< "Could not allocate a free RTP port for RemoteParticipantDialogSet!");
			mAllocateLocalRTPPortFailed = true;
			return 0;
		}

		InfoLog(<< "Port allocated: " << mLocalRTPPort);

		// Create localBinding Tuple - note:  transport may be changed depending on NAT traversal mode
		reTurn::StunTuple localBinding(reTurn::StunTuple::UDP,
			asio::ip::address::from_string(m_conversationProfile->sessionCaps().session().connection().getAddress().c_str()),
			mLocalRTPPort);

		// Set other Srtp properties
		//mLocalSrtpSessionKey = Random::getCryptoRandom(SRTP_MASTER_KEY_LEN);
		mLocalSrtpSessionKey = resip::Random::getCryptoRandom(30); //  in srtp.h SRTP_MASTER_KEY_LEN = 30

		// InfoLog( << "RTP Port allocated=" << mLocalRTPPort << " (sipXmediaConnectionId=" << mMediaConnectionId << ", BridgePort=" << mConnectionPortOnBridge << ", ret=" << ret << ")");

		return mLocalRTPPort;
	}

	// -------------------------------------------------------------------------------------

	void RemoteParticipantDialogSet::processMediaStreamReadyEvent(
		const reTurn::StunTuple& rtpTuple, const reTurn::StunTuple& rtcpTuple)
	{

	   InfoLog( << "processMediaStreamReadyEvent: rtpTuple=" << rtpTuple << " rtcpTuple=" << rtcpTuple);
	   mRtpTuple = rtpTuple;
	   mRtcpTuple = rtcpTuple;   // Check if we had operations pending on the media stream being ready

	   if (mPendingInvite.get() != 0)
	   {
		  // Pending Invite Request
		  doSendInvite(mPendingInvite);
		  mPendingInvite.reset();
	   }

	   if (mPendingOfferAnswer.mSdp.get() != 0)
	   {
		  // Pending Offer or Answer
		  doProvideOfferAnswer(mPendingOfferAnswer.mOffer,
							   mPendingOfferAnswer.mSdp,
							   mPendingOfferAnswer.mInviteSessionHandle,
							   mPendingOfferAnswer.mPostOfferAnswerAccept,
							   mPendingOfferAnswer.mPostAnswerAlert);
		  assert(mPendingOfferAnswer.mSdp.get() == 0);
	   }
	}

	// -------------------------------------------------------------------------------------

	void RemoteParticipantDialogSet::processMediaStreamErrorEvent(unsigned int errorCode)
	{
	   InfoLog( << "processMediaStreamErrorEvent, error=" << errorCode);

	   // Note:  in the case of an initial invite we must issue the invite in order for dum to cleanup state
	   //         properly - this is not ideal, since it may cause endpoint phone device to ring a little
	   //         before receiving the cancel
	   if (mPendingInvite.get() != 0)
	   {
		  // Pending Invite Request - Falling back to using local address/port - but then end() immediate
		  doSendInvite(mPendingInvite);
		  mPendingInvite.reset();
	   }

	   // End call
	   if (mNumDialogs > 0)
	   {
		  std::map<resip::DialogId, RemoteParticipant*>::iterator it;
		  for(it = mDialogs.begin(); it != mDialogs.end(); it++)
		  {
			 it->second->`Participant();
		  }
	   }
	   else
	   {
		  end();
	   }
	}
*/

	// -------------------------------------------------------------------------------------

	std::string RemoteParticipantDialogSet::MsgToString(const resip::SipMessage &msg)
	{
		return SipUtils::ToString(m_conversationProfile->Misc.ResipLogMode(), msg);
	}

	// -------------------------------------------------------------------------------------

	void RemoteParticipantDialogSet::RemoteParticipantRegistrator(RemoteParticipant *item, bool addToList)
	{
		if (addToList)
		{
			m_dialogs.push_back(item);
			return;
		}

		Dialogs::iterator i = std::find(m_dialogs.begin(), m_dialogs.end(), item);

		ESS_ASSERT (i != m_dialogs.end());

		m_dialogs.erase(i);

		if (item == m_uacOriginalRemoteParticipant.get()) m_uacOriginalRemoteParticipant.release();

		// If we have no more dialogs and we never went connected - make sure we cancel the Invite transaction
		// if(m_dialogs.empty() && !isUACConnected()) end();
	}

	// -------------------------------------------------------------------------------------

	RemoteParticipant *RemoteParticipantDialogSet::CreateUasOrForkParticipant()
	{
		return new RemoteParticipant(
			m_conversationManager,
			mDum,
			*this,
			*m_log,
			boost::bind(&T::RemoteParticipantRegistrator, this, _1, _2),
			m_conversationProfile);
	}

}
