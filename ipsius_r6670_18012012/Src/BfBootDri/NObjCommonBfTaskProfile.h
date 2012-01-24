#ifndef NOBJGENERALTASKSETTINGS_H
#define NOBJGENERALTASKSETTINGS_H

#include "stdafx.h"
#include "Domain/NamedObject.h"
#include "Domain/DomainClass.h"
#include "BfBootCli/TransportFactory.h"
#include "BfBootCli/TaskMngProfile.h"
#include "IBroadcastReceiverReport.h"

#include "Utils/ProgressBar.h"

namespace BfBootDRI
{   
    class ITask : public Utils::IBasicInterface
    {
    public:
        virtual void Run(bool& isLastTask) = 0;
        virtual void RunNext() = 0;
    };

    class NObjBroadcastReceiver;
    using boost::scoped_ptr;    
    

    // CommonTaskProfile    
    class NObjCommonBfTaskProfile :
        public Domain::NamedObject,        
        private IBroadcastReceiverReport,
        public BfBootCli::ITaskManagerReport,
        public Utils::ILogForProgressBar
    {
        Q_OBJECT;

    // DRI interface
    public:
        NObjCommonBfTaskProfile(Domain::IDomain *pDomain, const Domain::ObjectName &name)
            : Domain::NamedObject(pDomain, name),
            m_traceClient(false),
            m_traceServer(false),
            m_cbpPort(0),            
            m_addInfoReceiver(0),
            m_postValidator(0),
            m_progress(*this),
            m_pNetTask(0)
        {             
            PropertyWriteEvent(this);
        }                            

        // Настройка доступа к плате        
        Q_PROPERTY(QString BoardAddress WRITE m_addr READ m_addr);
        Q_PROPERTY(int CbpPort WRITE m_cbpPort READ m_cbpPort);
        Q_PROPERTY(QString BoardPwd WRITE m_boardPwd READ ReadPwd);
        Q_PROPERTY(bool TraceClient WRITE m_traceClient READ m_traceClient);        
        Q_PROPERTY(bool TraceServer WRITE m_traceServer READ m_traceServer);        
        Q_INVOKABLE void ObtainCbpPort(DRI::IAsyncCmd *pAsyncCmd, int msec = -1); // альтернатива ручной установке CbpPort

        // получение сетевого адреса доступа к плате по бродкастам 
        Q_INVOKABLE void ObtainAddressByMac(DRI::IAsyncCmd *pAsyncCmd, QString mac, int msec = -1);
        Q_INVOKABLE void ObtainAddressByNum(DRI::IAsyncCmd *pAsyncCmd, int hwType, int hwNum, int msec = -1);

    // NamedObject override
    private:
        bool OnPropertyWrite(NamedObject *pObject, QString propertyName, QString val);
        
    // Used by owner
    public:

        // пользователь объекта (объект задание) обязан вызвать этот метод перед использованием.
        // Разблокировка выполняется при автоматически при завершении задания
        void LockChanges(NamedObject& obj); //??
        QString getBoardHostAddress();        
        BfBootCli::ITransportFactory& getTransport();               
        BfBootCli::TaskMngProfile& getMngProf();
        bool getTraceServer() const;
        std::string getPwd() const;
        void RunTask(DRI::IAsyncCmd *pAsyncCmd, ITask& task, const Domain::ObjectName& taskName, NObjBroadcastReceiver* postValidator = 0);

    // ITaskManagerReport impl
    private:
        void AllTaskFinished(const BfBootCli::TaskResult& result);
        void Info(QString eventInfo, int val);

    // IBroadcastReceiverReport impl
    private:
        void ExpectedMessageReceived(const BfBootCore::BroadcastMsg& msg);
        void OnTimeout();

    //ILogForProgressBar
    private:
        void LogProgressBar(const std::string& msg, bool eof);        

    private:
        QString ReadPwd() const
        {
            return "***";
        }
        void UnlockChanges();
        void AssertUnlocked(); 
        void Log(QString msg);
        void AbortAsync()
        {
            AsyncComplete(false, "Aborted by user");
        }
        void RunNext()
        {
            ESS_ASSERT(m_pNetTask);
            ITask* pNext = m_pNetTask;
            m_pNetTask = 0;
            pNext->RunNext();
        }
        void OnDeleteRec(NObjBroadcastReceiver*)
        {}
        NObjBroadcastReceiver* BeginAddInfoObtain(DRI::IAsyncCmd *pAsyncCmd);

    private:
        scoped_ptr<BfBootCli::TaskMngProfile> m_prof;        
        scoped_ptr<BfBootCli::ITransportFactory> m_transport;
        
        QString m_boardPwd; 
        bool m_traceClient;
        bool m_traceServer;
        QString m_addr;
        int m_cbpPort;

        // Состояние
        QString m_lockedBy; // read only, отработка задачи
                
        NObjBroadcastReceiver* m_addInfoReceiver; // ожидание бродкаста для уточнения CbpPort                
        scoped_ptr<iLogW::LogSession> m_taskLog;
        NObjBroadcastReceiver* m_postValidator;
        
        Utils::ProgressBar m_progress;
        ITask* m_pNetTask;
    };

   
    

} // namespace BfBootDRI

#endif
