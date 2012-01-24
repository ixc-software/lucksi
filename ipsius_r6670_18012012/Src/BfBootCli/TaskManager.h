#ifndef TASKMANAGER_H
#define TASKMANAGER_H

#include "Utils/ManagedList.h"
#include "iCore/MsgObject.h"
#include "iCore/MsgTimer.h"
#include "iLog/LogWrapper.h"
#include "BootControlProtoConnection.h"
#include "ITask.h"
#include "ITaskManagerReport.h"
#include "IProtoConnectionToOwner.h"
#include "ITaskManager.h"
#include "TaskMngProfile.h"
#include "ITaskOwner.h"


namespace BfBootCli
{    
    using boost::scoped_ptr;
    using Platform::word;    

    class TaskManager :   
        boost::noncopyable,
        public iCore::MsgObject,
        public ITaskOwner,
        public IProtoConnectionToOwner,
        public ITaskManager
    {
        scoped_ptr<iLogW::LogSession> m_logSession;
        ITaskManagerReport& m_owner;      
        Platform::dword m_timeoutMsec;
        iCore::MsgTimer m_connectionTimeout;
        BootControlProtoConnection m_remoteServer; 
        Utils::ManagedList<ITask> m_tasks;  
        bool m_active;

    private:
        void OnConnectTimout(iCore::MsgTimer*);
        void OnDeleteTask (ITask*);
        void OnFullTaskFailed(const std::string& info);
        void OnFullTaskComplete();
        void StartTask_0();              

    // IProtoConnectionToOwner
    private:
        void DiscWithProtoError(const std::string& info);
        void Connected();

    // ITaskOwner
    private:
        void TaskComplete(const ITask* taskId);
        void TaskFailed(const ITask* taskId, QString errorMsg);            
        void AddTask(ITask* pTask); // Пошаговое програмирование задачи. Создаваемые
        iCore::MsgThread& getThread();

        void Info(const ITask* taskId, QString eventInfo, int progress = -1);

    // ITaskManager impl
    private:        
        QString StateInfo();
          
    public:
        TaskManager(const TaskMngProfile& prof, boost::shared_ptr<SBProto::ISbpTransport> transport, Platform::dword waitMsec);        
        void RunTasks(); // Начать выполнение // need AsyncRun -- ?          
    };

} // namespace BfBootCli

#endif
