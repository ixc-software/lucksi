#ifndef _CALL_TEST_RTP_WITH_SIP_H_
#define _CALL_TEST_RTP_WITH_SIP_H_

#include "stdafx.h"
#include "iCore/MsgThread.h"
#include "iCore/MsgObject.h"
#include "iCore/MsgTimer.h"
#include "Utils/IBasicInterface.h"
#include "Utils/IVirtualDestroy.h"
#include "Sip/ISipCall.h"

namespace Sip
{
	class ISipCall;
	class ISipToRtp;
};

namespace TestRtpWithSip
{
    class RtpClient;

    class CallTestRtpWithSip : 
        boost::noncopyable, 
        public iCore::MsgObject,
        public Utils::IVirtualDestroy,
        public Sip::ISipCallEvents
    {
		typedef CallTestRtpWithSip T;
    public:    
        CallTestRtpWithSip(iCore::MsgThread &thread,
            Utils::IVirtualDestroyOwner &owner,
            Utils::SafeRef<Sip::ISipCall> call, 
			Utils::SafeRef<Sip::ISipToRtp> rtp,
			int timeout = 0); 
    // ISipCallEvents
    private:
		void Alerted(const Sip::ISipCall *);
		void Connected(const Sip::ISipCall *);
		void Terminated(const Sip::ISipCall *, int statusCode = 0);
    private:
        void EndTalk(iCore::MsgTimer *pT);
    private:
        Utils::IVirtualDestroyOwner &m_owner;
		Utils::SafeRef<Sip::ISipCall> m_call;
        iCore::MsgTimer m_timer;
    };
};

#endif
