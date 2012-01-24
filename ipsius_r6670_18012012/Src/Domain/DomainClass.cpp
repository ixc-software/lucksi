
#include "stdafx.h"

#include "Platform/Platform.h"

#include "Utils/ErrorsSubsystem.h"
#include "Utils/QtHelpers.h"

#include "iCore/MsgObject.h"
#include "iCore/MsgThreadFn.h"
#include "iLog/LogWrapperInterfaces.h"

#include "DRI/ExecutiveDRI.h"
#include "DRI/DriSessionList.h"
#include "DriSec/NObjSecRoot.h"

#include "NObjRoot.h"
#include "NObjDomainAutorun.h"
#include "NObjLogManager.h"
#include "DomainCoutFormatter.h"
#include "DomainClass.h"


namespace
{
    const int CDomainStopKey = 27; // esc
    const Platform::dword CCheckKeyIntervalMs = 200;

    QString GetHeapInfo(size_t *pAllocated = 0)
    {
        Platform::HeapState hs = Platform::GetHeapState();

        if (pAllocated != 0) *pAllocated = hs.BytesAllocated;

        if (!hs.Availble()) return "Heap: <no info>";

        return QString("Heap: blocks %1, allocated %2")
            .arg(Utils::NumberFormat(hs.BlockCount))
            .arg(Utils::NumberFormat(hs.BytesAllocated));
    }    

} // namespace 

// ----------------------------------------

namespace Domain
{
    // Handle run-time pressed keys (for now just ESC)
    class DomainClass::DomaintScanKey : public iCore::MsgObject
    {
        DomainClass &m_owner;
        iCore::MsgTimer m_timer;

        void OnTimer(iCore::MsgTimer *pT)
        {
            if ((CDomainStopKey == Platform::KeyWasPressed()) 
                && (!m_owner.Params().DisableExitByKey()))
            {
                m_timer.Stop(); 
                m_owner.Stop(DomainExitEsc); 
            }
        }

    public:
        DomaintScanKey(DomainClass &owner) : 
            iCore::MsgObject(owner.getMsgThread()), 
            m_owner(owner), m_timer(this, &DomaintScanKey::OnTimer)
        {
            m_timer.Start(CCheckKeyIntervalMs, true);
        }
    };

    // ------------------------------------------------------

    DomainClass::DomainClass( iCore::IThreadRunner &runner, DomainStartup &params ) : 
        iCore::MsgObject(runner.getThread()),
        m_handler( boost::bind(&DomainClass::QtHandle, this, _1, _2, _3) ),
        m_runner(runner), 
        m_params(params),
        m_name(params.Name()),
        m_keyScanner(new DomaintScanKey(*this)),
        m_coutFormatter(new DomainCoutFormatter(""/*m_name.Name()*/, false)),
        m_traceHeap(false),
        m_namedObjectsDestroyed(0),
        m_prevHeapAllocated(0)
    {
        InitLog();

        m_exeDRI.reset(new DRI::ExecutiveDRI(*this));

        try
        {
            m_root.reset( new NObjRoot(this, Log(), "", params.BuildInfo()) );
        }
        catch (DRI::NObjTelnetServerDRI::SocketError &e)
        {
            QString msg = QString("Domain %1 startup error: %2.")
                .arg(m_name.Name()).arg(e.getTextMessage().c_str());
            Platform::ExitWithMessage(msg.toStdString().c_str());
        }

        SetupExeDRI();

        VerifyIncludeDirList();

        params.SetThreadEventsHandler(*this);

        Start();
    }


    // ---------------------------------------------

    DomainClass::~DomainClass()
    {
        // autorun
        if (m_autorun != 0)  // hmmm, it's bad
        {
            LogDomainOutput( QString("Warning: autorun live in ~DomainClass()!") );
        }
        m_autorun.reset();

        m_innerCreate.reset();                          // 'couse ~root can do callbacks to fields of DomainClass        
        
        // TelnetServer must be killed here, so all session must be 'lost'
        // We must kill session for free links to security objects
        m_exeDRI->SessionList().Shutdown();             // remove all sessions

        // kill root (all NObjects); two stages for correct callback usege of m_root
        m_root->ClearAllChildren();                     // log, telnet, security
        m_root.reset();                                 

        // kill DRI
        m_exeDRI.reset();                               // dri sessions 

        // heap info
        LogDomainOutput( QString("Domain stoped. %1").arg(GetHeapInfo()) );

        // log
        m_logSession.reset();
        m_log.reset();
    }


    // ----------------------------------------
    
    /*class DomainClass::DomainCoutFormatter : public iLogW::ILogRecordFormater
    {

    // ILogRecordFormater impl
    public:

        void FormatRecord(iLogCommon::LogRecord &record, iLogCommon::LogString &result)
        {
            using Utils::PadLeft;
            using iLogCommon::LogStringConvert;
            using iLogCommon::LogStringStream;
            using iLogCommon::LogString;

            LogString time;
            if ( !record.Timestamp().IsEmpty() ) 
            {
                Utils::DateTime dt = record.Timestamp().get();
                time = LogStringConvert::To( dt.time().ToString() );
            }

            LogString data = record.Data();
            // LogString recordKind = record.RecordKind();

            // QString("%1 %2").arg(time, -10).arg(data)

            LogStringStream os;
            if (!time.empty()) os << "[" << PadLeft(time, 10, ' ') << "] ";
            // os << " [" << PadLeft(recordKind, 12, ' ') << "] ";
            os << data;

            result = os.str(); 
        }

    public:

        DomainCoutFormatter()
        {
        }

    };*/

    // ---------------------------------------------

    NamedObject* DomainClass::ObjectRoot()
    {
        ESS_ASSERT(m_root != 0);
        return m_root.get();
    }

    // ---------------------------------------------

    NObjRoot& DomainClass::ObjectRootStat()
    {
        ESS_ASSERT(m_root != 0);
        return *m_root;
    }

    // ---------------------------------------------

    void DomainClass::Stop(DomainExitCode code)
    {
        m_autorun.reset();

        m_params.ExitCode(code);
        m_root->SyncStop(m_runner.getCompletedFlag());
    }

    // ---------------------------------------------

    /*void DomainClass::SetupThreadMonitor()
    {
        m_monitor.SetMaxMsgQueueSize(4 * 1024);
        m_monitor.SetMaxTimerLag(10, 1000);
        m_monitor.SetMsgGrowDetection(10, 1024);
    }*/


    // ---------------------------------------------

    void DomainClass::InitLog()
    {
        // Create log manager
        m_log.reset(new iLogW::LogManager(m_params.LogSettings()));

        /*
        -- > this setting in m_params.LogSettings()
        if (m_params.RemoteLogAddr().IsValid()) 
        { 
            m_log->Output().DstUdpTrace(m_params.RemoteLogAddr());
			m_log->Output().TraceToUdp(true);
        }*/

        // Create local log session

        bool logActive = true;

        m_logSession.reset(
            m_log->CreateSession(m_name.Name().toStdString(), logActive));
        m_tagDomain = m_logSession->RegisterRecordKindStr("Domain");
    }
    
    // ---------------------------------------------
    
    void DomainClass::SetupExeDRI()
    {
        m_exeDRI->HintDB().DoStaticInit(); 
    }

    // ---------------------------------------------

    void DomainClass::VerifyIncludeDirList()
    {
        QStringList invalidPathes;
        if (m_params.IncludeDirList().Verify(&invalidPathes)) return;

        QString warning("Warning! Pathes haven't found: ");
        warning.append(invalidPathes.join("; "));
        
        LogDomainOutput(warning);
    }

    // ---------------------------------------------

    void DomainClass::Start()
    {
		QString port = QString::number(ObjectRootStat().LocalHostInf().Port());
		QString msg = QString("%1 Domain %2 started at port %3. %4").
			arg(QDateTime::currentDateTime().toString(Qt::LocalDate)).
			arg(m_name.Name()).arg(port)
            .arg(GetHeapInfo());

		LogDomainOutput(msg);

        std::string buildInfo = m_params.BuildInfo();
        if (buildInfo.empty()) buildInfo = "<empty>";
        else buildInfo = '"' + buildInfo + '"';

        LogDomainOutput( QString("Build info: %1").arg(buildInfo.c_str()) );
        LogDomainOutput("");

        PutMsg(this, &DomainClass::OnDoInnerCreate);
        PutMsg(this, &DomainClass::OnExecuteAutorunScript);
    }
    
    // ---------------------------------------------

    void DomainClass::LogDomainOutput(const QString &text)
    {
        // to cout
        if (!m_params.SuppressVerbose())
        {
            std::cout << m_coutFormatter->Format(text) << std::endl;
        }

        // to log
        if (text.isEmpty()) return;
        if (!m_logSession->LogActive()) return;
        *m_logSession << m_tagDomain << text << iLogW::EndRecord;
    }

    // ---------------------------------------------

    void DomainClass::AutorunFinished(const QString &error)
    {
        Domain::DomainExitCode code = DomainExitOk;

        bool reqExit = false;
        m_autorun.reset();

        if ( error.length() ) 
        {
            LogAutorunOutput("\n" + error);
            code = DomainExitErr;

            if ( !Params().DisableExitOnAutorunError() ) reqExit = true;
        }

        if ( Params().ExitAfterAutoRun() ) reqExit = true;
            
        if (reqExit) Stop(code);
    }

    // ---------------------------------------------
    
    void DomainClass::LogAutorunOutput(const QString &text)
    {
        LogDomainOutput(text);
    }
    
	// ---------------------------------------------

    iLogW::LogManager &DomainClass::Log() 
    { 
        return *m_log;
    }

	// ---------------------------------------------

	Utils::SafeRef<iLogW::ILogSessionCreator> DomainClass::LogCreator()
	{
		return m_logSession->LogCreator();
	}

    // ---------------------------------------------

    void DomainClass::OnExecuteAutorunScript()
    {
        if (m_params.ScriptFileName().isEmpty()) return;

        const bool stopOnFirstError = true;

        m_autorun.reset(
            new NObjDomainAutorun(ObjectRoot(), "Autorun", this, stopOnFirstError));
    }

    // ---------------------------------------------

    void DomainClass::OnDoInnerCreate()
    {
        ESS_ASSERT(m_innerCreate == 0);

        IDomainInnerCreate *pInnerCreate = m_params.DomainInnerCreate();

        if (pInnerCreate) 
        {
            m_innerCreate.reset( new DomainInnerCreator(*pInnerCreate, *this));
        }
    }

    // ---------------------------------------------

    int DomainClass::FindExceptionHook(Utils::SafeRef<IDomainExceptionHook> hook)
    {
        for(int i = 0; i < m_exceptionHooks.size(); ++i)
        {
            if (m_exceptionHooks.at(i) == hook) return i;
        }

        return -1;
    }

    // ---------------------------------------------

    iCore::MsgThreadMonitor& DomainClass::ThreadMonitor()
    {
        ESS_ASSERT(m_root != 0);
        
        return m_root->ThreadMonitor();
    }

    // ---------------------------------------------

    void DomainClass::OnThreadException( iCore::IThreadRunner &sender, const std::exception *pE, bool *pSuppressIt )
    {
        ESS_ASSERT(pSuppressIt != 0);
        ESS_ASSERT(pE != 0);

        *pSuppressIt = false;

        try
        {
            for(int i = 0; i < m_exceptionHooks.size(); ++i)
            {
                if (m_exceptionHooks.at(i)->DomainExceptionSuppress(*pE))
                {
                    *pSuppressIt = true;
                    return;
                }
            }
        }
        catch(const std::exception &e)
        {
            ESS_UNEXPECTED_EXCEPTION(e);
        }
    }

    // ---------------------------------------------

    void DomainClass::OnNamedObjectDestroyed(const NamedObject &object)
    {
        ++m_namedObjectsDestroyed;

        if (!m_traceHeap) return;

        // trace info to log
        
        size_t allocated;
        QString heapInfo = GetHeapInfo(&allocated);
        int diff = (int)allocated - (int)m_prevHeapAllocated;
        if (m_prevHeapAllocated == 0) diff = 0;
        m_prevHeapAllocated = allocated;

        QString msg = QString("Destroyed %1; %2; diff %3")
            .arg(object.Name().Name())
            .arg(heapInfo)
            .arg(Utils::NumberFormat(diff));

        LogDomainOutput(msg);
    }

    // ---------------------------------------------

    QString DomainClass::NamedObjectHeapInfo() const
    {
        return QString("Objects destroyed %1; %2")
            .arg(m_namedObjectsDestroyed)
            .arg(GetHeapInfo());
    }

    // ---------------------------------------------

    DriSec::NObjSecRoot& DomainClass::Security()
    {
        return m_root->Security();
    }

    // ---------------------------------------------

    void DomainClass::QtHandle(QtMsgType type, QString typeAsString, QString msg)
    {
        if (type == QtDebugMsg) return;

        if (type == QtWarningMsg)
        {
            if (msg.indexOf("QMetaObject::invokeMethod") == 0) return;
        }

        QString text = typeAsString + ": " + msg;

        // async log
        {
            iCore::MsgThreadFn( getMsgThread(), 
                boost::bind( &DomainClass::DoLog, this, m_tagDomain, text )
            );
        }

        {
            bool halt = (m_root != 0) ? m_root->HaltOnQtWarning() : true;
            if ((type == QtWarningMsg) && !halt) return;
        }

        ESS_HALT( text.toStdString() );
    }

    // ---------------------------------------------

    bool DomainClass::Authorize( const QString login, const QString &password, 
        const Utils::HostInf &remoteAddr, QString &authError )
    {
        authError.clear();

        DriSec::NObjSecUser *pUser = Security().FindUser(login);
        if (pUser == 0)
        {
            authError = "No such user!";
            return false;
        }

        if (!pUser->VerifyPassword(password))
        {
            authError = "Bad password!";
            return false;
        }

        // verify ip
        if ( !pUser->Profile().VerifyIP(remoteAddr) )
        {
            authError = "Bad IP address";
            return false;
        }

        // verify user limit
        int currUsers = m_exeDRI->SessionList().LoggedCount(login);
        if ( !pUser->VerifyLimit(currUsers) )
        {
            authError = "User limit or login disabled!";
            return false;
        }

        return true;
    }

    // ---------------------------------------------

    void DomainClass::DoLog( iLogW::LogRecordTag tag, QString msg )
    {
        if (m_logSession && m_logSession->LogActive())
        {
            *m_logSession << tag << msg << iLogW::EndRecord;
        }       
    }

    // ---------------------------------------------

    iReg::ISysReg& DomainClass::SysReg()
    {
        return ObjectRootStat().SysReg();
    }

}  // namespace Domain


