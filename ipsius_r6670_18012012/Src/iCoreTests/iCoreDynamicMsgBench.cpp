#include "stdafx.h"

#include "Utils/IBasicInterface.h"
#include "iCore/MsgObject.h"
#include "iCore/MsgTemplates.h"
#include "iCore/MsgTimer.h"
#include "Domain/DomainTestRunner.h"
#include "Domain/DomainClass.h"

#include "iCoreDynamicMsgBench.h"

// -------------------------------------------

namespace
{
    using namespace iCore;
    using Domain::DomainClass;

    enum 
    { 
        CTestTime = 1000,
        CTestTimeout = CTestTime * 2,
    };

    class ITestInterface : public virtual IMsgObject
    {
    public:
        virtual void SendMessage() = 0;
    };

    // -----------------------------------------------

    class TestParams 
    {
        int m_testTime; 
        bool m_useDirectMode;
        int m_msgCount;

    public:
        TestParams(int testTime, bool useDirectMode) 
            : m_testTime(testTime), m_useDirectMode(useDirectMode), m_msgCount(0)
        {
        }

        int getTestTime() const { return m_testTime; }
        bool getUseDirectMode() const { return m_useDirectMode; }

        int getMsgCount() const { return m_msgCount; }
        void setMsgCount(int value) { m_msgCount = value; }

    };

    // -----------------------------------------------

    class TestClass : public MsgObject, public ITestInterface
    {
        DomainClass &m_domain;
        TestParams &m_params;
        MsgTimer m_timer;
        int m_msgCount;
        ITestInterface &m_intf;

        void TestEnd(MsgTimer *pT)
        {
            m_params.setMsgCount(m_msgCount);
            m_domain.Stop(Domain::DomainExitOk);
        }

        template<class TOwner, class TFn>
        MsgBase* CreateMsg(TOwner owner, TFn fn)
        {
            return new MsgSimple<TOwner, TFn>(owner, fn);
        }

    // ITestInterface impl
    private:

        void SendMessage()
        {
            m_msgCount++;

            if (m_params.getUseDirectMode())
            {
                PutMsg(this, &TestClass::SendMessage);
            }
            else
            {
                MsgBase *p = CreateMsg(&m_intf, &ITestInterface::SendMessage);
                m_intf.PutMsgInQueue(p);
            }
        }

    public:
        TestClass(DomainClass &domain, TestParams &params) 
            : MsgObject(domain.getMsgThread()), 
            m_domain(domain), m_params(params), 
            m_timer(this, &TestClass::TestEnd), m_msgCount(0), m_intf(*this)
        {
            m_timer.Start(m_params.getTestTime());
            SendMessage();
        }
    };

    int Run(bool directMode)
    {
        Domain::DomainTestRunner runner(CTestTimeout);

        TestParams params(CTestTime, directMode);

        bool res = runner.Run<TestClass>(params);
        TUT_ASSERT(res);

        return params.getMsgCount();
    }



}  // namespace

// -------------------------------------------

namespace iCoreTests
{
    void iCoreDynamicMsgBenchmark()
    {
        int direct = Run(true);
        int dynamic = Run(false);
    
        if (dynamic > 0)
        {
            int diff = ((direct * 100) / dynamic) - 100;
            int i = 0;
            // ...
        }
    }

} // namespace iCoreTests
