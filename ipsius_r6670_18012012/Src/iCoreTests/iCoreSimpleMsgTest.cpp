
#include "stdafx.h"
#include "iCoreSimpleMsgTest.h"

#include "iCore/MsgObject.h"
#include "iCore/MsgThread.h"
#include "iCore/SyncTimer.h"

#include "Utils/ErrorsSubsystem.h"
#include "Utils/Random.h"
#include "Utils/AtomicTypes.h"
#include "Utils/SafeRef.h"


namespace
{
    using namespace iCore;

    class IncrementParam
    {
        int m_value;

        static Utils::AtomicInt ClassCounter;

    public:
        IncrementParam(int value) : m_value(value)
        {
            ClassCounter.Inc();
        }

        ~IncrementParam()
        {
            ClassCounter.Dec();
        }

        int getValue() { return m_value; }

        static void Reset()
        {
            ClassCounter.Set(0);
        }

        static void Check()
        {
            TUT_ASSERT(ClassCounter == 0);
        }
    };

    Utils::AtomicInt IncrementParam::ClassCounter;

    // ----------------------------------------------------

    class MsgTestObject : public MsgObject
    {

        class InnerClass
        {
            int m_counter;

        public:

            InnerClass() : m_counter(0)
            {
            }

            void TestFn() 
            {
                m_counter++;
            }
        };

        int m_counter;
        InnerClass m_inner;

        void OnIncrement()
        {
            m_counter++;
        }

        void OnDecrement()
        {
            m_counter--;
        }

        void OnAdd(int value)
        {
            m_counter += value;
        }

        void OnIncrementByParam(IncrementParam *pParam)
        {
            ESS_ASSERT(pParam);
            m_counter += pParam->getValue();
        }

        void SendToInnerTestFn()
        {
            m_inner.TestFn();
        }


        typedef MsgTestObject T;

    public:

        MsgTestObject(MsgThread &thread) 
            : MsgObject(thread), m_counter(0)
        {
        }

        void Increment()
        {
            PutMsg(this, &T::OnIncrement);
        }

        void Decrement()
        {
            PutMsg(this, &T::OnDecrement);
        }

        void Add(int value)
        {		
            PutMsg(this, &T::OnAdd, value);
        }

        void TestOtherType()
        {
            // PutMsg(&m_inner, &InnerClass::TestFn);
            PutMsg(this, &T::SendToInnerTestFn);
        }

        void IncrementByParam(IncrementParam *pParam)
        {
            PutMsg(this, &T::OnIncrementByParam, pParam);
        }

        int getCounter() { return m_counter; }
    };

    // -------------------------------------------------------------

    void WaitForMsgCount(const MsgThread &m, int count, dword timeout = 0xFFFFFFFF)
    {
        SyncTimer t;

        while(t.Get() < timeout)
        {
            Platform::Thread::Sleep(50);

            if (m.getSleeping())
            {
                if (m.GetStats().MsgProcessed >= count) return;
            }

        }

        TUT_ASSERT(0 && "Timeout");
    }

    // -------------------------------------------------------------

    class ThreadExceptionsHook : 
        public virtual Utils::SafeRefServer,
        public IMsgProcessorErrorHook
    {
        int m_counter;

    // IMsgProcessorErrorHook impl
    private:

        void OnMsgException(const std::exception &e, MsgBase *pMsg, bool &suppressIt) 
        {
            if ( dynamic_cast<const ESS::HookRethrow*>(&e) )
            {
                suppressIt = true;
                m_counter++;
            }

        }

    public:

        ThreadExceptionsHook() : m_counter(0)
        {
        }

        int getCounter() { return m_counter; }

    };


}  // namespace

// -------------------------------------------------------------

namespace iCoreTests
{
    void iCoreSimpleMsgTest()
    {
        MsgThread m("iCoreSimpleMsgTest", Platform::Thread::LowPriority);
    	m.Run();
    
    	MsgTestObject obj(m);
    
    	obj.Increment();
    	obj.Add(10);
    	WaitForMsgCount(m, 2);
    		
    	obj.Increment();
    	obj.Increment();
    	obj.Decrement();  
    	WaitForMsgCount(m, 2 + 3);   // 12
    
    	TUT_ASSERT(obj.getCounter() == 12);
    }
    
    // -------------------------------------------------------------
    
    void iCoreParamMsgTest()
    {
        MsgThread m("iCoreParamMsgTest", Platform::Thread::LowPriority);
        m.Run();
    
        MsgTestObject obj(m);
    
        // test message param delete
        IncrementParam::Reset();
    
        obj.IncrementByParam(new IncrementParam(10));
        obj.IncrementByParam(new IncrementParam(20));
        obj.IncrementByParam(new IncrementParam(30));  // +60
        WaitForMsgCount(m, 3);  
    
        IncrementParam::Check();
    
        TUT_ASSERT(obj.getCounter() == 60);
    }
    
    
    // -------------------------------------------------------------
    
    void MakeTestObject(MsgThread &m)
    {
        MsgTestObject obj(m);
        obj.Increment();
        obj.Increment();
        obj.Increment();
    }
    
    // -------------------------------------------------------------
    
    /*
         Этот тест раньше падал на проверке сигнатуры, теперь он проверяет
         невозможность отправить сообщение объекту после того, как был вызван
         деструктор MsgObject
    
    */
    
    void iCoreSignatureCheck()
    {
        ESS::ExceptionHookRethrow<> globalHook;
    
        ThreadExceptionsHook hook;

        MsgThread m("iCoreSignatureCheck", Platform::Thread::LowPriority);    
        m.AddErrorHook(&hook);
    
        m.Run();
    
        MakeTestObject(m);
    
        SyncTimer t;
    
        while(t.Get() < 1000)
        {
            Platform::Thread::Sleep(50);

            if (hook.getCounter())  // Test OK
            {
                Platform::Thread::Sleep(100);
                return;  
            }
        }
    
        TUT_ASSERT(0 && "Wait fail!");   
    }

} // namespace iCoreTests
