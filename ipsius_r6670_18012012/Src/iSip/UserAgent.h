#pragma once

#include "stdafx.h"
#include "iCore/MsgThread.h"
#include "iCore/MsgObject.h"
#include "iCore/MsgTimer.h"

#include "IRemoteParticipant.h"
#include "IUserAgentRegistrator.h"
#include "IUserAgentSubscription.h"

#include "ConversationProfile.h"
#include "UserAgentMasterProfile.h"
#include "SipUtils.h"

#include "iLog/LogWrapper.h"

#include "Utils/StatisticElement.h" 
#include "Utils/ThreadContext.h"

#include <resip/stack/InterruptableStackThread.hxx>
#include <resip/stack/SelectInterruptor.hxx>

#include <resip/dum/DumShutdownHandler.hxx>
#include <resip/dum/DialogUsageManager.hxx>


namespace resip { class SipMessage; }

namespace iSip
{
	class ConversationManager;
	class SipTrustedNodeList;
	class UserAgentClientSubscription;
	class UserAgentRegistrator;

	class ConversationProfile;
	class IRemoteParticipant;

	/**
	  This class is one of two main classes of concern to an application
	  using the UserAgent library.  This class should be subclassed by 
	  the application and the UserAgent handlers should be implemented
	  by it.

	  This class is responsible for handling tasks that are not directly
	  related to managing the Conversations themselves.  All conversation
	  management is done via the ConversationManager class.  
	  
	  This class handles tasks such as:
	  - Startup, Process Loop, and shutdown handling
	  - Handling user agent settings (UserAgentMasterProfile)
	  - Management of Conversation Profiles and Registrations
	  - Management of Subscriptions
	*/

	class UserAgent : boost::noncopyable,
		public iCore::MsgObject, 
		public resip::DumShutdownHandler
	{
		
		typedef UserAgent T;

	public:

		/**
		 Constructor

		 @param conversationManager Application subclassed Conversation 
									Manager
		 @param masterProfile       Object containing useragent settings
		 @param socketFunc          A pointer to a function that will be called after a socket 
									in the DNS or SIP transport layers of the stack has been 
									created.  This callback can be used to control low level 
									socket options, such as Quality-of-Service/DSCP.
									Note:  For SIP TCP sockets there is one call for the listen
									socket, and one (or two) calls for each connection created 
									afterwards.  For each inbound TCP connection the first 
									callback is called immediately before the socket is connected, 
									and if configured it is called again after the connect call
									has completed and before the first data is sent out.  
									On some OS's you cannot set QOS until the socket is successfully 
									connected.  To enable this behavior call:
									Connection::setEnablePostConnectSocketFuncCall();
		*/
		UserAgent(iCore::MsgThread &thread,
			iLogW::ILogSessionCreator &logCreator,
			const SipTrustedNodeList &trustedNodeList,
			resip::SharedPtr<UserAgentMasterProfile>  masterProfile,
			boost::shared_ptr<ConversationProfile>    defaultConversationProfile,
			resip::AfterSocketCreationFuncPtr socketFunc=0);

		~UserAgent();

		// activate UserAgent
		bool Startup(std::string &error);

		resip::DialogUsageManager& getDialogUsageManager();

		IRemoteParticipantCreator& CallCreator();

		IUserAgentRegistrator& Registrator();

		const IUserAgentRegistrator& Registrator() const;

		const UserAgentMasterProfile::TransportList& getTransports() const;

		std::string DumProceessingStat() const 
		{
			std::string val;

			m_dumProcessDuration.ToString(val, Utils::StatElementForInt::LevFull);

			return val;
		}

		/**
		 Used to initiate a snapshot of the existing DNS entries in the
		 cache to the logging subsystem.
		*/
		void logDnsCache();

		/**
		 Used to clear the existing DNS entries in the cache.
		*/
		void clearDnsCache();

		/**
		 Requests that the user agent create and manage an event subscription.  
		 When an subscribed event is received the onSubscriptionNotify callback 
		 is used.  If the subscription is terminated by the server, the application 
		 or due to network failure the onSubscriptionTerminated callback is used.

		 @param eventType Event type we are subscribing to
		 @param target    A URI representing the location of the subscription server
		 @param subscriptionTime Requested time that the subscription should stay active, 
						  before sending a refresh request is required.
		 @param mimeType  The mime type of event body expected
		*/

		IUserAgentSubscription* CreateSubscription(
			iLogW::ILogSessionCreator &logCreator,
			IUserAgentSubscriptionEvent &user,
			resip::SharedPtr<resip::UserProfile> profile,
			const resip::Data& eventType, const resip::NameAddr& target, unsigned int subscriptionTime, const resip::Mime& mimeType);

	// Shutdown Handler 
	private:

		void onDumCanBeDeleted();

	private:

		/**
			Used to initiate a shutdown of the useragent.  This function blocks 
			until the shutdown is complete.  

			@note There should not be an active process request when this 
			is called.
		*/
		void Shutdown();

	private:

		class ShutdownCmd;

		resip::SharedPtr<UserAgentMasterProfile> getUserAgentMasterProfile();

		bool addTransports(std::string &error);
		void post(resip::ApplicationMessage& message, unsigned int ms=0);
		void shutdownImpl(); 

	private:
		
		void OnPulse(iCore::MsgTimer *pT);

		/**
		 This should be called in a loop to give process cycles to the UserAgent.

		 @param timeoutMs Will return after timeoutMs if nothing to do.
						  Application can do some work, but should call
						  process again ASAP.
		*/
		void process(); // call this in a loop
	
	private:

		iCore::MsgTimer m_timer;
		Utils::StatElementForInt m_dumProcessDuration;

		boost::scoped_ptr<iLogW::LogSession>  m_log;
		iLogW::LogRecordTag m_infoTag;
		iLogW::LogRecordTag m_warningTag;

		Utils::ThreadContext m_threadContext;

		resip::SharedPtr<UserAgentMasterProfile> m_profile;
	
		UserAgentMasterProfile::TransportList m_transportList;

		resip::Security *m_security;
		resip::SelectInterruptor mSelectInterruptor;
		resip::SipStack m_stack;
		resip::InterruptableStackThread m_stackThread;
		resip::DialogUsageManager m_dum;
		bool  m_dumShutdown;

		boost::scoped_ptr<UserAgentRegistrator> m_registrator;
		boost::scoped_ptr<UserAgentClientSubscription> m_subscriber;
		boost::scoped_ptr<ConversationManager> m_conversationManager;
		bool m_running;
	};
 
}

