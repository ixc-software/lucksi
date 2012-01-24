
#pragma once

#include "Utils/AtomicTypes.h"
#include "Utils/HostInf.h"
#include "Utils/Random.h"

#include "iCore/MsgThreadMonitor.h"

#include "DRI/ITelnetSessionManagerToOwner.h"
#include "DRI/NObjTelnetServerDRI.h"
#include "DRI/INonCreatable.h"


#include "NamedObject.h"
#include "IDomain.h"
#include "NObjThreadMonitor.h"
#include "NObjLogManager.h"

namespace iLogW
{
    class LogManager;    
}  

namespace iCore
{
    class MsgThreadMonitor;
} 

namespace DRI
{
    class DriSessionList;
}

namespace DriSec
{
    class NObjSecRoot;
}

namespace iReg
{
    class NObjReg;
    class ISysReg;
}


namespace Domain
{
    class DomainStartup;
    
    // ------------------------------------------------------

    // Main object in system - parent for all other created objects.
    class NObjRoot : 
        public NamedObject,
        public DRI::INonCreatable
    {
        Q_OBJECT;

        // children; view ClearAllChildren()
        DriSec::NObjSecRoot *m_pSecurity;
        NObjLogManager *m_pLog;
        DRI::NObjTelnetServerDRI *m_pTelnet;
        NObjThreadMonitor *m_pMonitor;
        iReg::NObjReg *m_pReg;

        // misc
        std::string m_buildInfo;
        QString m_startDateTime;
        Utils::Random m_random;
        bool m_netMonStarted;
        bool m_haltOnQtWarning;

        boost::scoped_ptr<iCore::MsgTimer> m_asyncCmdTestTimer;

        void SetupMonitor();

		QString NetMonitorStatus() const;
        void RaiseException();

        bool getHeapTrace() { return getDomain().TraceHeap(); }
        void setHeapTrace(bool val) { return getDomain().TraceHeap(val); }

        QString getHeapInfo() { return getDomain().NamedObjectHeapInfo(); }
        QString getSessionsInfo();

        bool ForceInheritedInfo() const { return true; } // override

        bool getExitOnLastSessionDrop() const 
        { 
            ESS_ASSERT(m_pTelnet != 0);
            return m_pTelnet->CloseDomainIfNoSessions(); 
        }

        void setExitOnLastSessionDrop(bool val) 
        { 
            ESS_ASSERT(m_pTelnet != 0);
            m_pTelnet->CloseDomainIfNoSessions(val); 
        }

        void TestAsyncCmdCompletedTimer(iCore::MsgTimer*);
        void TestAsyncCmdCompleted();

        bool getTraceSessions();
        void setTraceSessions(bool value);

        DRI::DriSessionList& SessionList();

        void DoDomainExit();

    // DRI interface
    public:

        Q_PROPERTY(QString BuildInfo READ BuildInfo);
        Q_PROPERTY(QString Started   READ m_startDateTime);
        Q_PROPERTY(bool HeapTrace READ getHeapTrace WRITE setHeapTrace);
        Q_PROPERTY(QString HeapInfo READ getHeapInfo);
        Q_PROPERTY(QString Sessions READ getSessionsInfo);
        Q_PROPERTY(QString NetMonitorStatus READ NetMonitorStatus);
        Q_PROPERTY(bool ExitOnLastSessionDrop READ getExitOnLastSessionDrop WRITE setExitOnLastSessionDrop);
        Q_PROPERTY(bool TraceSessions READ getTraceSessions WRITE setTraceSessions);

        Q_INVOKABLE void TestStackTrace(DRI::ICmdOutput *pCmd);
        Q_INVOKABLE void TestAssertion();
        Q_INVOKABLE void TestException() { RaiseException(); }
        Q_INVOKABLE void TestAsyncException();
        Q_INVOKABLE void TestAV();
        Q_INVOKABLE void TestMsgOverload();
        Q_INVOKABLE void TestAsyncCmd(DRI::IAsyncCmd* pAsyncCmd, bool canAborted = false, int timeOut = 0);
        Q_INVOKABLE void TestQtWarningMsg();
        Q_INVOKABLE void Sleep(DRI::IAsyncCmd* pAsyncCmd, int timeOut);

        Q_INVOKABLE void SetMsgProfiling(int depth);
        Q_INVOKABLE void PrintMsgProfiling(DRI::ICmdOutput *pCmd);

        Q_INVOKABLE void SetMaxMessageTime(int intervalMs);
        Q_INVOKABLE void TestMsgWatchdog();

		Q_INVOKABLE void StartNetMonitor(int intervalMs);
		Q_INVOKABLE void StopNetMonitor();

        Q_INVOKABLE void SessionsList(DRI::ICmdOutput *pCmd);

        Q_INVOKABLE void HaltOnQtWarning(bool val) { m_haltOnQtWarning = val; }

        Q_INVOKABLE void RunTestScript(DRI::ICmdOutput *pCmd, 
            QString scpName, QString refOutput = "");

        Q_INVOKABLE void DomainExit()
        {
            PutMsg(this, &NObjRoot::DoDomainExit);
        }

        Q_INVOKABLE void ListThreads(DRI::ICmdOutput *pCmd);

    public:

        NObjRoot(IDomain *pDomain, 
                 iLogW::LogManager &log,
                 const Domain::ObjectName &name = "",
                 const std::string &buildInfo = "");

        ~NObjRoot();

        iCore::MsgThreadMonitor& ThreadMonitor();
        Utils::HostInf LocalHostInf() const;
            
        QString BuildInfo() const { return m_buildInfo.c_str(); }
                    
        void SyncStop(Utils::AtomicBool &flag);

        DriSec::NObjSecRoot& Security()
        {
            ESS_ASSERT(m_pSecurity != 0);
            return *m_pSecurity;
        }

        iReg::ISysReg& SysReg();

        bool HaltOnQtWarning() const { return m_haltOnQtWarning; }

        void ClearAllChildren()
        {
            m_pSecurity = 0;
            m_pLog = 0;
            m_pTelnet = 0;
            m_pMonitor = 0;
            
			FreeChildren();
			
			// m_pReg нельзя занулять пока не удалены дочерние обьекты
			// может быть обращение к m_pReg из деструкторов NamedObject
			m_pReg = 0;
        }

        /*
        QString TelenetSessionInfo(const QString &name) const; // CS_Info
        void CloseTelnetSession(const QString &name); // CS_Exit
        */
    };
    
} // namespace Domain


