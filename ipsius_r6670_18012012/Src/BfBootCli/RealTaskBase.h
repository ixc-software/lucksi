#ifndef REALTASKBASE_H
#define REALTASKBASE_H

#include "ITask.h"
#include "BfBootCore/IBootServerToClient.h"
#include "iCore/MsgObject.h"
#include "iCore/MsgTimer.h"


namespace BfBootCli
{
    // todo:    
    //      - имена тасков через typeid

    class ITaskOwner;
    using Platform::dword;    

    class TaskBase : public iCore::MsgObject,
        public ITask, private BfBootCore::IBootServerToClient, 
        boost::noncopyable
    {
        enum State {st_null, st_started, st_complete, st_failed};
        ITaskOwner& m_callback;
        iCore::MsgTimer m_taskTimeout;
        dword m_taskTimeoutVal;
        State m_state;   

        void Start();
        void OnHalt(const std::string& info);
        void OnComplete();
        void OnTimeout(iCore::MsgTimer*);

    // Task child interface
    private:
        virtual void RunTask( BfBootCore::IClientToBooter& remoteServer ) = 0;
        virtual QString InfoTask() const = 0;

    // ITask impl
    private:
        void Run( BfBootCore::IClientToBooter& remoteServer );
        QString Info() const;
        BfBootCore::IBootServerToClient& getClientIntf();

    // IProtoObserver impl. Processed as unexpected events. (exclude Info)
    private:        
        void RespLogin(const BfBootCore::UserResponseInfo&, const std::string&, const std::string&, bool);
        void RespCmd(const BfBootCore::UserResponseInfo& code);
        void RespGetParam(const std::vector<BfBootCore::ConfigLine>& userParams);
        void RespGetAppList(const BfBootCore::ImgDescrList& appList);
        void RespNewImgAdded(const BfBootCore::ImgDescriptor&);
        void RespGetScriptList(const BfBootCore::ScriptList&);    
        void StateInfo(const BfBootCore::SrvStateInfo& info, Platform::int32 progress);

    protected:
        TaskBase(ITaskOwner& callback, dword taskTimeoutVal = 0); // if taskTimeoutVal == 0 don`t control timeout
        bool IsActive() const;
        void Halt(const std::string& info);
        void Complete();   
        ITaskOwner& getOwner();                                
    };    

} // namespace BfBootCli

#endif
