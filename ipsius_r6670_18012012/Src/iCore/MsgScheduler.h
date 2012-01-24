#pragma once

#include "Utils/ManagedList.h"
#include "Platform/PlatformMutex.h"

#include "MsgObject.h"
#include "MsgTimer.h"

namespace iCore
{
    
    /* 
        It's like MsgThreadFn.h but in much more safe manner 'couse it put messages to MsgObject host
    */
    
    class MsgScheduler : public MsgObject
    {
        typedef boost::function<void ()> RunFn;
        typedef MsgScheduler T;
        typedef MsgTimerT<RunFn> Timer;

        struct TimerPtr
        {
            Timer *p;
        };

        Platform::Mutex m_mutex;  // for m_list
        Utils::ManagedList<Timer> m_list;

        void RunFunc(RunFn fn)
        {
            fn();
        }

        void DeleteTimer(TimerPtr val)
        {
            Platform::MutexLocker lock(m_mutex);

            int i = m_list.Find(val.p);            
            ESS_ASSERT(i >= 0);
            m_list.Delete(i);
        }

        void OnTimer(Timer *pT)
        {
            // request timer delete (thru stuct 'couse auto delete in message)
            TimerPtr val; 
            val.p = pT;
            MsgObject::PutMsg(this, &T::DeleteTimer, val);

            // run function 
            RunFn f = pT->getTag();
            f();
        }

    public:

        MsgScheduler(MsgThread &t) : MsgObject(t)
        {
        }

        void PutMsg(const RunFn &fn, int delayMs = 0)
        {
            ESS_ASSERT(fn);
            ESS_ASSERT(delayMs >= 0);
            
            if (delayMs == 0)
            {
                MsgObject::PutMsg(this, &T::RunFunc, fn);
                return;
            }

            {
                Platform::MutexLocker lock(m_mutex);

                Timer *p = new Timer(this, &T::OnTimer);
                p->setTag(fn);
                p->Start(delayMs);

                m_list.Add(p);
            }
        }

    };
        
}  // namespace iCore