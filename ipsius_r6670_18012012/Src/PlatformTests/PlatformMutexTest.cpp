#include "stdafx.h"

#include "Platform/Platform.h"

#include "Utils/TimerTicks.h"
#include "Utils/ErrorsSubsystem.h"
#include "Utils/ManagedList.h"
#include "Utils/Random.h"
#include "Utils/AtomicTypes.h"

#include "PlatformTests.h"

// -------------------------------------

namespace MultiThreadLock
{
    enum
    {
        CLockNumber = 8,
        CSleepTime  = 20,
        CLockTime   = 10,

        CThreadNumber = 4,
        CTestTimeout  = (CSleepTime + CLockTime) * CThreadNumber * 10,
    };

    Utils::AtomicBool GInLock;
    Utils::AtomicInt  GLockCount;


    class Thread : public Platform::Thread
    {
        Platform::Mutex &m_mutex;
        Utils::Random m_random;

        static void LockBody(Platform::Mutex &mutex, int lockTime)
        {
            mutex.Lock();

            TUT_ASSERT( !GInLock.Get() );

            GInLock.Set(true);
            Sleep(lockTime);
            GInLock.Set(false);

            GLockCount.Inc();

            mutex.Unlock();
        }
        
        void run()  // override
        {
            for(int i = 0; i < CLockNumber; ++i)
            {
                int lockTime = 1 + m_random.Next(CLockTime);
                LockBody(m_mutex, lockTime);

                Sleep(1 + m_random.Next(CSleepTime));
            }
        }

    public:

        Thread(Platform::Mutex &mutex, int seed) 
            : Platform::Thread("Thread"), m_mutex(mutex), m_random(seed)
        {
            start(LowPriority);
        }

    };

	void Test()
	{		
        GInLock.Set(false);
        GLockCount.Set(0);

        Platform::Mutex mutex;
        Utils::ManagedList<Thread> threads;

        for(int i = 0; i < CThreadNumber; ++i)
        {
            threads.Add( new Thread(mutex, i * i) );
        }

        // wait
        Utils::TimerTicks t;
        t.Set(CTestTimeout);

        while(true)
        {
            if (t.TimeOut()) TUT_ASSERT(0 && "Timeout!");

            Platform::ThreadSleep(CSleepTime);

            // all thread done
            bool done = true;
            for(int i = 0; i < CThreadNumber; ++i)
            {
                if (!threads[i]->isFinished())
                {
                    done = false;
                    break;
                }
            }

            if (done) break;
        }

        TUT_ASSERT(GLockCount.Get() == (CThreadNumber * CLockNumber));
	}

}

// -------------------------------------

namespace Collisions
{

    void DeadlockTest()
    {
        Platform::Mutex mutex;

        mutex.Lock();
        mutex.Lock();  // deadlock here
    }

    // -------------------------------------------------

    void WrongUnlock()
    {
        Platform::Mutex mutex;

        mutex.Unlock();  // bug here, mutex not locked
        mutex.Unlock();

        mutex.Lock();
        mutex.Unlock();
    }

    // -------------------------------------------------

    class Thread : public Platform::Thread
    {
        Platform::Mutex &m_mutex;

        void run()  // override
        {
            m_mutex.Unlock(); // bug here, unlock in another thread
        }

    public:

        Thread(Platform::Mutex &mutex) : Platform::Thread("Thread"), m_mutex(mutex)
        {
            start(HighPriority);
        }

    };

    void WrongUnlockContext()
    {
        Platform::Mutex mutex;
        mutex.Lock();

        Thread thread(mutex);
        thread.wait();
    }

}  // namespace Collisions


// -------------------------------------

namespace PlatformTests
{

	void MutexTest()
	{
        // All this tests stop application:
        // Collisions::DeadlockTest();        
        // Collisions::WrongUnlock();
        // Collisions::WrongUnlockContext();        

        // MultiThreadLock::Test();
        int count = 8;
        while(count--)
        {
            MultiThreadLock::Test();
        }
		
	}
	
		
}  // namespace PlatformTests




