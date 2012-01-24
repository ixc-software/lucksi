#include "stdafx.h"

#include "Platform/Platform.h"
#include "iCoreTests/TestUtils.h"
#include "iCore/SyncTimer.h"
#include "iCore/MsgObject.h"

#include "MutexTest.h"
#include "Utils/Random.h"
#include "Utils/ErrorsSubsystem.h"
#include "Utils/AtomicTypes.h"
#include "Utils/ManagedList.h"



namespace 
{
    using namespace iCore;
    using namespace Utils;
    using namespace TestUtils;

    enum 
    { 
        CMutexBreakerCount = 10, 
        CMinLockCount = 5, 
        CTestDurationMS = 50, 
        CSleepDurationMS = 10,

        CDuration = CMutexBreakerCount * CTestDurationMS 
                    * CSleepDurationMS * 2,
    };

    class ListMutexBreakers
    {
        class TestObject
        {
            AtomicBool m_testFlag; //false
            Platform::Mutex m_locker; //test obj
            Random m_random;
    
        public:
            TestObject()
            {}
    
            void TestLock()
            {
                m_locker.Lock();
    
                TUT_ASSERT(!m_testFlag.Get()); // <---- test ok
    
                m_testFlag.Set(true);
    
                int diff = CSleepDurationMS / 2;
                Platform::Thread::Sleep(m_random.Next(diff) + (CSleepDurationMS - diff)); //ms
    
                m_testFlag.Set(false);
    
                m_locker.Unlock();
            }
        };

        class MutexBreaker : public QThread
        {   
            TestObject& m_testObject;
            AtomicBool m_done;
            dword m_testDurationMS;
            int m_lockCount;

   
            void run()
            {
                SyncTimer t;
                while ((t.Get() < m_testDurationMS) && (m_lockCount))
                {
                    m_testObject.TestLock();
                    --m_lockCount;
                }
                m_done.Set(true);
            }
            
        public:
            MutexBreaker(TestObject& testObject, Priority threadPriority, dword testDurationMS) 
                : m_testObject(testObject), m_testDurationMS(testDurationMS), m_lockCount(CMinLockCount)            
            {
                start(threadPriority);
            }
    
            ~MutexBreaker()
            {
                QThread::wait();
            }

            bool getDone()
            {
                return m_done;
            }
        };

        TestObject m_testObject;
        ManagedList<MutexBreaker> m_breakers;
        
    public:
        ListMutexBreakers(int count)
        {
            m_breakers.Reserve(count);
        
            QThread::Priority p;
            for (int i = 0; i < count; ++i)
            {
                p = ((i % 2) == 0) ? (QThread::LowPriority) : (QThread::LowestPriority);
                m_breakers.Add(new MutexBreaker(m_testObject, p, CTestDurationMS));
            }        
        }

        bool IsProcessed()
        {
            for(size_t i = 0; i < m_breakers.Size(); ++i)
            {
                if (!m_breakers[i]->getDone()) return false;
            }

            return true;
        }
    };

    // ----------------------------------------

    class MutexTestClass : public TestProcess, 
                        public TestLog
    {
        ListMutexBreakers m_breakers;

        bool IsProcessed()
        {
            return m_breakers.IsProcessed();
        }

    public:
        MutexTestClass() 
            : TestProcess(CDuration), 
                TestLog("Mutex test", "Test strarted." ),
                m_breakers(CMutexBreakerCount)
        {}

        void ShowLog()
        {
            /*
            std::string s = "QuickLock conflicts count = ";
            Log(s, QuickLock::GetConflictsCount());

            WriteTo(std::cout); */
        }
    };

} // namespace

// ----------------------------------------

namespace UtilsTests
{
    void MutexTest(bool showInfo)
    {
        MutexTestClass test;
        test.Run();
    
        if (showInfo) test.ShowLog();
    }

} // namespace UtilsTests
