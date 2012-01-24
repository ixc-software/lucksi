#include "stdafx.h"
#include "RealTaskBase.h"
#include "ITaskOwner.h"


namespace BfBootCli
{
    TaskBase::TaskBase( ITaskOwner& callback, dword taskTimeoutVal ) : MsgObject(callback.getThread()), 
        m_callback(callback), 
        m_taskTimeout(this, &TaskBase::OnTimeout),
        m_taskTimeoutVal(taskTimeoutVal),
        m_state(st_null)
    {
        callback.AddTask(this);
    }

    // ------------------------------------------------------------------------------------

    void TaskBase::Start()
    {
        ESS_ASSERT(m_state == st_null);
        if (m_taskTimeoutVal > 0) m_taskTimeout.Start(m_taskTimeoutVal);
        m_state = st_started;
    }

    // ------------------------------------------------------------------------------------

    void TaskBase::OnHalt( const std::string& info )
    {
        if (m_state == st_failed) return; // already Halt
        ESS_ASSERT(m_state == st_started);
        m_state = st_failed;

        m_callback.TaskFailed(this, info.c_str());
    }

    /*void TaskBase::SetTimeout( dword taskTimeout )
    {
        m_taskTimeoutVal = taskTimeout;
        m_taskTimeout.Start(m_taskTimeoutVal);
    }*/

    // ------------------------------------------------------------------------------------

    void TaskBase::OnComplete()
    {
        if (m_state == st_complete) return;                         
        ESS_ASSERT(m_state == st_started);
        m_state = st_complete;

        m_callback.TaskComplete(this);
    }

    // ------------------------------------------------------------------------------------

    void TaskBase::Run( BfBootCore::IClientToBooter& remoteServer )
    {
        Start();
        RunTask(remoteServer);
    }

    // ------------------------------------------------------------------------------------

    QString TaskBase::Info() const
    {
        return InfoTask();
    }

    void TaskBase::StateInfo(const BfBootCore::SrvStateInfo& info, Platform::int32 progress)
    {
        m_callback.Info(this, info.AsString().c_str(), progress);
    }

    // ------------------------------------------------------------------------------------

    BfBootCore::IBootServerToClient& TaskBase::getClientIntf()
    {
        return *this;
    }

    // ------------------------------------------------------------------------------------

    void TaskBase::RespLogin( const BfBootCore::UserResponseInfo&, const std::string&, const std::string&, bool )
    {
        Halt("Unexpected RespLogin");
    }

    // ------------------------------------------------------------------------------------

    void TaskBase::RespCmd( const BfBootCore::UserResponseInfo& code )
    {
        Halt("Unexpected RespCmd");
    }

    // ------------------------------------------------------------------------------------

    void TaskBase::RespGetParam( const std::vector<BfBootCore::ConfigLine>& userParams )
    {
        Halt("Unexpected RespGetParam");
    }

    // ------------------------------------------------------------------------------------

    void TaskBase::RespGetAppList( const BfBootCore::ImgDescrList& appList )
    {
        Halt("Unexpected RespGetAppList");
    }

    // ------------------------------------------------------------------------------------

    void TaskBase::RespNewImgAdded( const BfBootCore::ImgDescriptor& )
    {
        Halt("Unexpected RespNewImgAdded");
    }

    // ------------------------------------------------------------------------------------

    void TaskBase::RespGetScriptList( const BfBootCore::ScriptList& )
    {
        Halt("Unexpected RespGetScriptList");
    }

    // ------------------------------------------------------------------------------------

    bool TaskBase::IsActive() const
    {
        return m_state == st_started;
    }

    // ------------------------------------------------------------------------------------

    void TaskBase::Halt( const std::string& info )
    {
        m_taskTimeout.Stop();
        PutMsg(this, &TaskBase::OnHalt, info);
    }

    // ------------------------------------------------------------------------------------

    void TaskBase::Complete()
    {
        m_taskTimeout.Stop();
        PutMsg(this, &TaskBase::OnComplete);
    }

    void TaskBase::OnTimeout( iCore::MsgTimer* )
    {
        m_taskTimeout.Stop();
        OnHalt( InfoTask().append(" Task failed by timeout.").toStdString() );
    }

    ITaskOwner& TaskBase::getOwner()
    {
        return m_callback;
    }

    
} // namespace BfBootCli

