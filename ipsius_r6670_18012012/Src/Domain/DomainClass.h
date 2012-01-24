#ifndef __DOMAINCLASS__
#define __DOMAINCLASS__

#include "stdafx.h"
#include "Utils/ErrorsSubsystem.h"
#include "Utils/IBasicInterface.h"
#include "Utils/SafeRef.h"
#include "Utils/ThreadContext.h"
#include "Utils/QtMsgHandler.h"
#include "iCore/MsgThread.h"
#include "iCore/ThreadRunner.h"
#include "iCore/MsgThreadMonitor.h"
#include "iCore/MsgTimer.h"
#include "iCore/MsgObject.h"

#include "NamedObject.h"
#include "DomainHelpers.h"
#include "IDomain.h"
#include "DomainStartup.h"
#include "DomainStore.h"
#include "IDomainAutorun.h"
#include "IDomainExceptionHook.h"

#include "iLog/LogWrapper.h"

namespace iLogW
{
    class LogManager;   
}  

namespace DRI
{
    class ExecutiveDRI;
}  

namespace DriSec
{
    class NObjSecRoot;
}

namespace iReg
{
    class ISysReg;
}


namespace Domain
{
    using Utils::SafeRef;
    using boost::shared_ptr;
    using boost::scoped_ptr;

    class NObjRoot;
    class NObjDomainAutorun;
    class DomainCoutFormatter;

    // домен
    class DomainClass : 
        // public virtual Utils::SafeRefServer,
        public iCore::MsgObject,
        public IDomain, 
        public IDomainAutorunToOwner,
        public iCore::IThreadRunnerEvents,
        boost::noncopyable
    {
        class DomaintScanKey;
        // class DomainCoutFormatter;

        Utils::QtMsgHandler m_handler;
        DomainStore m_store;
        iCore::IThreadRunner &m_runner;
        DomainStartup &m_params;
        DomainName m_name;
        Utils::ThreadContext m_threadContext;
        std::vector< Utils::SafeRef<IDomainExceptionHook> > m_exceptionHooks;

        scoped_ptr<iLogW::LogManager> m_log;
        scoped_ptr<DomainInnerCreator> m_innerCreate;
        scoped_ptr<DRI::ExecutiveDRI> m_exeDRI;
        scoped_ptr<NObjRoot> m_root;

        scoped_ptr<iLogW::LogSession> m_logSession;
        iLogW::LogRecordTag m_tagDomain;
        
        scoped_ptr<NObjDomainAutorun> m_autorun; // parent root
        scoped_ptr<DomaintScanKey> m_keyScanner;

        scoped_ptr<DomainCoutFormatter> m_coutFormatter;

        bool m_traceHeap;
        size_t m_namedObjectsDestroyed;
        size_t m_prevHeapAllocated;
        
        void InitLog();
        void SetupExeDRI();
        
        void VerifyIncludeDirList();
        void Start();
        void LogDomainOutput(const QString &text);
        
        void OnExecuteAutorunScript();
        void OnDoInnerCreate();

        int FindExceptionHook(Utils::SafeRef<IDomainExceptionHook> hook);

        // for async call 
        void DoLog(iLogW::LogRecordTag tag, QString msg);

        void QtHandle(QtMsgType type, QString typeAsString, QString msg);

    // iCore::IThreadRunnerEvents
    private:

        void OnCreateObjectFail(iCore::IThreadRunner &sender) {}
        void OnTimeout(iCore::IThreadRunner &sender) {}
        void OnThreadUnexpectedStop(iCore::IThreadRunner &sender) {}

        // this called from MsgThread context
        void OnThreadException(iCore::IThreadRunner &sender, 
            const std::exception *pE, bool *pSuppressIt);

        // this called from pool thread context
        void OnThreadPoolException(iCore::IThreadRunner &sender, 
            const std::exception *pE, bool *pSuppressIt) 
        {
            // nothing
        }
        
    // IDomainAutorunToOwner impl
    private:
        void AutorunFinished(const QString &error);
        void LogAutorunOutput(const QString &text);
        
    // IDomain impl
    public:
        DomainClass& getDomain() { return (*this); }

    public:
        DomainClass(iCore::IThreadRunner &runner, DomainStartup &params);
        ~DomainClass();

        const DomainStartup& Params() const { return m_params; }
        DomainStartup& Params() { return m_params; }

        DRI::ExecutiveDRI& ExeDRI() 
        { 
            ESS_ASSERT(m_exeDRI != 0);
            return *m_exeDRI; 
        }

        iLogW::LogManager& Log();

		Utils::SafeRef<iLogW::ILogSessionCreator> LogCreator();

        const DomainName& getName() { return m_name; }

        iCore::MsgThread& getMsgThread() { return m_runner.getThread(); }

        iCore::MsgThreadMonitor& ThreadMonitor();

        DriSec::NObjSecRoot& Security();
        bool Authorize(const QString login, const QString &password, const Utils::HostInf &remoteAddr,
            /* out */ QString &authError);

        DomainStore& Store() { return m_store; }

        // Рутовый объект, корень дерева именованных объектов
        NamedObject* ObjectRoot();
        NObjRoot& ObjectRootStat();

        iReg::ISysReg& SysReg();

        // асинхронная процедура поиска занного домена, результа поиска -- DRI
        void FindDomain(const DomainName &name, SafeRef<IDomainFindResult> callback)
        {
            // m_connectionsDRI.FindDomain(name, callback);
        }

        NamedObject* FindFromRoot(const ObjectName &name)
        {
            return ObjectRoot()->FindObject(name);
        }

        template<class TNObj>
        TNObj* FindFromRoot(QString name, QString* errDscr = 0)
        {
            Domain::NamedObject* pNObj = FindFromRoot(name);
            if (pNObj == 0) 
            {
                if (errDscr != 0) *errDscr = QString("Dri-object %1 not found").arg(name);            
                return 0;
            }

            TNObj* pTypeObj = dynamic_cast<TNObj*>(pNObj);
            if (pTypeObj == 0 && errDscr != 0) 
                *errDscr = QString("Can't type cast dri-object %1 to %2").arg(name).arg(typeid(TNObj).name());  

            return pTypeObj;
        }

        template<class TNObj>
        TNObj& FindRefFromRoot(QString name)
        {
            TNObj* pObj = FindFromRoot<TNObj>(name);
            ESS_ASSERT(pObj);
            return *pObj;
        }

        void Stop(DomainExitCode code); 

        void RegisterExceptionHook(Utils::SafeRef<IDomainExceptionHook> hook)
        {
            m_threadContext.Assert();

            // dublicates check
            ESS_ASSERT(FindExceptionHook(hook) < 0);

            m_exceptionHooks.push_back(hook);
        }

        void UnregisterExceptionHook(Utils::SafeRef<IDomainExceptionHook> hook)
        {
            m_threadContext.Assert();

            int indx = FindExceptionHook(hook);
            ESS_ASSERT(indx >= 0);

            m_exceptionHooks.erase(m_exceptionHooks.begin() + indx);
        }

    // NamedObject / heap info
    public:

        void OnNamedObjectDestroyed(const NamedObject &object); 

        QString NamedObjectHeapInfo() const;

        bool TraceHeap() const { return m_traceHeap; }
        void TraceHeap(bool val) { m_traceHeap = val; }
        
    };

}  // namespace Domain

#endif



