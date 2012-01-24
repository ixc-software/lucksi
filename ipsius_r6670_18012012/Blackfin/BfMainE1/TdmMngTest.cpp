#include "stdafx.h"

#include "Platform/Platform.h"
#include "iCore/MsgObject.h"
#include "iCore/ThreadRunner.h"
#include "iCore/MsgTimer.h"
#include "TdmMng/TdmManager.h"
#include "iLog/LogManager.h"
#include "BfDev/VdkThreadDcb.h"
#include "BfDev/E1LedsAssign.h"
#include "Utils/IntToString.h"
#include "Ds2155/boardds2155.h"
#include "BfDev/SysProperties.h"
#include "TdmMng/TestRtpToTdm.h"
#include "UtilsTests/MemoryPoolTests.h"

// #include "E1App/AppConfig.h"
#include "E1App/AppCpuUsage.h"
#include "E1App/MngLwip.h"

#include "TdmMngTest.h"

// -------------------------------------------------

using namespace iCore;

using boost::scoped_ptr;

using TdmMng::TdmException;
using TdmMng::TdmErrorCode;
using TdmMng::TdmManagerProfile;
using TdmMng::TdmManager;
using TdmMng::TdmStream;
using TdmMng::ITdmChannel;

using iLogW::LogSettings;
using iLogW::LogManager;
using iLogW::LogSession;
using iLogW::LogRecordTag;
using iLogW::EndRecord;
using iLogW::ILogSessionCreator;

using Ds2155::LiuProfile;
using Ds2155::Hdlc;
using Ds2155::HdlcProfile;
using Ds2155::BoardDs2155;


// -------------------------------------------------

enum
{
    CStatTime                  = 5 * 1000,

    CCreateBoard               = false,

    CPrintIrqProcessInfo       = true,

    // Create/destroy
    CCreateTDM                 = 1 * 1000,
    CDestroyTDM                = 0, // 51 * 1000, // 20 * 1000,

    // RTP send
    CEnableRtpSend             = true,  // send to CRtpRecvHost + CRtpRecvPort
    CEnableRtpSendLog          = true,   
    CRtpSendChNumber           = 1,
    CRtpSendChCount            = 2, 

    // TDM/TDM
    CEnableTdmToTdmSend        = false,
    CEnableTdmToTdmSendLog     = true,   
    CTdmToTdmChNumber          = 1,      
    CTdmToTdmChPairCount       = 7, 

    // Stream params
    CDevSportNum               = 0,
    CDevSportCh                = 0,

};

const char *CStreamName = "TDM0";

// const char *CChannelSet = "1..3";
const char *CChannelSet = "1..15, 17..31";

const char *CRtpRecvHost = "192.168.0.1";
const int   CRtpRecvPort = 10620;  // 10620

// -------------------------------------------------

namespace
{
	using namespace E1App;

    struct AppParams
    {
        AppCpuUsage &CpuThread;
        const E1App::E1AppConfig &Config;

        AppParams(const E1App::E1AppConfig &config, AppCpuUsage &cpuThread) : 
            CpuThread(cpuThread), Config(config)
        {            
        }
    };

    // ---------------------------------------

    class Application : 
        public MsgObject,
        boost::noncopyable 
    {
        AppParams m_param;
        std::string m_ip;

        LogManager m_log;
        scoped_ptr<LogSession> m_session;

        MsgTimer m_timer;
        bool m_ledIsOn;
        int  m_lastStatTime;
        int m_prevHeap;
        bool m_destroyed;

        std::vector<int> m_channels;

        scoped_ptr<TdmManager>  m_tdm;     // TDM/RTP
        scoped_ptr<BoardDs2155> m_board;   // LIU/HDLC

        void LogChannelStats(ITdmChannel &ch, bool rtp, const std::string &desc)
        {
            TdmMng::TdmChannelStatistic stats;

            ch.GetLastCallStats(stats);

            if (rtp)
            {
                *m_session << desc << " RTP buff " << stats.RtpBuffStat.ToString()    << EndRecord;
                *m_session << desc << " RTP sess " << stats.RtpStat.ToString()     << EndRecord;
            }
            else
            {                
                *m_session << desc << " " << stats.TdmBuffStat.getAsString() << EndRecord;
            }
                                
        }

        void LogRtpSimpleStats(ITdmChannel &ch, const std::string &desc)
        {
            TdmMng::TdmChannelStatistic stats;
            ch.GetLastCallStats(stats);

            *m_session << desc << " ";

            if (!stats.RtpStat.SendPackSizeStat.Empty())
            {
                *m_session << " | RTP out " << stats.RtpStat.SendPackSizeStat.Sum();
            }

            if (!stats.RtpBuffStat.BufferingRtpPackSize.Empty())
            {
                *m_session
                    << " | Out count "  << stats.RtpBuffStat.SuccessReadFromBuffCount
                    << ", RTP in buff " << stats.RtpBuffStat.BufferingRtpPackSize.Sum() 
                    << ", goes buff "   << stats.RtpBuffStat.GoesBufferingCount; // SlipCount;
            }

            *m_session  << EndRecord;
        }

        void StatsForRTP()
        {
            const bool CSimpleLog = true;
            int startIndx = GetIndexForChannel(CRtpSendChNumber);

            for(int i = 0; i < CRtpSendChCount; ++i)
            {
                int chNum = m_channels.at(startIndx + i);

                std::string prefix;
                prefix = "RTP Ch " + Utils::IntToString(chNum);

                if (CSimpleLog) LogRtpSimpleStats(Channel(chNum), prefix);
                           else LogChannelStats( Channel(chNum), true, prefix);
            }            

            /*
            for(int i = 0; i < CRtpSendChCount; ++i)
            {
                int chNum = CRtpSendChNumber + i;
                int remotePort = CRtpRecvPort + (i * 2);

                LogChannelStats( Channel(chNum), true, "RTP Ch " + Utils::IntToString(chNum) );
            } */           
        }

        void StatsForTdmToTdm()
        {
            int startIndx = GetIndexForChannel(CTdmToTdmChNumber);
            
            for(int i = 0; i < CTdmToTdmChPairCount; ++i)
            {
                int firstChIndx = startIndx + (i * 2);

                std::ostringstream ss;
                ss << "TDM2TDM " << i << "/" << m_channels.at(firstChIndx);

                ITdmChannel &ch0 = Channel( m_channels.at(firstChIndx) );
                ITdmChannel &ch1 = Channel( m_channels.at(firstChIndx + 1) );

                LogChannelStats(ch0, false, ss.str());
                LogChannelStats(ch1, false, ss.str() + "+1"); 
            }

            /*
            for(int i = 0; i < CTdmToTdmChPairCount; ++i)
            {
                int firstCh = CTdmToTdmChNumber + (i * 2);

                std::ostringstream ss;
                ss << "TDM2TDM " << i << "/" << firstCh;

                LogChannelStats( Channel(firstCh),   false, ss.str());
                LogChannelStats( Channel(firstCh+1), false, ss.str() + "+1");
            } */

        }

        void PrintStat()
        {
            int currHeap = heap_space_unused(0);

            *m_session << "CPU " << m_param.CpuThread.CpuLoad() << "%" 
                << "; Heap " << Utils::IntToStringSign(m_prevHeap - currHeap)
                << " free "  << currHeap
                << EndRecord;

            m_prevHeap = currHeap;

            if (m_tdm.get() == 0) return;

            if (CPrintIrqProcessInfo)
            {
                *m_session << "ProcessIRQ " << m_tdm->getStat().getAsString() << EndRecord;
            }

            if ((CEnableRtpSend) && (CEnableRtpSendLog))
            {
                StatsForRTP();
            }

            if ((CEnableTdmToTdmSend) && (CEnableTdmToTdmSendLog))
            {
                StatsForTdmToTdm();
            }

        }

        void DestroyTDM()
        {
            *m_session << "Enter destroy..." << EndRecord;

            m_tdm.reset( 0 );
            m_destroyed = true;

            *m_session << "Destroy done!" << EndRecord;
        }

        void OnTimer(MsgTimer *pT)
        {
            int curr = Platform::GetSystemTickCount();

            // keep alive
            {
                m_ledIsOn = !m_ledIsOn;
                BfDev::BfLed::Set(CLed_MsgAlive, m_ledIsOn);                
            }

            // Ethernet state
            BfDev::BfLed::Set(CLed_Ethernet, Stack::Instance().IsEstablished());

            // create TDM
            if ((CCreateTDM > 0) && (curr > CCreateTDM))
            {
                if ((m_tdm.get() == 0) && (!m_destroyed))
                {
                    CreateTDM();
                }
            }
            
            // destroy TDM
            if ((CDestroyTDM > 0) && (Platform::GetSystemTickCount() > CDestroyTDM))
            {
                if ((m_tdm.get() != 0) && (!m_destroyed))
                {
                    DestroyTDM();
                }
            }

            // stats
            int diff = curr - m_lastStatTime;

            if (diff >= CStatTime)
            {
                PrintStat();

                m_lastStatTime = curr;
            }
        }

        ITdmChannel& Channel(int number)
        {
            ESS_ASSERT(m_tdm.get() != 0);

            TdmStream *pS = m_tdm->Stream(CStreamName);
            ESS_ASSERT(pS != 0);

            ITdmChannel *pC = pS->Channel(number);
            ESS_ASSERT(pC != 0);

            return *pC;
        }

        static void ChannelStart(ITdmChannel &ch, const std::string ip, int port, bool withRecv)
        {
            ch.StartSend(ip, port, false, true);
            if (withRecv) ch.StartRecv();
        }

        void EnableRtpSend()
        {
            // Channel(CRtpSendChNumber).StartSend(CRtpRecvHost, CRtpRecvPort, true, false, true);

            int startIndx = GetIndexForChannel(CRtpSendChNumber);

            for(int i = 0; i < CRtpSendChCount; ++i)
            {
                int chNum = m_channels.at(startIndx + i);
                int remotePort = CRtpRecvPort + (i * 2);

                ChannelStart( Channel(chNum), CRtpRecvHost, remotePort, true );
            }            
        }


        void EnableTdmToTdmSendPair(int ch0Num, int ch1Num)
        {
            ITdmChannel &ch0 = Channel(ch0Num);
            ITdmChannel &ch1 = Channel(ch1Num);

            {
                *m_session << "Ch0 " << ch0.GetChNumber() << ", port " << ch0.GetRtpPort() << EndRecord;
                *m_session << "Ch1 " << ch1.GetChNumber() << ", port " << ch1.GetRtpPort() << EndRecord;
            }

            ChannelStart(ch0, m_ip, ch1.GetRtpPort(), true);
            ChannelStart(ch1, m_ip, ch0.GetRtpPort(), true);
        }

        void EnableTdmToTdmSend()
        {
            int startIndx = GetIndexForChannel(CTdmToTdmChNumber);
            
            for(int i = 0; i < CTdmToTdmChPairCount; ++i)
            {
                int firstChIndx = startIndx + (i * 2);

                EnableTdmToTdmSendPair( m_channels.at(firstChIndx), 
                                        m_channels.at(firstChIndx + 1) );
            }
        }

        int GetIndexForChannel(int chValue, bool assertError = true)
        {
            for(int i = 0; i < m_channels.size(); ++i)
            {
                if (m_channels.at(i) == chValue) return i;
            }

            if (assertError) ESS_HALT("Not found!");

            return -1;
        }


        void CreateTDM()
        {
            // TDM

			TdmManagerProfile profile( m_param.Config.TdmManagerCfg(m_log, getMsgThread()) );  // AppConfig::CreateTdmManagerProfile( m_log, getMsgThread() )
            m_ip = profile.LocalIp;
            m_tdm.reset( new TdmManager(profile) );

            // stream
            TdmMng::TdmChannelsSet chSet;
            ESS_ASSERT( chSet.Parse(CChannelSet) );
           
            m_tdm->RegisterStream(CStreamName, chSet, CDevSportNum, CDevSportCh, true /* A-low */);

            // fill m_channels
            {
                m_channels.clear();
                int max = chSet.MaxValue();

                for(int i = 0; i <= max; ++i)
                {
                    if ( chSet.IsExist(i) ) m_channels.push_back(i);                    
                }
            }


            // board
            if (CCreateBoard)
            {
                m_board.reset
                ( 
                    new BoardDs2155(m_param.Config.AppName, Ds2155::CBaseAddress, LiuProfile(), m_param.Config.HdlcConfig )                 
                );
            }

            // RTP send
            if (CEnableRtpSend) EnableRtpSend();

            // TDM to TDM
            if (CEnableTdmToTdmSend) EnableTdmToTdmSend();

            // Log
            *m_session << "TDM created!" << EndRecord;
          
        }

    public:

        Application(IThreadRunner &runner, AppParams &param) :
            MsgObject(runner.getThread()),
            m_param(param),
            m_log( param.Config.LogConfig ),
            m_timer(this, &Application::OnTimer)
        {
            // log
            m_session.reset( m_log.CreateLogSesion("TDM/Test", true) );

            // timer
            m_timer.Start(500, true);
            m_ledIsOn = false;
            m_lastStatTime = Platform::GetSystemTickCount();
            m_destroyed = false;

            // heap
            m_prevHeap = heap_space_unused(0);

            // TDM
            // CreateTDM();
        }

    };
	
}  // namespace


// -------------------------------------------------

void RunTdmMngTest(const E1App::E1AppConfig &config)
{
    AppCpuUsage cpuUsage( config.MonitorCpuUsage );
    // std::cout << cpuUsage.CalibrationInfo() << std::endl;
    
    // buff test
    // TestBuffering::TestRtpToTdmBuff();
    
    // pool test
    // UtilsTests::MemPoolExceptionTest();

    {
        // BfDev::DisableDataCache cacheDisabler;      
    }

    AppParams params(config, cpuUsage);

    // run
    iCore::ThreadRunner test(Platform::Thread::NormalPriority, 0);
    bool res = test.Run<Application>( params, 0, config.ThreadRunnerSleepInterval );
}

