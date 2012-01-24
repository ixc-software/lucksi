#include "stdafx.h"
#include "AozExec.h"

#include "Platform/Platform.h"

#include "iCore/ThreadRunner.h"
#include "iCore/MsgObject.h"
#include "iCore/MsgTimer.h"
#include "iLog/LogManager.h"
#include "BfDev/E1LedsAssign.h"
#include "BfDev/SysProperties.h"
#include "E1App/AppCpuUsage.h"
#include "TdmMng/TdmHAL.h"
#include "DevIpTdm/BfKeys.h"
#include "Utils/IntToString.h"
#include "DevIpTdm/AozInit.h"
#include "Utils/MsDateTime.h"

#include "AozRawTest.h"
#include "AozBoard.h"
#include "AozLwBoard.h"

// --------------------------------------------------

namespace
{
    using namespace E1App;
    using boost::scoped_ptr;

    typedef /* DrvAoz::AozLwBoard */ DrvAoz::AozBoard AozBoardType;

    const std::string CLogName = "AOZ";

    struct Params
    {
        AppCpuUsage &CpuUsage;

        Params(AppCpuUsage &cpuUsage) : CpuUsage(cpuUsage)
        {
        }
    };

    class AozExec : 
        public iCore::MsgObject,
        public TdmMng::ITdmHalEvents
    {
        Params &m_param;
        iLogW::LogManager m_log;
        iCore::MsgTimer m_pollTimer;
        bool m_prevKeyPressed;

        scoped_ptr<iLogW::LogSession> m_session;
        iLogW::LogRecordTag m_tagInfo;

        scoped_ptr<TdmMng::TdmHAL> m_hal;
        scoped_ptr<AozBoardType> m_board;

        static iLogW::LogSettings GetLogSettings()
        {
            const char *CUdpLogIp = "192.168.0.1";
            const int CUdpLogPort = 56001;

            iLogW::LogSettings ls;

            ls.StoreRecordsLimit(0);
            ls.StoreCleanTo(0);
            ls.TimestampInd(true);
            ls.CountSyncroToStore(1);  // sync on every records
            ls.UseRecordsReorder(false);
            ls.DublicateSessionNamesDetection(false);
            ls.out().Udp().TraceInd = true;
            ls.out().Udp().DstHost = Utils::HostInf(CUdpLogIp, CUdpLogPort);

            return ls;
        }

        void LogTdmDump(const std::string &prefix, const std::vector<Platform::byte> &data)
        {
            *m_session << m_tagInfo << prefix << ": "
                << Utils::DumpToHexString(data)
                << iLogW::EndRecord;
        }

        void LogTdmSnapshot()
        {
            std::vector<Platform::byte> tdmIn, tdmOut;

            m_board->GetTdmSnapshot(tdmIn, tdmOut);
            LogTdmDump("TdmIn ", tdmIn);
            LogTdmDump("TdmOut", tdmOut);
        }

        void OnPollTimer(iCore::MsgTimer *pT)
        {
            // DataTime capture poll
            {
                Utils::MsDateTime dt;
                dt.Capture();           // TODO -- do not on every iteration
            }

            bool key = DevIpTdm::BfKeys::Get(DevIpTdm::KEY1);

            if (key && !m_prevKeyPressed)
            {
                if (m_session->LogActive())
                {
                    LogTdmSnapshot();
                }
            }

            m_prevKeyPressed = key;
        }

    // ITdmHalEvents
    private:

        void OnTdmHalIrq(int sportNum, TdmMng::ITdmBlock &block)
        {
            m_board->BlockProcess(block);

            // poll events
            ESS_UNIMPLEMENTED;

            /*
            while(m_board->HasEvents())
            {
                DrvAoz::AozEvent event = m_board->PopEvent();

                if (m_session->LogActive())
                {
                    *m_session << m_tagInfo << "Event " << event.Name 
                        << " ch" << event.ChNumber << " " << event.Params
                        << iLogW::EndRecord;
                }

                // iCmp::BfAbEvent::Send(Proto(), m_boards[i]->Name(), 
                //    event.ChNumber, event.Name, event.Params);
            } */
        }

        void OnTdmHalBlockCollision()
        {
            m_board->BlockCollision();            
        }

    public:

        AozExec(iCore::IThreadRunner &runner, Params &param) :
          iCore::MsgObject(runner.getThread()),
          m_param(param),
          m_log( GetLogSettings() ),
          m_pollTimer(this, &AozExec::OnPollTimer)
        {
            // polling
            m_pollTimer.Start(100, true);
            m_prevKeyPressed = false;

            // log
            {
                m_session.reset( m_log.LogCreator()->CreateSession(CLogName, true) );
                m_tagInfo       = m_session->RegisterRecordKindStr("Info");

                *m_session << "Started!" << iLogW::EndRecord;
            }
            
            // hal
            {
                BfTdm::TdmProfile tdmProfile(4, 160, true, BfTdm::CNoCompanding);
                tdmProfile.Name(CLogName);

                TdmMng::TdmHalProfile profile(runner.getThread(), 
                    tdmProfile, tdmProfile, m_session->LogCreator());

                m_hal.reset( new TdmMng::TdmHAL(profile) );
                m_hal->RegisterEventsHandler(this);
            }

            // init data
            iCmp::BfInitDataAoz data = iCmp::BfInitDataAoz::DefaultValue();

            data.DeviceName = CLogName;
            data.StartRtpPort = 12500; 
            data.VoiceChannels = "0 .. 15"; 
            data.UseAlaw = true;

            // TdmMng (for voice)
            // ... 

            // board
            m_board.reset( 
                new AozBoardType(runner.getThread(), m_session->LogCreator(), data, *m_hal) 
            );

        }

        ~AozExec()
        {
            m_hal->UnregisterEventsHandler(this);
        }

    };

    
}  // namespace

// --------------------------------------------------

namespace DrvAoz
{
    
    void RunAoz(const AozExecConfig &config)
    {
    	// RunRawAoz();  // raw version
    	
        // AOZ extra init 
        if (!DevIpTdm::AozInit::InitDone())
        {
            ESS_ASSERT( DevIpTdm::AozInit::InitSync() );  
        }

        AppCpuUsage cpuUsage(config.CpuUsage);
        Params params(cpuUsage);

        // run
        iCore::ThreadRunner test(Platform::Thread::NormalPriority, 0);
        bool res = test.Run<AozExec>("AozExec", params);
    }
    
}  // namespace DrvAoz
