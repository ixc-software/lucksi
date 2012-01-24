#ifndef __ITIMERMANAGER__

#define __ITIMERMANAGER__

#include "stdafx.h"
#include "Platform/PlatformTypes.h"
#include "Utils/IBasicInterface.h"

namespace iCore
{
    using Platform::dword;
    class MsgObjectBase;

    class IMsgTimer : public Utils::IBasicInterface
    {
    protected:
        friend class TimerThread;

        virtual void SafeStop() = 0;
        virtual void SafeStart(dword currTicks, dword interval, bool repeated) = 0;
        virtual dword GetWaitTime(dword currTicks) = 0;
        virtual bool Process(dword currTicks) = 0;
        virtual bool TryRestart(dword currTicks) = 0;
        virtual void NotifyMsgObjectDestroyed(MsgObjectBase *pObject) = 0;
    };

    class ITimerManager : public Utils::IBasicInterface
    {
    public:
        virtual void TimerStart(IMsgTimer *pT, dword interval, bool repeated) = 0;
        virtual void TimerStop(IMsgTimer *pT) = 0;

    };
	
}  // namespace iCore

#endif

