#pragma once

#include "stdafx.h"

#include "iLog/LogWrapper.h"

#include <resip/dum/SubscriptionHandler.hxx>
#include <resip/dum/SubscriptionHandler.hxx>


namespace resip	
{	
	class DialogUsageManager;
	class SipMessage; 
}

namespace iSip
{

	class ConversationProfile;

    class ServerSubscriptionHandler : boost::noncopyable,
        public resip::ServerSubscriptionHandler
    {
	public:

		typedef boost::function<void (resip::ServerSubscriptionHandle, const resip::SipMessage& msg) > 
			NewParticipantCreator;

		ServerSubscriptionHandler(
			iLogW::ILogSessionCreator &logCreator,
			resip::DialogUsageManager &dum,
			NewParticipantCreator newParticipantCreator);

		~ServerSubscriptionHandler();

	// implement of ServerSubscriptionHandler
    private:
		void onNewSubscription(
			resip::ServerSubscriptionHandle, const resip::SipMessage& sub);

		void onNewSubscriptionFromRefer(
			resip::ServerSubscriptionHandle, const resip::SipMessage& sub);

		void onRefresh(resip::ServerSubscriptionHandle, const resip::SipMessage& sub);

		void onTerminated(resip::ServerSubscriptionHandle);

		void onReadyToSend(resip::ServerSubscriptionHandle, resip::SipMessage&);

		void onNotifyRejected(resip::ServerSubscriptionHandle, const resip::SipMessage& msg);

		void onError(resip::ServerSubscriptionHandle, const resip::SipMessage& msg);

		void onExpiredByClient(
			resip::ServerSubscriptionHandle, const resip::SipMessage& sub, resip::SipMessage& notify);

		void onExpired(resip::ServerSubscriptionHandle, resip::SipMessage& notify);

		bool hasDefaultExpires() const;

		boost::uint32_t  getDefaultExpires() const;
	
	private:

		void InfoLog(const std::string &event, const resip::SipMessage *msg = 0)
		{
			LogMsg(m_infoTag, event, msg);
		}

		void WarningLog(const std::string &event, const resip::SipMessage *msg = 0)
		{
			LogMsg(m_warningTag, event, msg);
		}

		void LogMsg(iLogW::LogRecordTag  &tag, const std::string &event, const resip::SipMessage *msg);

	private:

		boost::scoped_ptr<iLogW::LogSession>  m_log;
		iLogW::LogRecordTag m_infoTag;
		iLogW::LogRecordTag m_warningTag;

		resip::DialogUsageManager &m_dum;
		NewParticipantCreator m_newParticipantCreator;
    };
}
