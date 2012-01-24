#pragma once

#include "IRemoteParticipant.h"
#include "SipRegistrationState.h"
#include "Utils/IBasicInterface.h"
#include "Utils/ErrorsSubsystem.h"

namespace resip	
{ 
	class SipMessage; 
	class SdpContents;
}

namespace iSip
{
	class ConversationProfile;

	class ISipAccessPointEvents;

	class ISipAccessPoint : public IRemoteParticipantCreator
	{
	public:
		
		virtual void SetCallReceiver(const std::string &callsReceiverName,
			Utils::WeakRef<ISipAccessPointEvents&> callReceiver) = 0;
	};

	// -------------------------------------------------------------------------------------

	class ISipAccessPointEvents : public Utils::IBasicInterface
	{
	public:

		// !slg! Note: We should eventually be passing back a generic ParticipantInfo object
		//             and not the entire SipMessage for these callbacks

		/**
		 Notifies an application about a new remote participant that is attempting
		 to contact it.

		 @param partHandle Handle of the new incoming participant
		 @param msg Includes information about the caller such as name and address
		 @param autoAnswer Set to true if auto answer has been requested
		*/
		virtual void OnIncomingParticipant(const ISipAccessPoint *src,
			Utils::SafeRef<IRemoteParticipant> newParticipant,
			const resip::SipMessage &msg, 
			bool autoAnswer, 
			const ConversationProfile &conversationProfile) = 0;

		/**
		 Notifies an application about a new remote participant that is trying 
		 to be contacted.  This event is required to notify the application if a 
		 call request has been initiated by a signaling mechanism other than
		 the application, such as an out-of-dialog REFER request.

		 @param partHandle Handle of the new remote participant
		 @param msg Includes information about the destination requested
					to be attempted
		*/
		virtual void OnRequestOutgoingParticipant(const ISipAccessPoint *src,
			Utils::SafeRef<IRemoteParticipant> newParticipant, 
			const resip::SipMessage &msg, 
			const ConversationProfile &conversationProfile) = 0;

	};

	class ISipAccessPointEventsExt : public ISipAccessPointEvents
	{
	public:
		virtual void RegistrationState(SipRegistrationState::Value state, 
			const std::string &description = "") = 0;
	};

}

