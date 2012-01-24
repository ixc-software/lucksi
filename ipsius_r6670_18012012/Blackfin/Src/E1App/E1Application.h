#ifndef _E1_APPLICATION_H_
#define _E1_APPLICATION_H_

#include "stdafx.h"

#include "Platform/Platform.h"
#include "iCore/MsgObject.h"
#include "iCore/ThreadRunner.h"
#include "iCore/MsgTimer.h"
#include "Utils/ManagedList.h"
#include "iLog/LogManager.h"
#include "SafeBiProtoExt/SpbTcpServer.h"
#include "SafeBiProtoExt/SbpConnectionList.h"
#include "iCmp/ChMngProto.h"
#include "BfDev/BfTimerCounter.h"
#include "BfBootSrv/BroadcastAutoSender.h"

#include "E1AppConfig.h"
#include "AppCpuUsage.h"
#include "E1ApplicationStats.h"
#include "ICloseApp.h"
#include "IEchoClient.h"
#include "AppLed.h"
#include "EchoSfxClient.h"



namespace iCore	{	class IThreadRunner; };
namespace BfBootSrv	
{ 
	class BoardSetup;
	
	class BroadcastAutoSender;
};

namespace BfDev
{
    class BfWatchdog;
}

namespace E1App
{
    using Utils::SafeRef;
    using SBProtoExt::ISbpConnection; 
    using Platform::word;

	struct E1ApplicationParams
	{
		AppCpuUsage &CpuThread;
		const E1App::E1AppConfig &Config;        
        ICloseApp& Reloader;
		//const BfBootSrv::BoardSetup &BoardConfig;          

        // from BfBootSrv::BoardSetup
        BfBootCore::BroadcastMsg BroadcastMessage;
        word CmpPort;

		E1ApplicationParams(
            const E1AppConfig &config, ICloseApp& reloader,
            AppCpuUsage &cpuThread, const BfBootCore::BroadcastMsg& msg, word cmpPort) : 
			CpuThread(cpuThread), Config(config), Reloader(reloader), BroadcastMessage(msg), CmpPort(cmpPort) 
        {}
	};

	class E1AppBody;
	class E1AppConnection;   

	class E1Application : boost::noncopyable,
		public iCore::MsgObject,
        public BfBootSrv::ICloseApp,
		public SBProtoExt::ISbpTcpServerEvents,
		public SBProtoExt::ISbpConnectionEvents,
        iCmp::IBfSideRecvGlobal
	{ 
		typedef E1Application T;
	public:
		E1Application(iCore::IThreadRunner &runner, E1ApplicationParams &param);        

    // iCmp::IBfSideRecvGlobal impl
    private:

        void OnPcCmdInitE1(SafeRef<ISbpConnection> src, const iCmp::BfInitDataE1 &data);
        void OnPcCmdInitAoz(SafeRef<ISbpConnection> src, const iCmp::BfInitDataAoz &data);
        void OnPcCmdInitPult(SafeRef<ISbpConnection> src, const iCmp::BfInitDataPult &data);
        void OnPcCmdGlobalSetup(SafeRef<ISbpConnection> src, const iCmp::BfGlobalSetup &data);
        void OnPcCmdStateInfo(SafeRef<ISbpConnection> src);

	//	ISbpTcpServerEvents
	private:
		void NewConnection(const SBProtoExt::SbpTcpServer*, 
			boost::shared_ptr<SBProtoExt::ISbpConnection>);
		void Error(const SBProtoExt::SbpTcpServer *, const std::string &);
	
	//	SBProto::ISbpConnectionEvents
	private:
		void CommandReceived(Utils::SafeRef<SBProtoExt::ISbpConnection> src,
			boost::shared_ptr<SBProto::SbpRecvPack> data);
		void ResponseReceived(Utils::SafeRef<SBProtoExt::ISbpConnection> src,
			boost::shared_ptr<SBProto::SbpRecvPack> data);
		void InfoReceived(Utils::SafeRef<SBProtoExt::ISbpConnection> src,
			boost::shared_ptr<SBProto::SbpRecvPack> data);
		void ProtocolError(Utils::SafeRef<SBProtoExt::ISbpConnection> src,
			boost::shared_ptr<SBProto::SbpError> err);
		void ConnectionActivated(Utils::SafeRef<SBProtoExt::ISbpConnection> src);
		void ConnectionDeactivated(Utils::SafeRef<SBProtoExt::ISbpConnection> src,
			const std::string &);

    // BfBootSrv::ICloseApp
    private:
        void AsyncReboot();

	private:

        void LogSetup(bool logEnable, const string &udpHostPort);

		void Log(iLogW::LogRecordTag tag, const std::string &s);
		void OnPhConnect();
		void OnPhDisconnect();
		void DestroyBodyRequest();
        void DestroyBodyImpl();
		// log and disconnect (msg not ref for iCore msg call)
		void ProtoError(Utils::SafeRef<SBProtoExt::ISbpConnection> src,
			const std::string &msg);		
		void LogState() const;
		void PollForAppExit();
		void OnTimer(iCore::MsgTimer *pT);
		void DebugCreateBody();
		bool IsActiveConnection(Utils::SafeRef<SBProtoExt::ISbpConnection> src) const;
        void ProcessCmpPacket(Utils::SafeRef<ISbpConnection> src, 
            boost::shared_ptr<SBProto::SbpRecvPack> data);
        void ProcessTimerEvent();
		void StartNetworkActivity();
		void StopNetworkActivity();
        void LedsUpdate();

        template<class TBody, class TBodyParams>
        bool CreateAppBody(Utils::SafeRef<SBProtoExt::ISbpConnection> src, const TBodyParams &data);

	private:

        BfDev::BfTimerCounter m_perfTimer;
        E1ApplicationStats m_stats;
		E1ApplicationParams m_param;
		iLogW::LogManager m_log;
		scoped_ptr<iLogW::LogSession> m_logSession;
		iLogW::LogRecordTag m_tagInfo;
		iLogW::LogRecordTag m_tagError;
		int m_cmpPort;
		boost::scoped_ptr<SBProtoExt::SbpTcpServer> m_tcpServer;
		boost::scoped_ptr<BfBootSrv::BroadcastAutoSender> m_broadcast;
		int m_timerCycle;
		iCore::MsgTimer m_timer;
        Platform::dword m_timerLastCallTicks;
		bool m_established;  // Ethernet ph level
		int  m_heapOnCreateBody;
		mutable int  m_prevFreeHeapValue;
		Utils::AtomicBool &m_breakFlag;
        bool m_asyncBodyDestroy;
        bool m_asyncDropConnOnException;
        boost::scoped_ptr<BfDev::BfWatchdog> m_watchdog;

        LedE1 m_ledSystem;
        LedCalls m_ledCalls;

		Utils::TimerTicks m_cpuUsagePrintTimer;
        Utils::TimerTicks m_serverPoll;
		SBProtoExt::SbpConnectionList m_connections;

        boost::scoped_ptr<IEchoClient> m_echo;  // it can't be null

		// create after SBP Init command, destroy on connection drop
		boost::scoped_ptr<E1AppBody> m_body;		
		
	};

};  // namespace E1App

#endif


