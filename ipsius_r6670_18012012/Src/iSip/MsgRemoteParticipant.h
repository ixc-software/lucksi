#pragma once

#include "IRemoteParticipant.h"

#include "iCore/MsgThread.h"
#include "iCore/MsgObject.h"
#include "Utils/WeakRef.h"

namespace iReg {	class CallRecordWrapper;	 }

namespace iSip
{

	class ConversationManager;

	class MsgRemoteParticipant : boost::noncopyable,
		public iCore::MsgObject,
		public IRemoteParticipant,
		public IRemoteParticipantUser
	{
		typedef MsgRemoteParticipant T;

	public:

		MsgRemoteParticipant(iCore::MsgThread &thread,
			ConversationManager &conversationManager,
			boost::function<void (IRemoteParticipant*, bool)> registrator,
			Utils::SafeRef<IRemoteParticipantUser> user,
			boost::shared_ptr<IRemoteParticipantCreator::Parameters> parameters);

		MsgRemoteParticipant(iCore::MsgThread &thread,
			ConversationManager &conversationManager,
			boost::function<void (IRemoteParticipant*, bool)> registrator,
			Utils::SafeRef<IRemoteParticipant> remoteParticipant);

		~MsgRemoteParticipant();

	// IRemoteParticipant
	private:
		std::string NameParticipant() const;
		void LinkUser(Utils::SafeRef<IRemoteParticipantUser> user);
		void MediaObjectIsReady();
		boost::shared_ptr<iReg::CallRecordWrapper> CallRecord();
		void RelatedParticipants(RelatedParticipantList &list);
		boost::shared_ptr<ConversationProfile> GetConversationProfile() const;
		void AlertParticipant( bool earlyFlag );
		void AnswerParticipant();
		void UpdateMedia();
		void DestroyParticipant( unsigned int rejectCode, // 480 - Temporarily Unavailable
			const std::string &description);
		void RedirectParticipant( const resip::NameAddr &destination);
		void RedirectToParticipant(IRemoteParticipant &dst);

	// impl msg IRemoteParticipant 
	private:
		void MediaObjectIsReadyImpl();
		void AlertParticipantImpl( bool earlyFlag );
		void AnswerParticipantImpl();
		void UpdateMediaImpl();
		void DestroyParticipantImpl( unsigned int rejectCode,
			const std::string &description);
		void RedirectParticipantImpl( const resip::NameAddr& destination );
		static void RedirectToParticipantImpl(Utils::WeakRef<MsgRemoteParticipant&> call1,
			Utils::WeakRef<MsgRemoteParticipant&> call2);

	// IRemoteParticipantUserExt
	private:

		std::string CallUserName() const;

		ISipMedia* Media();

		const ISipMedia* Media() const;

		boost::shared_ptr<iReg::CallRecordWrapper> CallRecordByUser(); 

		void OnParticipantTerminated(const IRemoteParticipant *src, 
			unsigned int statusCode);

		void OnRefer(const IRemoteParticipant *src, const resip::SipMessage &msg);

		void OnParticipantAlerting(const IRemoteParticipant *src, 
			const resip::SipMessage& msg);

		void OnParticipantConnected(const IRemoteParticipant *src,
			const resip::SipMessage &msg);

		void OnParticipantRedirectSuccess(const IRemoteParticipant *src);

		void OnParticipantRedirectFailure(const IRemoteParticipant *src, 
			unsigned int statusCode);

		void OnForkingOccured(const IRemoteParticipant *original, 
			Utils::SafeRef<IRemoteParticipant> newParticipant,
			const resip::SipMessage &msg);

		void OnParticipantReplacedBy(const IRemoteParticipant *original, 
			Utils::SafeRef<IRemoteParticipant> newCall);

	private:

		void CreateParticipant(ConversationManager &conversationManager,
			boost::shared_ptr<IRemoteParticipantCreator::Parameters> parameters);

		void Close();

		Utils::WeakRef<MsgRemoteParticipant&> SelfRef();

		typedef std::vector<Utils::WeakRef<MsgRemoteParticipant&> > ForkingLegs;
		void SetupForkingLegsList(boost::shared_ptr<ForkingLegs> legs);

		Utils::SafeRef<IRemoteParticipant> RemoteParticipantRef() 
		{
			return m_remoteParticipant;
		}
		
		void TalkRegistration(bool turnOn);

	private:

		Utils::WeakRefHost m_selfRefHost;
		ConversationManager &m_conversationManager;
		bool m_isDestroyed;
		boost::function<void (IRemoteParticipant*, bool)> m_registrator;

		Utils::SafeRef<IRemoteParticipantUser> m_user;
		Utils::SafeRef<IRemoteParticipant> m_remoteParticipant;

		boost::shared_ptr<ForkingLegs> m_legs;
		
		class MediaWrapper;
		boost::scoped_ptr<MediaWrapper> m_media;		
	};

}
