#include "stdafx.h"

#include "Utils/FuncRAII.h"

#include "MsgObject.h"
#include "MsgTimer.h"
#include "MsgThreadFn.h"

// -----------------------------------------

namespace
{
    using namespace iCore;

    class Runner : public MsgThread::AsyncTask
    {
        boost::function<void ()> m_fn;

        void Execute()  // override
        {
            m_fn();
        }

    public:

        Runner(const boost::function<void ()> &fn) : m_fn(fn)
        {
        }

    };

    // -------------------------------------------------

    class DelayedRunner : public MsgObject
    {
        typedef DelayedRunner T;

        MsgTimer m_timer;
        boost::function<void ()> m_fn;

        void OnTimer(MsgTimer *pT)
        {
            // or just call AsyncDeleteSelf() :)
            Utils::FuncRAII raii( boost::bind(&T::AsyncDeleteSelf, this) );  

            m_fn();
        }

        void Init(int delayMs)
        {
            m_timer.Start(delayMs, false);
        }

    public:

        DelayedRunner(MsgThread &thread, const boost::function<void ()> &fn, int delayMs) :
          MsgObject(thread),
          m_timer(this, &T::OnTimer),
          m_fn(fn)
        {
            PutMsg(this, &T::Init, delayMs);
        }

    };

}

// -----------------------------------------

namespace iCore
{

    void MsgThreadFn(MsgThread &thread, const boost::function<void ()> &fn)
    {
        ESS_ASSERT( fn );

        boost::shared_ptr<MsgThread::AsyncTask> task( new Runner(fn) );
        thread.RunAsyncTask(task);
    }

    void MsgThreadFnDelayed(MsgThread &thread, const boost::function<void ()> &fn, int delayMs)
    {
        ESS_ASSERT( fn && delayMs > 0 );

        // yes, it fly in space, pointer wasn't stored
        DelayedRunner *p = new DelayedRunner(thread, fn, delayMs);
    }


}  // namespace iCore
