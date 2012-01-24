#include "stdafx.h"

#include "Utils/ErrorsSubsystem.h"
#include "Utils/TimerTicks.h"
#include "E1App/AppCpuUsage.h"
#include "E1App/ModeLed.h"
#include "iCore/ThreadRunner.h"
#include "iCore/MsgObject.h"
#include "iCore/MsgTimer.h"
#include "iLog/LogManager.h"
#include "iLog/LogWrapper.h"
#include "BfDev/BfWatchdog.h"
#include "DevIpTdm/BfLed.h"
#include "BfDev/BfUartSimpleCore.h" 
#include "BfDev/SysProperties.h"
#include "DevIpTdm/AozInit.h"
#include "BfTdm/TdmProfile.h"

#include "SafeBiProto/SbpRecvPack.h"
#include "SafeBiProtoExt/SbpUartTransport.h"
#include "SafeBiProtoExt/SbpUartTransportFactory.h"
#include "SafeBiProtoExt/SbpConnProfile.h"
#include "SafeBiProtoExt/SbpConnection.h"

#include "EchoAppBody.h"
#include "EchoAppMain.h"
#include "SfxProto.h"



/*
#include "LowTransform.h"
#include "LowTransformT.h"
#include "LowBench.h" */

// --------------------------------------------------------

namespace
{
    using namespace EchoApp;
    using SBProtoExt::ISbpConnection;
    using boost::scoped_ptr;
    using boost::shared_ptr;
    using Utils::SafeRef;
    using SBProto::SbpRecvPack;

    // debug 
    enum
    {
        CAutoInit               = false,
        CDoAozInit              = false,
        CUartLogInterval        = 0,//5 * 1000,
        CDisableTdmCopy         = false,
        CTracerDropTime         = 0,  // 0 -- for disable

        // enable echo at startup
        CEnableEchoChMask       =  0xffffbffe, //0xfffffffc,
        CEchoTaps               = 64,        
        
        CLogPrematureUartData   = 0, // 4000 mcs, 0 - disable
        
        // user contex profiling
        // todo 1 named profiler
        CEnableTdmProfiling             = false,
        CEnableBlockProcProfiling       = false, 
        CEnablePollTimerProfiling       = false,
        CCmdProfiler                    = false,
    };

    enum
    {
        CPollInterval = 50,
    };

    struct AppParams
    {
        const EchoAppProfile &Profile;
        E1App::AppCpuUsage &CpuUsage;

        AppParams(const EchoAppProfile &profile, E1App::AppCpuUsage &cpuUsage) :
            Profile(profile), CpuUsage(cpuUsage)
        {
        }

    };
    
    // -----------------------------------------
    
    class TickProfiler : public BfTdm::ITdmProfiler
    {
        Utils::TimerTicks m_timer;
        Utils::StatElementForInt m_stats;
        bool m_on;
        
    public:
        TickProfiler()
        {
            Reset();
        }

        void Reset()
        {
            m_timer.Reset();
            m_stats.Reset();
            m_on = false;
        }

        const Utils::StatElementForInt& GetStat() const
        {
            return m_stats;
        }

    // ITdmProfiler impl:
        void OnProcessingBegin()
        {
            ESS_ASSERT(!m_on);
            m_timer.Reset();
            m_on = true;
        }

        void OnProcessingEnd()
        {
            ESS_ASSERT(m_on);
            m_stats.Add( m_timer.Get() );
            m_on = false;
        }
    };

    // -----------------------------------------
    
        
    struct CmdStat
    {
    	int Ms;
        std::string Cmd;
        
        CmdStat()
        {
        	Ms = 0;            	
        }                                         
    };
        
    class CmdProfiler
    {
    	Utils::TimerTicks t;
    	std::string m_cmd;        	            
    	CmdStat& m_stat;
        
     public:
        CmdProfiler(CmdStat& stat, const std::string& cmd) : m_stat(stat), m_cmd(cmd)
        {            	
        	t.Reset();            	
        }
        
        ~CmdProfiler()
        {
        	if (t.Get() <= m_stat.Ms) return;
        	if (MainCmdInit::CName() == m_cmd) return; //не учитывать инит
        	
        	m_stat.Ms = t.Get();
            m_stat.Cmd = m_cmd;
        }
    };
    
    
    // -----------------------------------------

    class UartDebug
    {
        Utils::TimerTicks m_timer;
        BfDev::BfUartSimpleCore m_uart;

        static int GetFreq() { return BfDev::SysProperties::Instance().getFrequencySys(); }

    public:

        UartDebug(int period) : m_uart(GetFreq(), 0, 115200)
        {
            m_timer.Set(period, true);
        }

        bool Process()
        {
            return m_timer.TimeOut();
        }

        void Send(const std::string &s)
        {
            m_uart.Send( s.c_str() );
            m_uart.Send(13);
            m_uart.Send(10);
        }
    };

    // -----------------------------------------

    struct TLed
    {
        static void Set(bool on)
        {
            DevIpTdm::BfLed::SetColor(on ? DevIpTdm::GREEN : DevIpTdm::OFF); 
        }
    };

    // -----------------------------------------

    class ErrorsCollector
    {
        typedef std::list<std::string> ListError;

        const int m_maxErrors;
        int m_count;
        ListError m_errors;

    public:

        ErrorsCollector(int maxErrors) : m_maxErrors(maxErrors), m_count(0)
        {            
        }

        void Add(const std::string &error)
        {
            ++m_count;

            // store text
            std::ostringstream oss;
            oss << m_count << "@" << Platform::GetSystemTickCount() << ": " << error;
                        
            m_errors.push_back( oss.str() );

            // pop to m_maxErrors
            while(m_errors.size() > m_maxErrors) m_errors.pop_front();
        }

        int Count() const { return m_count; }

        std::string LastErrors() const
        {
            std::ostringstream oss;

            ListError::const_iterator i = m_errors.begin();

            while(i != m_errors.end())
            {
                oss << (*i);
                ++i;

                if (i != m_errors.end()) oss << " // ";
            }

            std::string res = oss.str();

            if (res.empty()) res = "(empty)";

            return res;
        }

    };

    // -----------------------------------------

    class DebugTracer
    {
        Utils::TimerTicks m_timer;
        std::vector<std::string> m_data;

    public:

        DebugTracer(int timeout)
        {
            m_timer.Set(timeout, false);
        }

        void Trace(const std::string &s)
        {
            m_data.push_back(s);
        }

        bool Timeout()
        {
            return m_timer.TimeOut();
        }

        void DropData()
        {
            UartDebug uart(1);

            uart.Send("=== Trace ===");
            
            for(int i = 0; i < m_data.size(); ++i)
            {
                uart.Send(m_data.at(i));
            }

            uart.Send("===  End  ===");
        }

    };

    // -----------------------------------------

    class EchoApplication : 
        public iCore::MsgObject,
        public SBProtoExt::ISbpConnectionEvents,
        public SBProtoExt::ISpbRawDataCallback,
        public IEchoRecvSide
    {
        // startup
        AppParams m_param;

        // log
        iLogW::LogManager m_log;
        scoped_ptr<iLogW::LogSession> m_logSession;
        iLogW::LogRecordTag m_tagInfo;
        iLogW::LogRecordTag m_tagErr;

        // runtime
        E1App::ModeLed<TLed> m_led;
        iCore::MsgTimer m_pollTimer;
        scoped_ptr<BfDev::BfWatchdog> m_watchdog;
        scoped_ptr<EchoAppBody> m_body;
        ErrorsCollector m_errorsCollector;

        // UART objects
        scoped_ptr<ISbpConnection> m_uart;
        scoped_ptr<UartDebug> m_uartDebug;
        scoped_ptr<DebugTracer> m_tracer;     
        
        // profiling
        scoped_ptr<TickProfiler> m_timerProfiler;
        scoped_ptr<TickProfiler> m_blocProcProfiler;
        CmdStat m_cmdStat;
        int m_stateReqCount;


        void InitUART()
        {
            // create SBP/UART transport
            SBProtoExt::SbpUartProfile prof;
            prof.BoudRate = CSfxProtoUartSpeed;
            prof.BuffCapacity = CSfxProtoUartSpeed * CPollInterval / 1000;

            std::string err;
            boost::shared_ptr<SBProto::ISbpTransport> transport =
                SBProtoExt::SbpUartTransportFactory::CreateTransport(0, prof, err, true);            

            ESS_ASSERT(transport != 0);

            // create SPB connection 
            {
                SBProtoExt::SbpConnProfile prof;
                prof.m_logCreator = m_logSession->LogCreator();   
                prof.SetTransport( transport );
                prof.m_user = this;
                prof.m_rawDataCallback = this;
                
                prof.m_sbpSettings.setReceiveEndTimeoutMs(800); // workaround uart trash on startup

                m_uart.reset(new SBProtoExt::SbpConnection(getMsgThread(), prof));
                m_uart->ActivateConnection();
            }
        }

        void SendDebugInfo()
        {
            std::ostringstream oss;
            oss << "CPU " << m_param.CpuUsage.CpuLoad() << "%";

            if (m_body != 0)
            {
                oss << "; " << m_body->Stats().ToString();
            }

            m_uartDebug->Send(oss.str());
        }

        void StopTrace()
        {
            // stop all UARTs
            m_uart.reset();
            m_uartDebug.reset();

            m_tracer->DropData();

            ESS_HALT("StopTrace!");
        }

        void OnPollTimer(iCore::MsgTimer *pT)
        {        	
        	if (CEnablePollTimerProfiling) m_timerProfiler->OnProcessingBegin();        	        	
        	
            // process SBP connection 
            if (m_uart != 0)
            {
                m_uart->Process();

                // peek events
                if (m_body != 0)
                {
                    m_body->SendAllEvents( m_uart->Proto() );
                }
            }

            // UART debug
            if ((m_uartDebug != 0) && (m_uartDebug->Process()))
            {
                SendDebugInfo();
            }

            // update LED
            m_led.Update( Platform::GetSystemTickCount() );

            // tracer
            if (m_tracer != 0)
            {
                if (m_tracer->Timeout()) StopTrace();
            }
                        
            if (CEnablePollTimerProfiling) m_timerProfiler->OnProcessingEnd();
        }

        void LogError(const std::string &msg)
        {
            m_errorsCollector.Add(msg);

            if (m_tracer != 0) m_tracer->Trace(msg);

            if (!m_logSession->LogActive()) return;

            *m_logSession << m_tagErr << msg << iLogW::EndRecord;
        }

        void LogInfo(const std::string &msg)
        {
            if (!m_logSession->LogActive()) return;

            *m_logSession << m_tagInfo << msg << iLogW::EndRecord;
        }

        void DoAutoInit()
        {
            CmdInitData data;
            data.ProtoVersion = CSfxProtoVersion;
            data.T1Mode = false;
            data.UseAlaw = true;
            data.VoiceChMask = 0xffffffff;

            OnCmdInit(data);

            // enable echo at startup
            if (CEchoTaps > 0)
            {
                for(int i = 0; i < 32; ++i)
                {
                    if (CEnableEchoChMask & (1 << i))
                    {
                        OnCmdEcho(i, CEchoTaps);
                    }
                }
            }

        }

        void ResetBody()
        {
            if (m_body == 0) return;

            LogInfo("Body destroy!");
            m_body.reset();
        }

    // SBProtoExt::ISpbRawDataCallback impl
    private:

        void OnSbpRawDataRecv(const void *pData, size_t size) 
        {       	        	        	
        	if (CLogPrematureUartData && Platform::GetSystemTickCount() < CLogPrematureUartData)
        	{
        		std::ostringstream ss;
        		ss << "Premature uart data: [" << Platform::GetSystemTickCount() << "]" 
        		<< Utils::DumpToHexString(pData, size);
        		
        		LogError(ss.str());        		      		
        	}
        	
            if (m_tracer == 0) return;

            std::string s = Utils::DumpToHexString(pData, size);
            m_tracer->Trace("<< " + s);
        }

    // IEchoRecvSide impl
    private:

        void OnCmdInit(const CmdInitData &data)
        {
            ResetBody();

            LogInfo("Body create!");
            m_body.reset( 
                new EchoAppBody(data, getMsgThread(), 
                m_log.LogCreator(), m_logSession->LogActive(), CEnableTdmProfiling, m_blocProcProfiler.get()) 
                );

            m_body->EnableCopy(!CDisableTdmCopy);
            
                        
            if (m_uart) 
            	EchoResponse::Send(m_uart->Proto(), EchoResp::ER_Ok, "Init complete");            
            
        }

        void OnCmdEcho(int chNum, int taps)
        {
            if (m_body == 0)
            {
                LogError("OnCmdEcho -> null body");
                return;
            }

                
            m_body->CmdEcho(chNum, taps);
        }

        void OnCmdSetFreqRecvMode(int chNum, const std::string &recvName, 
            const std::string &params, bool enable)
        {
            if (m_body == 0)
            {
                LogError("OnCmdSetFreqRecvMode -> null body");
                return;
            }

            m_body->SetFreqRecvMode(chNum, recvName, params, enable);
        }

        void OnCmdRequestState()
        {               	            
            EchoAppStats stats;             

            stats.IsEmpty = false;

            stats.CpuUsage = m_param.CpuUsage.CpuLoad();
            stats.BuildInfo = m_param.Profile.BuildInfo;

            if (m_body != 0)
            {
                const TdmStat &s = m_body->Stats();
                stats.BlockCollisions = s.BlockCollisions;
                stats.ChannelsWithEcho = s.ChannelsWithEcho;                 
            }

            stats.SbpErrorsCount = m_errorsCollector.Count();
            stats.LastSbpErrors = m_errorsCollector.LastErrors();                                                                          

            stats.HeapFreeBytes = Platform::GetHeapState().BytesFree;
            stats.SysTicks = Platform::GetSystemTickCount();
            stats.SendCounter = m_stateReqCount++;

            EchoStateResp::Send(m_uart->Proto(), stats);            
        }

        /*
        // old vesion, with full profiling support
        void OnCmdRequestState()
        {               	            
            EchoAppStats stats;             
            
            stats.StatReqCount = m_stateReqCount++;
            stats.IsEmpty = false;
            
            stats.CpuUsage = m_param.CpuUsage.CpuLoad();
            stats.BuildInfo = m_param.Profile.BuildInfo;

            if (m_body != 0)
            {
                const TdmStat &s = m_body->Stats();
                stats.BlockCollisions = s.BlockCollisions;
                stats.ChannelsWithEcho = s.ChannelsWithEcho;

                if (CEnableTdmProfiling)
                {
                    {
                        TdmMng::TdmProfiler::Info i = m_body->GetTdmProfilingInfo(true);
                        stats.TdmIrqCount = i.Count;
                        stats.TdmIrqAvgMcs = i.AvgTimeMcs;
                        stats.TdmIrqMaxMcs = i.MaxTimeMcs;
                    }

                    {
                        TdmMng::TdmProfiler::Info i = m_body->GetTdmProfilingInfo(false);
                        stats.TdmDcbCount = i.Count;
                        stats.TdmDcbAvgMcs = i.AvgTimeMcs;
                        stats.TdmDcbMaxMcs = i.MaxTimeMcs;
                    }
                }
                
                if (CEnableBlockProcProfiling && !m_blocProcProfiler->GetStat().Empty())
                {                		                	
                	stats.TdmBlockCount = m_blocProcProfiler->GetStat().Count();
                	stats.ProcBlockAvgMcs = m_blocProcProfiler->GetStat().Average();
                	stats.ProcBlockMaxMcs = m_blocProcProfiler->GetStat().Max();
                	
                	m_blocProcProfiler->Reset();
                }                
                
                if (CEnablePollTimerProfiling && !m_timerProfiler->GetStat().Empty())
                {
                	stats.PollTimerProcCount = m_timerProfiler->GetStat().Count();
                	stats.PollTimerProcAvgMcs = m_timerProfiler->GetStat().Average();
                	stats.PollTimerProcMaxMcs = m_timerProfiler->GetStat().Max();
                	
                	m_timerProfiler->Reset();
                }                                
                 
            }

            if (CCmdProfiler) 
            {
                stats.MaxCmd = m_cmdStat.Cmd;
                stats.MaxCmdMs = m_cmdStat.Ms;
            }            

            stats.SbpErrorsCount = m_errorsCollector.Count();
            stats.LastSbpErrors = m_errorsCollector.LastErrors();                                                                          

            stats.Heap = Platform::GetHeapState().ToString();
            
            EchoStateResp::Send(m_uart->Proto(), stats);            
        } */

        void OnCmdRequestHalt()
        {
            ESS_HALT("Test halt!");
        }

        void OnCmdShutdown()
        {
            ResetBody();
        }

    // SBProtoExt::ISbpConnectionEvents impl
    private:

        void CommandReceived(SafeRef<ISbpConnection> src,  shared_ptr<SbpRecvPack> data)
        {
            LogError("CommandReceived");
        }

        void ResponseReceived(SafeRef<ISbpConnection> src, shared_ptr<SbpRecvPack> data)
        {
            LogError("ResponseReceived");
        }
        
              
        void InfoReceived(SafeRef<ISbpConnection> src, shared_ptr<SbpRecvPack> data)
        {
            ESS_ASSERT(data != 0);                              
            CmdProfiler profiler(m_cmdStat, (*data)[0].AsString());    

            try
            {           	            	
                if (MainCmdInit::ProcessPacket(*data, *this))               return;                                           
                
                if (MainCmdEcho::ProcessPacket(*data, *this))               return;
                if (MainCmdSetFreqRecvMode::ProcessPacket(*data, *this))    return;
                if (MainCmdRequestState::ProcessPacket(*data, *this))       return;
                if (MainCmdShutdown::ProcessPacket(*data, *this))           return;
                if (MainCmdRequestHalt::ProcessPacket(*data, *this))        return;

                LogError("Unknown packet!");
            }
            catch(/* const */ EchoException &e)
            {
                EchoResp resp = e.getProperty();
                EchoResponse::Send(src->Proto(), resp.getCode(), resp.getText());
            }
            catch(/* const */ std::exception &e)
            {
                std::string msg = "Exception in InfoReceived; ";
                msg += e.what();
                LogError(msg);
            }         
        }

        void ProtocolError(SafeRef<ISbpConnection> src, shared_ptr<SBProto::SbpError> err)
        {
            // ignore first error -- trash from UART till BfMainE1 startup
            if ( (Platform::GetSystemTickCount() < 10000) && 
                 (m_errorsCollector.Count() == 0) )
            {
                if ( typeid(*err.get()) == typeid(SBProto::SbpWaitingEndOfReceiveTimeout) )
                {
                    return;
                }
            }

            // report
            std::string msg = "ProtocolError; ";
            msg += err->ToString();
            LogError(msg);
        }

        void ConnectionActivated(SafeRef<ISbpConnection>) 
        { 
            // nothing
        }        

        void ConnectionDeactivated(SafeRef<ISbpConnection> src,  const std::string &errInfo)
        {
            LogError("ConnectionDeactivated " + errInfo);
        }

    public:

        EchoApplication(iCore::IThreadRunner &runner, AppParams &param) :
          iCore::MsgObject(runner.getThread()), 
          m_param(param),
          m_log(m_param.Profile.LogConfig),
          m_pollTimer(this, &EchoApplication::OnPollTimer),
          m_errorsCollector(4),
          m_stateReqCount(0) 
        {   
        	
        	
        	// profiling
        	{
        		if (CEnablePollTimerProfiling) m_timerProfiler.reset(new TickProfiler);
        		if (CEnableBlockProcProfiling) m_blocProcProfiler.reset(new TickProfiler);        		
        	}
        	
            // log init
            {
                m_logSession.reset( m_log.LogCreator()->CreateSession(m_param.Profile.AppName, m_param.Profile.LogTrace) );
                m_tagInfo = m_logSession->RegisterRecordKindStr("Info");
                m_tagErr = m_logSession->RegisterRecordKindStr("ERR");
            }

            // UART 
            if (CUartLogInterval > 0)
            {
                ESS_ASSERT(CTracerDropTime <= 0);
                m_uartDebug.reset( new UartDebug(CUartLogInterval) );
            }
            else
            {
                if (CTracerDropTime > 0) 
                {
                    m_tracer.reset( new DebugTracer(CTracerDropTime) );
                }

                InitUART();
            }

            m_led.Mode(1000, 2000);

            m_pollTimer.Start(CPollInterval, true);

            // auto init
            if (CAutoInit) 
            {
                PutMsg(this, &EchoApplication::DoAutoInit);
            }

        }

    };
    
}  // namespace


// --------------------------------------------------------

namespace EchoApp
{
        
    void RunEchoApp(const EchoAppProfile &profile)
    {
        // AOZ init (debug)
        if (CDoAozInit) 
        {
            DevIpTdm::AozInit::LockTimers();
            ESS_ASSERT( DevIpTdm::AozInit::InitSync() );
        }

        // params
        E1App::AppCpuUsage cpuUsage( profile.MonitorCpuUsage );		
        AppParams params(profile, cpuUsage);

        // run
        iCore::ThreadRunner app(Platform::Thread::NormalPriority, 0);
        bool res = app.Run<EchoApplication>( "EchoApplication", params, 0, profile.ThreadRunnerSleepInterval );		
        ESS_HALT("");
    }
        
}  // namespace EchoApp
