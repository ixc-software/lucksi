#include "stdafx.h"

#include "Utils/HostInf.h"
#include "Utils/MsDateTime.h"
#include "Utils/MemLeak.h"

#include "BfDev/E1LedsAssign.h"
#include "BfDev/SysProperties.h"
#include "BfDev/BfWatchdog.h"

#include "BfBootCore/BroadcastMsg.h"
#include "BfBootCore/GeneralBooterConst.h"
#include "TdmMng/TdmException.h"
#include "DevIpTdm/AozInit.h"
#include "SndMix/MixerError.h"
#include "psbCofidec/PsbCofidecInit.h"

#include "E1AppConfig.h"
#include "E1App/E1AppMisc.h"
#include "E1AppBody.h"
#include "E1AppRunner.h"
#include "E1Application.h"
#include "AppPlatformHook.h"
#include "AppLed.h"
#include "EchoSfxClient.h"
#include "EchoHwClient.h"

namespace 
{   
	enum
	{
		// Info
		CPrintInfoInterval         = 0, // 30 * 1000,
        CDisableLeds               = false, 
	};

    // интервал запуска aсcept для серверного сокета
    // при активной сессии интервал больше, т.к. это блокирующая операция 
    enum
    {
        CServerPollTimeActive       = 100,
        CServerPollTimePassive      = 5 * 1000,
    };

    enum
    {
        CCmpTraceMaxParams = 5,
        CCmpTraceMaxBinary = 6,
    };

    enum
    {
        CEnableWatchdogOnStartup        = true,
        CEnableAutorebootOnStartup      = true,
    };

    class SafeRefHost : public Utils::SafeRefServer {};

    void SafeRefTest()  // + exception test
    {
        try
        {
            ESS_THROW(ESS::Exception);
        }
        catch(/* const */ std::exception &e)
        {
            boost::scoped_ptr<SafeRefHost> m_host( new SafeRefHost() );

            Utils::SafeRef<SafeRefHost> ref( m_host.get() );
            m_host.reset();
        }
    }

};


namespace E1App
{
	using namespace SBProtoExt;
    using Platform::word;

    void RunE1Application(BfBootSrv::IReload& reloader, ICloseApp& closeImpl, const E1App::E1AppConfig &config, const BfBootCore::BroadcastMsg &msg, word cmp)
	{
		AppCpuUsage cpuUsage( config.MonitorCpuUsage );		
		E1ApplicationParams params(config, closeImpl, cpuUsage, msg, cmp);

		// run
		iCore::ThreadRunner test(Platform::Thread::NormalPriority, 0);
		bool res = test.Run<E1Application>( "E1Application", params, 0, config.ThreadRunnerSleepInterval );		
        reloader.Reboot(); // return from Run only under Emulator
	}    

	// ----------------------------------

	E1Application::E1Application(iCore::IThreadRunner &runner, E1ApplicationParams &param) : 
		iCore::MsgObject(runner.getThread()),
        m_perfTimer( BfDev::BfTimerCounter::GetFreeTimer() ),
		m_param(param),
		m_log( param.Config.LogConfig ),
        m_logSession( m_log.CreateSession(param.Config.AppName, param.Config.IsTraceOn) ),
        m_tagInfo( m_logSession->RegisterRecordKindStr("Info") ),
        m_tagError( m_logSession->RegisterRecordKindStr("Error") ),
		m_timer(this, &T::OnTimer),
		m_breakFlag(runner.getCompletedFlag()),
        m_asyncBodyDestroy(false),
        m_asyncDropConnOnException(false),
		m_connections(runner.getThread(), this)
	{
		m_established = false;
		m_timerCycle = 0;

		m_cmpPort = param.CmpPort;
        
        m_broadcast.reset(
            new BfBootSrv::BroadcastAutoSender(getMsgThread(), param.BroadcastMessage, param.Config.BroadcastSrcPort, this)
            );        

        // echo
        {
            EchoModeEnum mode = param.Config.EchoMode;
            IEchoClient *pEcho = 0;

            if ( (mode == emHardware) || mode == emHardwareThenSfx ) 
            {
                pEcho = EchoHwClient::Create();

                if ( (pEcho == 0) && (mode == emHardwareThenSfx) ) 
                {
                    pEcho = new EchoSfxClient(runner.getThread(), 
                                              m_logSession->LogCreator());
                }
            }

            if (pEcho == 0) pEcho = new NullEchoClient();

            m_echo.reset(pEcho);
        }

		// begin polling
		m_timer.Start(param.Config.AppPollTimeMs, true);

		// cpu usage timer            
		if (CPrintInfoInterval > 0)
		{
			m_cpuUsagePrintTimer.Set(CPrintInfoInterval, true);
		}

        m_serverPoll.Set(CServerPollTimeActive, false);

		m_prevFreeHeapValue = Platform::GetHeapState().BytesFree;

        // stats init
        m_stats.StartTimeTicks = Platform::GetSystemTickCount();

        // error handle
        if (CEnableAutorebootOnStartup)
        {
            AppPlatformHookSetup(true, true, true);
        }

        if (CEnableWatchdogOnStartup)
        {
            m_watchdog.reset( new BfDev::BfWatchdog(5 * 1000) );
        }

        // fix interval time 
        m_timerLastCallTicks = Platform::GetSystemTickCount();
	}

	// ----------------------------------
	//	ISbpTcpServerEvents

	void E1Application::NewConnection(const SbpTcpServer *src, 
		boost::shared_ptr<ISbpConnection> connection)
	{
		ESS_ASSERT(m_tcpServer.get() == src);
		
		m_connections.Push(connection);
	}

	// ----------------------------------

	void E1Application::Error(const SbpTcpServer *src, const std::string &err)
	{
		ESS_ASSERT(m_tcpServer.get() == src);

		if (m_logSession->LogActive()) 
		{
			*m_logSession << m_tagError << "Tcp server error: " << err << iLogW::EndRecord;
		}
	}

	// ----------------------------------
	//	SBProto::ISbpConnectionEvents

	void E1Application::CommandReceived(Utils::SafeRef<ISbpConnection> src,
		boost::shared_ptr<SBProto::SbpRecvPack> data)
	{
		ProtoError(src, "CommandReceived");
	}

	// ----------------------------------

	void E1Application::ResponseReceived(Utils::SafeRef<ISbpConnection> src,
		boost::shared_ptr<SBProto::SbpRecvPack> data)
	{
		ProtoError(src, "ResponseReceived");		
	}

    // -----------------------------------------------------------------------

    void E1Application::ProcessCmpPacket( Utils::SafeRef<ISbpConnection> src, boost::shared_ptr<SBProto::SbpRecvPack> data )
    {
        try
        {
            // process global commands
            if (iCmp::PcCmdInitAoz::ProcessPacket(*data, src, *this))      return;
            if (iCmp::PcCmdInitE1::ProcessPacket(*data, src, *this))       return;
            if (iCmp::PcCmdInitPult::ProcessPacket(*data, src, *this))       return;
            if (iCmp::PcCmdGlobalSetup::ProcessPacket(*data, src, *this))  return;
            if (iCmp::PcCmdStateInfo::ProcessPacket(*data, src, *this))    return;

            if (m_body == 0) ESS_THROW_T(TdmMng::TdmException, TdmMng::terCommandBeforeInit);

            m_body->ProcessCmd(src, *data);
        }
        catch(/* const */ TdmMng::TdmException &e)
        {
            const TdmMng::TdmErrorInfo &info =  e.getProperty();
			iCmp::BfResponse::Send(src->Proto(), info.Code(), info.Desc());            
			
			if(info.Code() == TdmMng::terBfInitDataE1 || 
				info.Code() == TdmMng::terBfInitDataAoz)
			{
				// ошибка инициализации, закрываем соединение
            	src->UnbindUser();
			}
        }
        catch(/* const */ SndMix::Error::Base &e)
        {
            std::ostringstream oss;
            oss << e.what();
            iCmp::BfResponse::Send(src->Proto(), TdmMng::terConferenceError, oss.str());            
        }
		catch(/* const */ iCmp::BfInitDataBase::BadCmpProtoVersion &e)
		{
			m_stats.CmpCommandsFatalExceptions++;
			std::string msg(e.getTextMessage());
			iCmp::BfResponse::Send(src->Proto(), TdmMng::terCmpProtoError, msg);
			ProtoError(src, msg);
		}
        catch(/* const */ std::exception &e)
        {
            m_stats.CmpCommandsFatalExceptions++;

            std::string msg("Unexpected exception during InfoReceived: ");
            msg += e.what();
            ProtoError(src, msg);
        }
    }

	// -----------------------------------------------------------------------

	void E1Application::InfoReceived(Utils::SafeRef<ISbpConnection> src, 
		boost::shared_ptr<SBProto::SbpRecvPack> data)
	{
        // stats
        m_stats.CmpPackInSession++; 
        m_stats.CmpPackInTotal++;

        // process packet
        m_perfTimer.Reset();

        ProcessCmpPacket(src, data);

        m_stats.CmpCommandTime.Add( m_perfTimer.GetCounter() );
	}

	// ----------------------------------

	void E1Application::ProtocolError(Utils::SafeRef<ISbpConnection> src,
		boost::shared_ptr<SBProto::SbpError> err)
	{
		ProtoError(src, err->ToString());		
	}

	// ----------------------------------

	void E1Application::ConnectionActivated(Utils::SafeRef<ISbpConnection> src)
	{}

	// ----------------------------------

	void E1Application::ConnectionDeactivated(Utils::SafeRef<ISbpConnection> src,
		const std::string &error)
	{
		ProtoError(src, error);
	}

	// ----------------------------------

	void E1Application::Log(iLogW::LogRecordTag tag, const std::string &s)
	{
		if (!m_logSession->LogActive()) return; 

		*m_logSession << tag << s << iLogW::EndRecord;
	}

	// ----------------------------------

	void E1Application::OnPhConnect()
	{
		Log(m_tagInfo, "PhConnected");
		StartNetworkActivity();
	}

	// ----------------------------------

	void E1Application::OnPhDisconnect()
	{
		Log(m_tagInfo, "PhDisconnect");

		StopNetworkActivity();
		DestroyBodyRequest();
        m_broadcast->UpdateOwnerInfo("");
	}


	// ----------------------------------

	void E1Application::DestroyBodyRequest()
	{
        if (!m_asyncBodyDestroy) 
        {
            DestroyBodyImpl();
            return;
        }

        if (m_logSession->LogActive())
        {
            *m_logSession << m_tagInfo << "Body destroyed requested " << iLogW::EndRecord;
        }

        PutMsg(this, &T::DestroyBodyImpl);
	}

    // ----------------------------------

    void E1Application::DestroyBodyImpl()
    {
        if (m_body == 0) return;

        m_body.reset();

        if (m_logSession->LogActive())
        {
            *m_logSession << m_tagInfo << "Body destroyed, heap " 
                << (int)(Platform::GetHeapState().BytesFree - m_heapOnCreateBody) << iLogW::EndRecord;
        }
    }

	// ----------------------------------

	void E1Application::ProtoError(Utils::SafeRef<ISbpConnection> src,
		const std::string &msg)
	{
		Log(m_tagError, msg);
        
		if (m_body != 0 && m_body->IsActiveConnection(src)) 
        {
            DestroyBodyRequest();
        }
		else
        {
            src->UnbindUser();
        }

        m_broadcast->UpdateOwnerInfo("");
	}

	// ----------------------------------

	void E1Application::LogState() const
	{
		if (!m_logSession->LogActive()) return;            // log inactive

		if (CPrintInfoInterval <= 0) return;            // interval == 0

		// std::cout << "CPU " << data.CPUUsagePercent() << "%" << endl;

		int curr = Platform::GetHeapState().BytesFree;
		int diff = m_prevFreeHeapValue - curr;

		m_prevFreeHeapValue = curr;

		*m_logSession << m_tagInfo << "CPU " 
			<< m_param.CpuThread.CpuLoad() << "%" 
			<< " Free heap " << Platform::GetHeapState().BytesFree 
			<< " (" << Utils::IntToStringSign(diff) << ")" << iLogW::EndRecord;

		if (m_body != 0) m_body->LogState();
	}

	// ----------------------------------

	void E1Application::PollForAppExit()
	{
		if (!m_param.Config.AppExitRequest()) return;

		// already done
		if (m_breakFlag.Get()) return;

		// exit
		if(m_logSession->LogActive()) *m_logSession << m_tagInfo << "Exit!" << iLogW::EndRecord;
		AsyncSetReadyForDeleteFlag(m_breakFlag);
	}

    // ------------------------------------------------------------------

    void E1Application::LedsUpdate()
    {
        if (CDisableLeds) return;

        Platform::dword t = Platform::GetSystemTickCount();
        E1AppBody *pBody = m_body.get();
        m_ledSystem.Update(t, pBody);
        m_ledCalls.Update(t, pBody);
    }

	// ------------------------------------------------------------------

    void E1Application::ProcessTimerEvent()
    {
        m_timerCycle++;

        // watchdog
        if (m_watchdog)
        {
            m_watchdog->Reset();
        }

        // leds
        LedsUpdate();

        // network poll
        {
            bool curr = true; // Stack::Instance().IsEstablished();  // LwIP race condition bug fix

            if (curr != m_established)
            {
                m_established = curr;

                if (curr) OnPhConnect();
                else OnPhDisconnect();
            }
        }

        // SBP (sockets) poll
        if (m_serverPoll.TimeOut())
        {
            if (m_established) m_tcpServer->Process(); // incoming connections

            int interval = (m_body == 0) ? CServerPollTimeActive : CServerPollTimePassive;
            m_serverPoll.Set(interval, false);
        }

        m_connections.Process();  // recv packets

        // Body poll
        if (m_body != 0) m_body->Process();

        // DataTime capture poll
        {
            Utils::MsDateTime dt;
            dt.Capture();           // TODO -- do not on every iteration
        }

        // CPU usage
        if (m_cpuUsagePrintTimer.TimeOut()) LogState();    

        // Echo
        m_echo->Process();

        // exit req
        PollForAppExit();
    }

    // ------------------------------------------------------------------

	void E1Application::StartNetworkActivity()
	{
		if(m_tcpServer == 0)
		{

			SbpTcpServer::Profile profile;
			profile.m_tcpPort = m_cmpPort;
			profile.m_user = this;
			// profile.sbpSettings = ...; use default constructor
			profile.m_connectionProfile.m_logCreator = m_logSession->LogCreator();

            profile.m_connectionProfile.m_sbpSettings.setMaxSendSize(iCmp::CPacketMaxSizeBytes);

			m_tcpServer.reset(new SbpTcpServer(getMsgThread(), profile));
			m_tcpServer->LogSettings().Init(CCmpTraceMaxParams, CCmpTraceMaxBinary);				
			m_cmpPort = m_tcpServer->LocalHostInf().Port();
		}
				
		m_broadcast->Start(m_cmpPort);
		
		// log
		if (m_logSession->LogActive()) 
		{
			*m_logSession << m_tagInfo << "Started, port " << m_cmpPort
				<< ", build " << __DATE__ << " " << __TIME__ << iLogW::EndRecord;
		}
		
		
	}

	// ------------------------------------------------------------------

	void E1Application::StopNetworkActivity()
	{
		m_broadcast->Stop();
	}

	// ------------------------------------------------------------------

	void E1Application::OnTimer(iCore::MsgTimer *pT)
	{
        {
            Platform::dword t = Platform::GetSystemTickCount();
            m_stats.TimerEventInterval.Add( t - m_timerLastCallTicks );
            m_timerLastCallTicks = t;
        }

        {
            m_perfTimer.Reset();

            ProcessTimerEvent();

            m_stats.TimerEventTime.Add( m_perfTimer.GetCounter() );
        }
	}

	// ----------------------------------

    template<class TBody, class TBodyParams>
    bool E1Application::CreateAppBody( Utils::SafeRef<SBProtoExt::ISbpConnection> src, const TBodyParams &data )
    {
        if (m_body != 0)
        {
            std::string info = m_body->ActiveConnectionInfo();
            info = "Body already created by " + info;

            iCmp::BfResponse::Send(src->Proto(), TdmMng::terAlreadyBusy, info);

            ProtoError(src, info);

            return false;
        }

        int bodySize = 0;
        m_heapOnCreateBody = Platform::GetHeapState().BytesFree;

        // try to set date/time (it's works on Bf only)
        if (!data.Date.IsNull() && !data.Time.IsNull())
        {
            Utils::DateTime dt(data.Date, data.Time);
            Utils::MsDateTime::SetSyncPoint(dt);
            if (m_logSession->LogActive()) *m_logSession << m_tagInfo << "Date/time set!" << iLogW::EndRecord;
        }

        // create
        E1BodyParams params(getMsgThread(), m_param.Config, m_logSession->LogCreator(), 
            *m_echo, src);

        m_body.reset( new TBody(params, data) );

        bodySize = m_heapOnCreateBody - Platform::GetHeapState().BytesFree;

        // stats
        m_stats.TotalSessions++;
        m_stats.CmpPackInSession = 0;
        m_stats.StartSessionTimeTicks = Platform::GetSystemTickCount();

        if (m_logSession->LogActive())
        {
            *m_logSession << m_tagInfo << "Init done, body size " << bodySize 
                << "; type " << typeid(TBody).name() << iLogW::EndRecord;
        }

        m_broadcast->UpdateOwnerInfo( src->TransportInfo() );

        // send resp
        m_body->SendInitResp();

        return true;
    }

	// ----------------------------------

    void E1Application::OnPcCmdInitE1( SafeRef<ISbpConnection> src, const iCmp::BfInitDataE1 &data )
    {
		if (!data.IsValid()) ESS_THROW_T(TdmMng::TdmException, TdmMng::terBfInitDataE1);

        CreateAppBody<E1AppBodyE1>(src, data);
    }

    // ----------------------------------

    void E1Application::OnPcCmdInitAoz( SafeRef<ISbpConnection> src, const iCmp::BfInitDataAoz &data )
    {
		if (!data.IsValid()) ESS_THROW_T(TdmMng::TdmException, TdmMng::terBfInitDataAoz);

        E1AppBody::InitAozSync();

        CreateAppBody<E1AppBodyAoz>(src, data);
    }

    // ----------------------------------

    void E1Application::OnPcCmdInitPult( SafeRef<ISbpConnection> src, const iCmp::BfInitDataPult &data )
    {
        if (!data.IsValid()) ESS_THROW_T(TdmMng::TdmException, TdmMng::terBfInitDataPult);

        {
            using PsbCofidec::PsbCofidecInit;

            if ( !PsbCofidecInit::InitDone() )
            {
                PsbCofidecInit::InitSync();
            }
        }

        CreateAppBody<E1AppBodyPult>(src, data);
    }

    // ----------------------------------

    void E1Application::OnPcCmdGlobalSetup(SafeRef<ISbpConnection> src, const iCmp::BfGlobalSetup &data)
    {
        // the very special case!
        if (data.DoEchoHalt)
        {
            iCmp::BfResponse::Send(src->Proto(), TdmMng::terOK, "OK!");  // OK
            m_echo->CmdHalt();
            return;
        }

        // verify access 
        if (m_body != 0)
        {
            if ( !m_body->IsActiveConnection(src) ) 
            {
                ESS_THROW_T(TdmMng::TdmException, TdmMng::terAlreadyBusy);
            }
        }

        m_asyncBodyDestroy = data.AsyncDeleteBody;
        m_asyncDropConnOnException = data.AsyncDropConnOnException;

        // setup log
        LogSetup(data.LogEnable, data.UdpHostPort);

        // safe ref test
        if (data.DoSafeRefTest) SafeRefTest();

        // reset echo
        if (data.SetupNullEcho)
        {
            if (m_body != 0)
            {
                ESS_THROW_T(TdmMng::TdmException, TdmMng::terNullEchoWithBody);
            }

            m_echo.reset( new NullEchoClient() );
        }

        // setup watchdog
        m_watchdog.reset();
        if (data.WatchdogTimeoutMs > 0) 
        {
            m_watchdog.reset( new BfDev::BfWatchdog(data.WatchdogTimeoutMs) );
        }

        // setup halt-reboot
        AppPlatformHookSetup(true, true, data.ResetOnFatalError);

        // send OK
        iCmp::BfResponse::Send(src->Proto(), TdmMng::terOK, "OK!");

        // test actions
        if (data.DoAssert) ESS_ASSERT(0 && "Test assertion!");
        if (data.DoLoopForever) { while(true); }
        if (data.DoOutOfMemory)
        {            
			Utils::MemLeak::LeakForever();
        }
        
    }

    // ----------------------------------

    void E1Application::LogSetup( bool logEnable, const string &udpHostPort )
    {
        // disable trace
        m_log.Output().TraceToUdp(false);

        // bind UDP output
        if (logEnable)
        {

            Utils::HostInf intf;
            if (!Utils::HostInf::FromString(udpHostPort, intf))
            {
                ESS_THROW_T(TdmMng::TdmException, TdmMng::terBadLogHostPort);
            }
            m_log.Output().DstUdpTrace(intf);
            m_log.Output().TraceToUdp(true);
        }

        // do it: disable all sessions, disable CMP trace
        // this affected for body and echo (it's child sessions)
        m_logSession->LogActive(logEnable);
    }

    // ----------------------------------

    void E1Application::OnPcCmdStateInfo( SafeRef<ISbpConnection> src )
    {        
        // fill current
        m_stats.CurrTicks = Platform::GetSystemTickCount();
        m_stats.CpuFreq = BfDev::SysProperties::Instance().getFrequencyCpu();
        m_stats.SysBusFreq = BfDev::SysProperties::Instance().getFrequencySys();
        m_stats.CpuUsage = m_param.CpuThread.CpuLoad(); 
        m_stats.HeapFreeBytes = Platform::GetHeapState().BytesFree;
        m_stats.BuildInfo = m_param.Config.AppBuildInfo;

        if (m_body != 0)
        {
            m_stats.ActiveLinkInfo = m_body->ActiveConnectionInfo(); 
            m_stats.ActiveChCount = m_body->ActiveChannelsCount();
            m_stats.MacHash = m_body->ProtectionInfo();
        }
        else
        {
            m_stats.ActiveLinkInfo.clear();
            m_stats.ActiveChCount = 0;
            m_stats.MacHash.clear();
        }

        m_stats.Echo = m_echo->GetStats();

        // copy and send
        iCmp::BfStateInfo info;
        info.Stats = m_stats;
        iCmp::BfRespStateInfo::Send(src->Proto(), info);
    }

    // ----------------------------------

    void E1Application::AsyncReboot()
    {        
        m_param.Reloader.Reboot(m_breakFlag);        
    }


}  // namespace E1App
