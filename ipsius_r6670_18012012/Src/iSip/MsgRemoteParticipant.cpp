#include "stdafx.h"
#include "iSip/undeferrno.h"
#include "MsgRemoteParticipant.h"
#include "RemoteParticipantDialogSet.h"
#include "RemoteParticipant.h"
#include "ConversationManager.h"
#include "Sdp/Sdp.h"

namespace iSip
{
	class MsgRemoteParticipant::MediaWrapper : boost::noncopyable,
		public iSip::ISipMedia
	{
		typedef MediaWrapper T;
	public:
		
		MediaWrapper(Utils::SafeRef<iSip::IRemoteParticipantUser> user,
			NatIpConverter natIpConverter) :
			m_user(user),
			m_natIpConverter(natIpConverter)
		{
			if(m_natIpConverter.empty() || m_user.IsEmpty()) return;
			
			iSip::ISipMedia *media = m_user->Media();

			if (media != 0) media->SetupNatIpConverter(m_natIpConverter);
		}

		void LinkUser(Utils::SafeRef<iSip::IRemoteParticipantUser> user)
		{
			ESS_ASSERT((m_user.IsEmpty() && !user.IsEmpty()) || m_user == user);

			m_user = user;
			
			iSip::ISipMedia *media = m_user->Media();

			ESS_ASSERT (media != 0);

			media->SetupNatIpConverter(m_natIpConverter);

			if(m_remoteSdp != 0) media->InitRemoteSdp(m_remoteSdp);

		}

		void UnlinkUser(Utils::SafeRef<iSip::IRemoteParticipantUser> user)
		{
			ESS_ASSERT(m_user == user || m_user.IsEmpty());

			if (!m_user.IsEmpty()) m_user.Clear();
		}

		bool IsReady() const
		{
			return !m_user.IsEmpty() && m_user->Media() != 0;
		}

	// iSip::ISipMedia
	public:

		void SetupNatIpConverter(boost::function<resip::Data (const resip::Data&)> converter)
		{
			m_natIpConverter = converter;

			if(IsReady()) m_user->Media()->SetupNatIpConverter(m_natIpConverter);
		}

		void InitRemoteSdp(boost::shared_ptr<resip::SdpContents> offer)
		{
			m_remoteSdp = offer;

			if(IsReady()) m_user->Media()->InitRemoteSdp(m_remoteSdp);
		}

		void AdjustRTPStreams(bool isOffer)
		{
			if(IsReady()) m_user->Media()->AdjustRTPStreams(isOffer);
		}

		bool GenerateAnswer(resip::SdpContents &answer) const
		{
			return (!IsReady()) ?
				false :
				m_user->Media()->GenerateAnswer(answer);
		}

		void GenerateOffer(resip::SdpContents &offer) const
		{
			if(IsReady()) m_user->Media()->GenerateOffer(offer);
		}

	private:

		Utils::SafeRef<iSip::IRemoteParticipantUser> m_user;
		NatIpConverter m_natIpConverter;
		boost::shared_ptr<resip::SdpContents> m_remoteSdp;
	};
}


namespace iSip
{

	MsgRemoteParticipant::MsgRemoteParticipant(iCore::MsgThread &thread,
		ConversationManager &conversationManager,
		boost::function<void (IRemoteParticipant*, bool)> registrator,
		Utils::SafeRef<IRemoteParticipantUser> user,
		boost::shared_ptr<IRemoteParticipantCreator::Parameters> parameters) :
		iCore::MsgObject(thread),
		m_conversationManager(conversationManager),
		m_registrator(registrator),
		m_user(user)
	{
		m_isDestroyed = false;
		m_registrator(this, true);

		PutFunctor(boost::bind(&T::CreateParticipant,
			this, boost::ref(conversationManager), parameters));
	}

	// -------------------------------------------------------------------------------------

	MsgRemoteParticipant::MsgRemoteParticipant(iCore::MsgThread &thread,
		ConversationManager &conversationManager,
		boost::function<void (IRemoteParticipant*, bool)> registrator,
		Utils::SafeRef<IRemoteParticipant> remoteParticipant) :
		MsgObject(thread),
		m_conversationManager(conversationManager),
		m_registrator(registrator)
	{
		m_isDestroyed = false;
		m_registrator(this, true);

		boost::shared_ptr<ConversationProfile> conversationProfile =
			remoteParticipant->GetConversationProfile();

		ESS_ASSERT(conversationProfile != 0);

		m_media.reset(new MediaWrapper(m_user,
			conversationProfile->GetNatIpConverter()));

		m_remoteParticipant = remoteParticipant;

		m_remoteParticipant->LinkUser(this);
	}

	// -------------------------------------------------------------------------------------

	MsgRemoteParticipant::~MsgRemoteParticipant()
	{
		if (!m_remoteParticipant.IsEmpty()) m_remoteParticipant.Clear()->DestroyParticipant();

		if (!m_user.IsEmpty())
		{
			if (m_media != 0) m_media->UnlinkUser(m_user);

			m_user.Clear()->OnParticipantTerminated(this);
		}
	}

	// -------------------------------------------------------------------------------------
	// IRemoteParticipant

	std::string MsgRemoteParticipant::NameParticipant() const
	{
		return (m_remoteParticipant.IsEmpty()) ? "-" : m_remoteParticipant->NameParticipant();
	}

	// -------------------------------------------------------------------------------------

	void MsgRemoteParticipant::LinkUser(Utils::SafeRef<IRemoteParticipantUser> user)
	{
		ESS_ASSERT(m_user.IsEmpty() || m_user == user);

		m_user = user;
	}

	// -------------------------------------------------------------------------------------

	void MsgRemoteParticipant::MediaObjectIsReady()
	{
		ESS_ASSERT (!m_user.IsEmpty() && m_user->Media() != 0);

		PutMsg(this, &T::MediaObjectIsReadyImpl);
	}

	// -------------------------------------------------------------------------------------

	boost::shared_ptr<iReg::CallRecordWrapper>
		MsgRemoteParticipant::CallRecord()
	{
		boost::shared_ptr<iReg::CallRecordWrapper> p;
		if (!m_remoteParticipant.IsEmpty()) p = m_remoteParticipant->CallRecord();
		return p;
	}

	// -------------------------------------------------------------------------------------

	void MsgRemoteParticipant::RelatedParticipants(RelatedParticipantList &list)
	{
		if (m_legs == 0)	return;

		for (ForkingLegs::iterator i = m_legs->begin();
			i != m_legs->end(); ++i)
		{
			if (!i->Empty()) list.push_back(&i->Value());
		}
	}

	// -------------------------------------------------------------------------------------

	boost::shared_ptr<ConversationProfile> MsgRemoteParticipant::GetConversationProfile() const
	{
		return (m_remoteParticipant.IsEmpty()) ?
			boost::shared_ptr<ConversationProfile>() :
			m_remoteParticipant->GetConversationProfile();
	}

	// -------------------------------------------------------------------------------------

	void MsgRemoteParticipant::AnswerParticipant()
	{
		PutMsg(this, &T::AnswerParticipantImpl);
	}

	// -------------------------------------------------------------------------------------

	void MsgRemoteParticipant::UpdateMedia()
	{
		PutMsg(this, &T::UpdateMediaImpl);
	}

	// -------------------------------------------------------------------------------------

	void MsgRemoteParticipant::AlertParticipant( bool earlyFlag )
	{
		ESS_ASSERT (!earlyFlag || m_media->IsReady());

		PutMsg(this, &T::AlertParticipantImpl, earlyFlag);
	}

	// -------------------------------------------------------------------------------------

	void MsgRemoteParticipant::DestroyParticipant( unsigned int rejectCode,
		const std::string &description)
	{
		if (!m_user.IsEmpty())
		{
			m_media->UnlinkUser(m_user);

			m_user.Clear();
		}

		PutFunctor(boost::bind(&T::DestroyParticipantImpl, this, rejectCode,  description));
	}

	// -------------------------------------------------------------------------------------

	void MsgRemoteParticipant::RedirectParticipant( const resip::NameAddr& destination )
	{
		PutMsg(this, &T::RedirectParticipantImpl, destination );
	}

	// -------------------------------------------------------------------------------------

	void MsgRemoteParticipant::RedirectToParticipant(IRemoteParticipant &dst)
	{
		MsgRemoteParticipant *dstCall = dynamic_cast<MsgRemoteParticipant *>(&dst);
		
		ESS_ASSERT (dstCall != 0);

		m_conversationManager.PostMsg(boost::bind(&T::RedirectToParticipantImpl,
			SelfRef(),
			dstCall->SelfRef()));
	}

	// -------------------------------------------------------------------------------------
	// impl msg IRemoteParticipant

	void MsgRemoteParticipant::MediaObjectIsReadyImpl()
	{
		if (m_user.IsEmpty() || m_user->Media() == 0) return;
			
		m_media->LinkUser(m_user);

		if (!m_remoteParticipant.IsEmpty()) m_remoteParticipant->MediaObjectIsReady();
	}

	// -------------------------------------------------------------------------------------

	void MsgRemoteParticipant::AlertParticipantImpl( bool earlyFlag )
	{
		if (!m_remoteParticipant.IsEmpty()) m_remoteParticipant->AlertParticipant(earlyFlag);
	}

	// -------------------------------------------------------------------------------------

	void MsgRemoteParticipant::AnswerParticipantImpl()
	{
		ESS_ASSERT (m_media->IsReady());

		TalkRegistration(true);
		if (!m_remoteParticipant.IsEmpty()) m_remoteParticipant->AnswerParticipant();
	}

	// -------------------------------------------------------------------------------------

	void MsgRemoteParticipant::UpdateMediaImpl()
	{
		if (!m_remoteParticipant.IsEmpty()) m_remoteParticipant->UpdateMedia();
	}

	// -------------------------------------------------------------------------------------

	void MsgRemoteParticipant::DestroyParticipantImpl( unsigned int rejectCode,
		const std::string &description)
	{
		if (!m_remoteParticipant.IsEmpty()) m_remoteParticipant.Clear()->DestroyParticipant(rejectCode, description);

		Close();
	}

	// -------------------------------------------------------------------------------------

	void MsgRemoteParticipant::RedirectParticipantImpl( const resip::NameAddr& destination )
	{
		if (!m_remoteParticipant.IsEmpty()) m_remoteParticipant->RedirectParticipant( destination );
	}

	// -------------------------------------------------------------------------------------
	
	void MsgRemoteParticipant::RedirectToParticipantImpl(
		Utils::WeakRef<MsgRemoteParticipant&> call1,
		Utils::WeakRef<MsgRemoteParticipant&> call2)
	{
		bool isCall1Valid = !call1.Empty() && !call1.Value().RemoteParticipantRef().IsEmpty();
		bool isCall2Valid = !call2.Empty() && !call2.Value().RemoteParticipantRef().IsEmpty();			

		if (isCall1Valid != isCall2Valid)
		{
			if (!isCall1Valid) call2.Value().OnParticipantRedirectFailure(
				call2.Value().RemoteParticipantRef().Clear(), 480);

			if (!isCall2Valid) call1.Value().OnParticipantRedirectFailure(
				call1.Value().RemoteParticipantRef().Clear(), 480);

			return;
		}

		if (!isCall1Valid && !isCall1Valid) return;


		call1.Value().RemoteParticipantRef()->RedirectToParticipant(*call2.Value().RemoteParticipantRef().Clear());
	}

	// -------------------------------------------------------------------------------------
	// IRemoteParticipantUser

	std::string MsgRemoteParticipant::CallUserName() const
	{
		return (m_user.IsEmpty()) ? "" : m_user->CallUserName();
	}

	// -------------------------------------------------------------------------------------

	ISipMedia* MsgRemoteParticipant::Media()			{	return m_media.get(); }

	// -------------------------------------------------------------------------------------

	const ISipMedia* MsgRemoteParticipant::Media() const{	return m_media.get(); }

	// -------------------------------------------------------------------------------------

	void MsgRemoteParticipant::OnParticipantTerminated(const IRemoteParticipant *src,
		unsigned int statusCode)
	{
		ESS_ASSERT(m_remoteParticipant.IsEqualIntf(src));

		m_remoteParticipant.Clear();

		if (!m_user.IsEmpty())
		{
			m_media->UnlinkUser(m_user);

			m_user.Clear()->OnParticipantTerminated(this, statusCode);
		}

		Close();
	}

	boost::shared_ptr<iReg::CallRecordWrapper>
		MsgRemoteParticipant::CallRecordByUser()
	{
		boost::shared_ptr<iReg::CallRecordWrapper> p;
		
		if (!m_user.IsEmpty()) p = m_user->CallRecordByUser();
		
		return p;
	}

	// -------------------------------------------------------------------------------------

	void MsgRemoteParticipant::OnRefer(const IRemoteParticipant *src, const resip::SipMessage &msg)
	{
		ESS_ASSERT(m_remoteParticipant.IsEqualIntf(src));

		m_user->OnRefer(this, msg);
	}

	// -------------------------------------------------------------------------------------

	void MsgRemoteParticipant::OnParticipantAlerting(const IRemoteParticipant *src,
		const resip::SipMessage& msg)
	{
		ESS_ASSERT(m_remoteParticipant.IsEqualIntf(src));

		if (!m_user.IsEmpty()) m_user->OnParticipantAlerting(this, msg);
	}

	// -------------------------------------------------------------------------------------

	void MsgRemoteParticipant::OnParticipantConnected(const IRemoteParticipant *src,
		const resip::SipMessage &msg)
	{
		ESS_ASSERT(m_remoteParticipant.IsEqualIntf(src));

		TalkRegistration(true);
		if (!m_user.IsEmpty()) m_user->OnParticipantConnected(this, msg);
	}

	// -------------------------------------------------------------------------------------

	void MsgRemoteParticipant::OnParticipantRedirectSuccess(const IRemoteParticipant *src)
	{
		ESS_ASSERT(m_remoteParticipant.IsEqualIntf(src));

		if (!m_user.IsEmpty()) m_user->OnParticipantRedirectSuccess(this);
	}

	// -------------------------------------------------------------------------------------

	void MsgRemoteParticipant::OnParticipantRedirectFailure(const IRemoteParticipant *src,
		unsigned int statusCode)
	{
		ESS_ASSERT(m_remoteParticipant.IsEqualIntf(src));

		if (!m_user.IsEmpty()) m_user->OnParticipantRedirectFailure(this, statusCode);
	}

	// -------------------------------------------------------------------------------------

	void MsgRemoteParticipant::OnForkingOccured(const IRemoteParticipant *original,
		Utils::SafeRef<IRemoteParticipant> newParticipant,
		const resip::SipMessage &msg)
	{
		ESS_ASSERT(m_remoteParticipant.IsEqualIntf(original));

		MsgRemoteParticipant *newMsgCall = new MsgRemoteParticipant(getMsgThread(),
			m_conversationManager,
			m_registrator,
			newParticipant);

		if (m_legs == 0) m_legs.reset(new ForkingLegs());

		newMsgCall->SetupForkingLegsList(m_legs);

		m_user->OnForkingOccured(this, newMsgCall, msg);
	}

	// -------------------------------------------------------------------------------------

	void MsgRemoteParticipant::OnParticipantReplacedBy(const IRemoteParticipant *original,
		Utils::SafeRef<IRemoteParticipant> newCall)
	{
		ESS_ASSERT(m_remoteParticipant.IsEqualIntf(original));

		m_remoteParticipant = newCall;

		m_remoteParticipant->LinkUser(this);
	}

	// -------------------------------------------------------------------------------------

	void MsgRemoteParticipant::CreateParticipant(ConversationManager &conversationManager,
		boost::shared_ptr<IRemoteParticipantCreator::Parameters> parameters)
	{
		ESS_ASSERT(m_media == 0);

		m_media.reset(new MediaWrapper(m_user,
			parameters->ConversationProfile->GetNatIpConverter()));

		RemoteParticipantDialogSet* participantDialogSet =
			new RemoteParticipantDialogSet(conversationManager.GetDialogUsageManager(),
			conversationManager,
			conversationManager.LogCreator(),
			parameters->ForkSelectMode);

		m_remoteParticipant = participantDialogSet->CreateOutgoingParticipant(
			this,
			parameters);
	}

	// -------------------------------------------------------------------------------------

	void MsgRemoteParticipant::Close()
	{
		if (m_isDestroyed) return;
		
		ESS_ASSERT(m_remoteParticipant.IsEmpty() && m_user.IsEmpty());

		m_isDestroyed = true;

		m_registrator(this, false);
	}

	// -------------------------------------------------------------------------------------

	Utils::WeakRef<MsgRemoteParticipant&> MsgRemoteParticipant::SelfRef()
	{
		return m_selfRefHost.Create<MsgRemoteParticipant&>(*this);
	}

	// -------------------------------------------------------------------------------------

	void MsgRemoteParticipant::SetupForkingLegsList(boost::shared_ptr<ForkingLegs> legs)
	{
		ESS_ASSERT (m_legs == 0);
		ESS_ASSERT (legs != 0);

		m_legs = legs;
				
		m_legs->push_back(SelfRef());
	}

	// -------------------------------------------------------------------------------------

	void MsgRemoteParticipant::TalkRegistration(bool turnOn)
	{
		if (!m_remoteParticipant.IsEmpty())
		{
			m_remoteParticipant->CallRecord()->Record().Talk();
		}
	}
}
