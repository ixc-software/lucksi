
#include "stdafx.h"

#include "Platform/Platform.h"
#include "Utils/ErrorsSubsystem.h"
#include "Utils/ThreadNames.h"

#include "iLog/LogManager.h"
#include "iNet/MemberSocketThread.h"
#include "iCore/MsgProcessorProfiler.h"
#include "iCore/TimerThread.h"
#include "iNet/NetThreadMonitor.h"
#include "DriTests/DRITestHelpers.h"
#include "DRI/NObjTelnetServerDRI.h"
#include "DRI/ExecutiveDRI.h"
#include "DRI/DriSessionList.h"
#include "DriSec/NObjSecRoot.h"
#include "iReg/NObjReg.h"

#include "NObjRoot.h"
#include "DomainStartup.h"
#include "DomainClass.h"
#include "NObjLogManager.h"
#include "NObjThreadMonitor.h" 

// ----------------------------------------------------------
// RootNamedObject impl

namespace Domain
{
    NObjRoot::NObjRoot(IDomain *pDomain, 
                       iLogW::LogManager &log,
                       const Domain::ObjectName &name,
                       const std::string &buildInfo) : 
        NamedObject(pDomain, name, 0, true), 
        m_pSecurity( new DriSec::NObjSecRoot(this) ),
        m_pLog( new NObjLogManager(this, "Log") ),
        m_pTelnet( new DRI::NObjTelnetServerDRI(this) ),
        m_pMonitor( new NObjThreadMonitor(this, "ThreadMonitor") ),
        m_pReg( new iReg::NObjReg(this) ),
        m_buildInfo(buildInfo),
        m_startDateTime( QDateTime::currentDateTime().toString("dd.MM.yyyy hh:mm:ss") ),
        m_netMonStarted(false),
        m_haltOnQtWarning(true)
    {
        SetupMonitor();
        
        AllowUserChildrenAdd(true);

        pDomain->getDomain().ExeDRI().SpecClassFactory()
                            .RegisterAlwaysShowWithInherited(metaObject()->className());
    }

    // ----------------------------------------------------------

    NObjRoot::~NObjRoot()
    {
        // kill all children before fields destroy
        // FreeChildren(3);  // dirty hack 
    }

    // ----------------------------------------------------------

    iCore::MsgThreadMonitor& NObjRoot::ThreadMonitor()
    {
        ESS_ASSERT(m_pMonitor != 0);

        return m_pMonitor->Monitor();
    }

    // ----------------------------------------------------------
    
    void NObjRoot::SyncStop(Utils::AtomicBool &flag)
    {
        // try stop all async commands
        // m_telnet.CloseAllSessions();
        if (m_pTelnet)
        {
            delete m_pTelnet;
            m_pTelnet = 0;
        }
        
        MsgObject::AsyncSetReadyForDeleteFlag(flag);
    }

    // ----------------------------------------------------------

    void NObjRoot::SetupMonitor()
    {
        ESS_ASSERT(m_pMonitor != 0);

        m_pMonitor->SetMaxMsgQueueSize(4 * 1024);
        m_pMonitor->SetMaxTimerLag(10, 1000);
        m_pMonitor->SetMsgGrowDetection(10, 1024);
    }

    // ----------------------------------------------------------

    /*
    QString NObjRoot::TelenetSessionInfo(const QString &name) const // CS_Info
    {
        return m_telnet.FindSessionInfo(name);
    }
    
    void NObjRoot::CloseTelnetSession(const QString &name) // CS_Exit
    {
        m_telnet.CloseSession(name);
    } */

    // ----------------------------------------------------------

    Utils::HostInf NObjRoot::LocalHostInf() const
    {
        ESS_ASSERT(m_pTelnet != 0);
        return m_pTelnet->LocalHostInf();
    }

    // ----------------------------------------------------------

    void NObjRoot::TestAssertion()
    {
        ESS_HALT("Test assertion");
    }

    // ----------------------------------------------------------

    void NObjRoot::TestStackTrace(DRI::ICmdOutput *pCmd)
    {
        std::string trace;
        Platform::GetStackTrace(trace);
        pCmd->Add(trace.c_str());
    }

    // ----------------------------------------------------------

    void NObjRoot::TestAV()
    {
        static volatile int *p = 0;
        volatile int result = *p;
    }

    // ----------------------------------------------------------

    void NObjRoot::TestAsyncException()
    {
        PutMsg(this, &NObjRoot::RaiseException);
    }

    // ----------------------------------------------------------

    void NObjRoot::TestMsgOverload()
    {
        int waitMcs = 50 + m_random.Next(5000);

        Platform::ddword t = Platform::GetSystemMicrosecTickCount();
        while(Platform::GetSystemMicrosecTickCount() - t < waitMcs);

        for(int i = 0; i < 16; ++i)
        {
            PutMsg(this, &NObjRoot::TestMsgOverload);
        }

    }

	// ----------------------------------------------------------

	QString NObjRoot::NetMonitorStatus() const
	{
		if(!m_netMonStarted) return "Inactive.";
	
		return QString("Active. Output file: ") + 
			iNet::NetThreadMonitor::OutputFileName(); 
	}

    // ----------------------------------------------------------

    ESS_TYPEDEF(NObjRootTestException);
    ESS_TYPEDEF(NObjRootBadTimeInterval);
    
    void NObjRoot::RaiseException()
    {
        ESS_THROW(NObjRootTestException);
    }

    // ----------------------------------------------------------

    void NObjRoot::SetMsgProfiling( int depth )
    {
        iCore::MsgProcessorProfiler *p = 0;
        if (depth > 0) p = new iCore::MsgProcessorProfiler(depth);
        getMsgThread().SetMsgProfiler(p);
    }

    // ----------------------------------------------------------

    void NObjRoot::PrintMsgProfiling( DRI::ICmdOutput *pCmd )
    {
        iCore::IMsgProcessorProfiler *p = getMsgThread().MsgProfiler();

        if (p == 0)
        {
            pCmd->Add("<message profiling is off>");
            return;
        }

        std::string info;
        p->GetDebugInfo(info);
        pCmd->Add( info.c_str() );
    }

    void NObjRoot::SetMaxMessageTime( int intervalMs )
    {
        if (intervalMs < 0) ESS_THROW(NObjRootBadTimeInterval);
        getMsgThread().SetMaxMessageProcessingTimeMs(intervalMs);
    }

    void NObjRoot::TestMsgWatchdog()
    {
        while(true);
    }

	// ----------------------------------------------------------

	void NObjRoot::StartNetMonitor(int intervalMs)
	{
		if(m_netMonStarted) ThrowRuntimeException("Object is activated.");

		if(intervalMs == 0) ThrowRuntimeException("Timeout is zero.");
		m_netMonStarted = true;
		iNet::MemberSocketThread::StartNetThreadMon(intervalMs);
	}
	
	void NObjRoot::StopNetMonitor()
	{
		if(!m_netMonStarted) ThrowRuntimeException("Object isn't activated.");
		m_netMonStarted = false;
		iNet::MemberSocketThread::StopNetThreadMon();
	}

    // ----------------------------------------------------------

    void NObjRoot::TestAsyncCmd( DRI::IAsyncCmd* pAsyncCmd, bool canAborted, int timeOut)
    {
        // if (m_asyncCmdTestTimer != 0) ThrowRuntimeException("Timer already started!"); 

        boost::function<void ()> abortFn;
        if (canAborted) 
        {
            abortFn = boost::bind(&NObjRoot::TestAsyncCmdCompleted, this);
        }

        AsyncBegin(pAsyncCmd, abortFn);

        if (timeOut <= 0) return;
        
        m_asyncCmdTestTimer.reset(
            new iCore::MsgTimer(this, &NObjRoot::TestAsyncCmdCompletedTimer) 
            );
        m_asyncCmdTestTimer->Start(timeOut);
    }

	// ----------------------------------------------------------

	void NObjRoot::Sleep(DRI::IAsyncCmd* pAsyncCmd, int timeOut)
	{
		TestAsyncCmd(pAsyncCmd, false, timeOut);
	}

    // ----------------------------------------------------------

    void NObjRoot::TestAsyncCmdCompleted()
    {
        if (!AsyncActive()) return;

        if (m_asyncCmdTestTimer) m_asyncCmdTestTimer->Stop();
            
        AsyncOutput("Command completed!");
        AsyncComplete(true);
    }


    void NObjRoot::TestAsyncCmdCompletedTimer( iCore::MsgTimer* )
    {
        TestAsyncCmdCompleted();
    }

    // ----------------------------------------------------------

    QString NObjRoot::getSessionsInfo()
    {
        return SessionList().GetBriefInfo();
    }

    // ----------------------------------------------------------

    void NObjRoot::SessionsList( DRI::ICmdOutput *pCmd )
    {
        QString s = SessionList().GetDetailedInfo();
        pCmd->Add(s);
    }

    // ----------------------------------------------------------

    bool NObjRoot::getTraceSessions() 
    {
        return SessionList().Log().LogActive();
    }

    void NObjRoot::setTraceSessions(bool value)
    {
        SessionList().Log().LogActive(value);        
    }

    // ----------------------------------------------------------

    DRI::DriSessionList& NObjRoot::SessionList() 
    {
        return getDomain().ExeDRI().SessionList();
    }

    // ----------------------------------------------------------

    void NObjRoot::DoDomainExit()
    {
        getDomain().Stop(Domain::DomainExitEsc);
    }

    // ----------------------------------------------------------

    void NObjRoot::RunTestScript(DRI::ICmdOutput *pCmd, QString scpName, QString refOutput)
    {
        const bool CSilentMode = true;
        QString dirs = getDomain().Params().IncludeDirList().ToString();

        QString errMsg;
        DriTests::DriTestsParams p(CSilentMode, dirs, 16);
        DriTestHelpers::RunScript(p, scpName, refOutput, QStringList(), &errMsg);

        if (errMsg.isEmpty()) return;  // OK

        pCmd->Add(errMsg);
        ThrowRuntimeException("ERROR!");
    }

    // ----------------------------------------------------------

    void NObjRoot::TestQtWarningMsg()
    {
        QObject obj;
        obj.startTimer(-1);  // <-- warning: bad param 
    }

    // ----------------------------------------------------------

    void NObjRoot::ListThreads( DRI::ICmdOutput *pCmd )
    {
        std::string s = Utils::ThreadNames::Instance().Info();
        pCmd->Add( s.c_str() );
    }

    // ----------------------------------------------------------

    iReg::ISysReg& NObjRoot::SysReg()
    {
        ESS_ASSERT(m_pReg != 0);

        return *m_pReg;
    }

} // namespace Domain

