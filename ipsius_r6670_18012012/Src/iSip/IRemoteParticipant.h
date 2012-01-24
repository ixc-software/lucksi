#pragma once

#include "ParticipantForkSelectMode.h"
#include "ISipMedia.h"
#include "Utils/WeakRef.h"
#include "Utils/SafeRef.h"
#include "Utils/IBasicInterface.h"
#include "iReg/CallRecord.h"

#include <resip/stack/NameAddr.hxx>

namespace resip	
{	
	class SipMessage;
	class SdpContents;
	class NameAddr; 
}

namespace iReg	{	class CallRecordWrapper; }

namespace iSip
{

	class ConversationProfile;
	class IRemoteParticipantUser;

	class IRemoteParticipant : Utils::IBasicInterface
	{
	public:

		virtual std::string NameParticipant() const = 0;

		virtual void LinkUser(Utils::SafeRef<IRemoteParticipantUser> user) = 0;

		typedef std::vector<IRemoteParticipant*> RelatedParticipantList;

		virtual void RelatedParticipants(RelatedParticipantList &list) = 0;

		virtual boost::shared_ptr<ConversationProfile> GetConversationProfile() const = 0;

		virtual void MediaObjectIsReady() = 0;

		virtual boost::shared_ptr<iReg::CallRecordWrapper> CallRecord() = 0;

	   /**
		 Signal to the participant that it should provide ringback.  
		 For SIP this causes a 180 to be sent.  
		 The early flag indicates if we are sending early media or not.  
		 (ie.  For SIP - SDP in 180).  

		 @param earlyFlag Set to true to send early media
	   */
	   virtual void AlertParticipant(bool earlyFlag = false) = 0;

	   /**
		 Signal to the participant that the call is answered.  
		 For SIP this causes a 200 to be sent.   
	   */
	   virtual void AnswerParticipant() = 0;

	   /**
		 Reinvite to be sent..   
	   */
	   virtual void UpdateMedia() = 0;

	   /**
		 Rejects an incoming remote participant with the specified code.  
		 Can also be used to reject an outbound participant request (due to REFER).   
		 Or for destroy participant in other states

		 @param rejectCode Code sent to remote participant for rejection
	   */
	   virtual void DestroyParticipant(unsigned int rejectCode = 480,
		   const std::string &description = "") = 0; // 480 - Temporarily Unavailable

	   /**
		 Redirects the participant to another endpoint.  For SIP this would 
		 either be a 302 response or would initiate blind transfer (REFER) 
		 request, depending on the state.   

		 @param partHandle Handle of the participant to redirect
		 @param destination Uri of destination to redirect to
	   */
	   virtual void RedirectParticipant(const resip::NameAddr &destination) = 0;


	   /**
		 This is used for attended transfer scenarios where both participants 
		 are no longer managed by the conversation manager  – for SIP this will 
		 send a REFER with embedded Replaces header.  Note:  Replace option cannot 
		 be used with early dialogs in SIP.  
	   */
	   virtual void RedirectToParticipant(IRemoteParticipant &dst) = 0;

	};

	// -------------------------------------------------------------------------------------

	class IRemoteParticipantUser : Utils::IBasicInterface
	{
	public:

		virtual std::string CallUserName() const = 0;

		virtual ISipMedia* Media() = 0;

		virtual const ISipMedia* Media() const = 0;

		virtual boost::shared_ptr<iReg::CallRecordWrapper> CallRecordByUser() = 0; 

		/**
		 Notifies an application about a disconnect by a remote participant.  
		 For SIP this could be a BYE or a CANCEL request.

		 @param partHandle Handle of the participant that terminated
		 @param statusCode The status Code for the termination.
		*/
		virtual void OnParticipantTerminated(const IRemoteParticipant *src, 
			unsigned int statusCode = 0) = 0;

		// called when an REFER message is received.
		virtual void OnRefer(const IRemoteParticipant *src, const resip::SipMessage &msg) = 0;

		/**
		 Notifies an application that a remote participant call attempt is 
		 alerting the remote party.  

		 @param partHandle Handle of the participant that is alerting
		 @param msg SIP message that caused the alerting
		*/
		virtual void OnParticipantAlerting(const IRemoteParticipant *src, 
			const resip::SipMessage& msg) = 0;

		/**
		 Notifies an application that a remote participant call attempt is 
		 now connected.

		 @param partHandle Handle of the participant that is connected
		 @param msg SIP message that caused the connection
		*/
		virtual void OnParticipantConnected(const IRemoteParticipant *src,
			const resip::SipMessage &msg) = 0;

		/**
		 Notifies an application that a redirect request has succeeded.  
		 Indicates blind transfer or attended transfer status. 

		 @param partHandle Handle of the participant that was redirected
		*/
		virtual void OnParticipantRedirectSuccess(const IRemoteParticipant *src) = 0;

		/**
		 Notifies an application that a redirect request has failed.  
		 Indicates blind transfer or attended transfer status. 

		 @param partHandle Handle of the participant that was redirected
		*/
		virtual void OnParticipantRedirectFailure(const IRemoteParticipant *src, 
			unsigned int statusCode) = 0;

		/**
		 Notifies an applications that a outbound remote participant request has 
		 forked.  A new Participant are created.  
		*/
		virtual void OnForkingOccured(const IRemoteParticipant *original, 
			Utils::SafeRef<IRemoteParticipant> newParticipant,
			const resip::SipMessage &msg) = 0;

		/**
		 Notifies an applications that a outbound remote participant request has 
		 replaced. 
		*/
		virtual void OnParticipantReplacedBy(const IRemoteParticipant *original, 
			Utils::SafeRef<IRemoteParticipant> newCall)
		{
			ESS_HALT ("forbid to implement");
		}
	};

	// -------------------------------------------------------------------------------------

	class IRemoteParticipantCreator : Utils::IBasicInterface
	{
	public:

		struct Parameters 
		{
			Parameters()
			{
				ForkSelectMode = ParticipantForkSelectMode::Automatic;
				
				UseSourceNameInContact = false;
				UseSourceUserNameInFrom = false;
				UseSourceUserNameAsDisplayName = false;
			}

			bool IsValid() const
			{
				return !UseSourceUserNameInFrom || UseSourceNameInContact;
			}

			// if you use SetDestinationUserName then destination host 
			// will be taken from ConversationProfile
			void SetDestinationUserName(const resip::Data &destinationUserName, bool isLocalCall = false)
			{
				Destination.uri().user() = destinationUserName;
                
                if (isLocalCall) Destination.uri().host() = "127.0.0.1";
			}

			// explicit setup of destination address
			void SetDestination(const resip::NameAddr &destination)
			{
				Destination = destination; 
			}
			
			resip::NameAddr GetDestination() const
			{
				return Destination;
			}

			resip::Data     SourceUserName;  // use as Contact if isn't empty

			bool UseSourceNameInContact;
			bool UseSourceUserNameInFrom;
			bool UseSourceUserNameAsDisplayName;

            boost::shared_ptr<iSip::ConversationProfile> ConversationProfile;
			ParticipantForkSelectMode::Value ForkSelectMode;

		private:
			resip::NameAddr Destination; // use if DestinationUser is empty
		};

		virtual Utils::SafeRef<IRemoteParticipant> CreateOutgoingParticipant(
			Utils::SafeRef<IRemoteParticipantUser> user,
			boost::shared_ptr<Parameters> parameters) = 0;
	};

}

