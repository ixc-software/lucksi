#ifndef ITASKOWNER_H
#define ITASKOWNER_H

#include "stdafx.h"
#include "iCore/MsgThread.h"

namespace BfBootCli
{    
    class ITask;

    class ITaskOwner : public Utils::IBasicInterface
    {
    public:
        virtual void TaskComplete(const ITask* taskId) = 0; 
        virtual void TaskFailed(const ITask* taskId, QString errorMsg) = 0;      
        virtual void Info(const ITask* taskId, QString eventInfo, int progress = -1) = 0; 

        // Принимает во владение
        virtual void AddTask(ITask* pTask) = 0;        

        virtual iCore::MsgThread& getThread() = 0;
    };
} // namespace BfBootCli

#endif
