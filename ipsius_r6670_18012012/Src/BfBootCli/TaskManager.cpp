#include "stdafx.h"
#include "TaskManager.h"
#include "Utils/QtHelpers.h"

namespace BfBootCli
{         
    void TaskManager::OnConnectTimout( iCore::MsgTimer* p)
    {
        ESS_ASSERT(p == &m_connectionTimeout);
        OnFullTaskFailed("Connection timeout");
    }

    // ------------------------------------------------------------------------------------

    void TaskManager::OnDeleteTask( ITask* )
    {
    }

    // ------------------------------------------------------------------------------------

    void TaskManager::OnFullTaskFailed( const std::string& info )
    {
        m_remoteServer.DestroyConnection();
        m_owner.AllTaskFinished(TaskResult(false, info));
    }

    // ------------------------------------------------------------------------------------

    void TaskManager::OnFullTaskComplete()
    {
        m_remoteServer.DestroyConnection();
        m_owner.AllTaskFinished(TaskResult(true, "Complete"));
    }

    // ------------------------------------------------------------------------------------

    void TaskManager::DiscWithProtoError( const std::string& info )
    {
        if (m_tasks.IsEmpty()) return;
        *m_logSession << "DiscWithProtoError: " << info << EndRecord;
        PutMsg(this, &TaskManager::OnFullTaskFailed, info);
    }

    // ------------------------------------------------------------------------------------

    void TaskManager::Connected()
    {
        ESS_ASSERT(!m_tasks.IsEmpty());
        *m_logSession << "Remote server connected." << EndRecord;
        m_connectionTimeout.Stop();
        m_owner.Info("Connected.");
        StartTask_0();
    }

    // ------------------------------------------------------------------------------------

    void TaskManager::StartTask_0()
    {
        QString msg = QString("%1 started...").arg(Platform::FormatTypeidName( typeid(*m_tasks[0]).name() ).c_str());
        *m_logSession << msg << EndRecord; 
        m_owner.Info(msg);        
        m_tasks[0]->Run(m_remoteServer);
    }

    // ------------------------------------------------------------------------------------

    void TaskManager::TaskComplete( const ITask* taskId )
    {
        ESS_ASSERT(m_tasks[0] == taskId);

        QString msg = QString("%1 complete.")
            .arg(Platform::FormatTypeidName( typeid(*m_tasks[0]).name() ).c_str());

        *m_logSession << msg << iLogW::EndRecord;
        m_owner.Info(msg);

        ITask* pTask = m_tasks.Detach(0);
        PutMsg(this, &TaskManager::OnDeleteTask, pTask); // удалится очередью
        //m_tasks.Delete(0);

        if (m_tasks.IsEmpty())
        {
            PutMsg(this, &TaskManager::OnFullTaskComplete);                
            return;
        }

        // run next
        m_remoteServer.ResetProtoObserver(m_tasks[0]->getClientIntf());            
        StartTask_0();
    }

    // ------------------------------------------------------------------------------------

    void TaskManager::TaskFailed( const ITask* taskId, QString errorMsg )
    {
        ESS_ASSERT(taskId == m_tasks[0]);
        *m_logSession << "Task " << Platform::FormatTypeidName(typeid(*m_tasks[0]).name() ) 
            << " failed. Error info: " << errorMsg << iLogW::EndRecord;

        PutMsg(this, &TaskManager::OnFullTaskFailed, errorMsg.toStdString());
    }

    // ------------------------------------------------------------------------------------

    iCore::MsgThread& TaskManager::getThread()
    {
        return getMsgThread();
    }

    // ------------------------------------------------------------------------------------

    QString TaskManager::StateInfo()
    {
        if (!m_active) return "Inactive state.";
        return m_tasks[0]->Info();
    }

    // ------------------------------------------------------------------------------------

    TaskManager::TaskManager( const TaskMngProfile& prof, boost::shared_ptr<SBProto::ISbpTransport> transport, Platform::dword waitMsec ) 
        : iCore::MsgObject(prof.Domain.getDomain().getMsgThread()),
        m_logSession(prof.LogCreator->CreateSession("TaskManager", prof.TraceActive)),
        m_owner(prof.Owner),                
        m_timeoutMsec(waitMsec),
        m_connectionTimeout(this, &TaskManager::OnConnectTimout),
        m_remoteServer(getMsgThread(), 
			*this, 
			transport, 
			m_logSession->LogCreator()),
        m_active(false)
    {}

    // ------------------------------------------------------------------------------------

    void TaskManager::RunTasks()
    {
        ESS_ASSERT(!m_active);
        if (m_tasks.IsEmpty()) return m_owner.AllTaskFinished(TaskResult(true, "Nothing todo"));              
        
        m_owner.Info("Connecting...");
        m_remoteServer.ResetProtoObserver(m_tasks[0]->getClientIntf());                    
        m_connectionTimeout.Start(m_timeoutMsec);         
        
        m_active = true;
    }

    // ------------------------------------------------------------------------------------

    void TaskManager::AddTask( ITask* pTask )
    {
        ESS_ASSERT(!m_active);
        ESS_ASSERT(pTask);
        m_tasks.Add(pTask);
    }

    void TaskManager::Info( const ITask* taskId, QString eventInfo, int progress /*= -1*/ )
    {
        ESS_ASSERT(taskId == m_tasks[0]);
        m_owner.Info(
            /*Platform::FormatTypeidName(typeid(*m_tasks[0]).name() ),*/
            eventInfo, progress
            );                   
    }
} // namespace BfBootCli

