#include "stdafx.h"
#include "BfRtpTests.h"

#include "BfRtp/RtpParams.h"
//#include "BfRtp/RtpCoreInfra.h"
#include "BfRtp/RtpCoreInfraForBlackfin.h"
#include "BfRtp/RtpCoreSession.h"
#include "BfRtp/IRtpCoreToUser.h"
#include "Utils/ErrorsSubsystem.h"

//#include "iLog/LogSessionProfileSetup.h"
//#include "iLog/LogSessionSettings.h"

#include "iCore/MsgThread.h"
#include "iCore/MsgObject.h"
#include "iCore/MsgTimer.h"
#include "iCore/ThreadRunner.h"

class TestProfile
{};

namespace BfRtpTests
{
    using Platform::dword;
    using Utils::BidirBuffer;
    
    class RtpClient
        : public iRtp::IRtpCoreToUser,
        public iCore::MsgObject
    {
        enum TestCfg
        {
            CActionInterval = 10,
        };
        
        iCore::MsgTimer m_timer;
                
        const iLogW::LogSessionSettings m_logSettings;
        iLogW::LogSessionProfileSetup m_logProf;
        iLogW::LogStore m_store;
        iRtp::RtpInfraParams m_infraProf;
        iRtp::RtpCoreInfraForBlackfin m_infra; 
        iRtp::RtpParams m_sessionProf;
        iRtp::RtpCoreSession m_rtp;
        
        
     // ITimerEvents impl   
     private:
        
        void OnTimer(iCore::MsgTimer*)
        {
            m_infra.Process();
            //...action
        }
        
     // IRtpCoreToUser impl
     private:
     
        void RxData(BidirBuffer* pBuff, iRtp::RtpHeaderForUser header)
        {}
        void RxEvent(iRtp::RtpEvent ev, dword timestamp)
        {
            ESS_ASSERT(0);
        }
        void RxNewPayload(iRtp::RtpPayload payload)
        {
            ESS_ASSERT(0);
        }
        void RtpErrorInd(iRtp::RtpBaseError er)
        {
            ESS_ASSERT(0);
        }
        
     public:
     
        RtpClient(iCore::IThreadRunner &runner, const TestProfile& prof)
            : iCore::MsgObject(runner.getThread()),
            m_timer(this, &RtpClient::OnTimer),
            m_logProf(m_logSettings),
            m_infraProf( m_logProf.getLogSessionProfile() ),
            m_infra(m_infraProf, m_store.getSessionInterface()), 
            m_rtp(m_sessionProf, m_infra, *this)
        {
            m_timer.Start(CActionInterval, true);
        }
             
        
             
    };
    
    void LoopBackTest()
    {
        const TestProfile prof;
        
        iCore::ThreadRunner test;
        
        if(!test.Run<RtpClient>(prof))
           TUT_ASSERT(0 && "Timeout");                
    }
}

extern "C"
{
    void LoopBackTest()
    {
        BfRtpTests::LoopBackTest();
    }
}



