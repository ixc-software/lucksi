#include "stdafx.h"
#include "NObjIsdnTestRunner.h"

// tests:
#include "LapdTestClass.h"
#include "RealScenaries.h"
#include "ReActivationL3Test.h"
#include "WrongL3PacketTest.h"

namespace IsdnTest
{
    // откладывает конструирование теста после создания профайла
    template <class TTest>
    class TestShell : public ITestShell
    {        
        Domain::NamedObject& m_parent;
        typedef typename TTest::TNObjProfile TNObjProfile;
        TNObjProfile* m_profile;
        boost::scoped_ptr<TTest> m_test;

        bool m_manageProfile;

    // ITestShell impl:
    private:
        void Run(ITestResult& callback)
        {            
            m_test.reset(new TTest(m_parent.getDomain(), *m_profile, callback));  
        }   

        void AbortTest()
        {
            m_test->AbortTest();
        }

        QString TestName()
        {
            return ISDN::IsdnUtils::TypeToShortName<TTest>();
        }

    public:
        TestShell(Domain::NamedObject *pParent) : m_parent(*pParent),
            m_profile( new TNObjProfile(pParent, QString("Profile")) ),
            m_manageProfile(true)
        {}

        ~TestShell()
        {
            if (m_manageProfile) m_profile->Delete();
        }

        // call this only in owner destructor.
        void DoNotDeleteProfile()
        {
            m_manageProfile = false;
        }
    };

    // ------------------------------------------------------------------------------------
    // NObjIsdnTestRunner impl

    template<class T>
    void NObjIsdnTestRunner::CreateTest()
    {
        try
        {
            m_test.reset(new TestShell<T>(this));                      
        }
        catch(const ESS::BaseException& e)
        {
            ThrowRuntimeException(e.getTextMessage());
        }
    }

    // ------------------------------------------------------------------------------------
    // Q_INVOKABLE 

    Q_INVOKABLE void NObjIsdnTestRunner::BeginLapdTest()
    {
        CreateTest<LapdTest>();
    }    

    Q_INVOKABLE void NObjIsdnTestRunner::BeginCheckBchannelsBusy()
    {
        CreateTest<ScnCheckBchannelsBusy>();
    }    

    Q_INVOKABLE void NObjIsdnTestRunner::BeginCheckConnectionDisconnectionRoutine()
    {
        CreateTest<ScnCheckConnectionDisconnectionRoutine>();
    }

    Q_INVOKABLE void NObjIsdnTestRunner::BeginCheckReloadDrv()
    {
        CreateTest<ScnCheckReloadDriver>();
    }

    Q_INVOKABLE void NObjIsdnTestRunner::BeginCheckReloadLapd()
    {
        CreateTest<ScnCheckReloadL2>();
    }

    Q_INVOKABLE void NObjIsdnTestRunner::BeginCheckReloadUser()
    {
        CreateTest<ScnCheckReloadUser>();
    }

    Q_INVOKABLE void NObjIsdnTestRunner::BeginSimulLiveTest()
    {
        CreateTest<ScnSimulLive>();
    }

    Q_INVOKABLE void NObjIsdnTestRunner::BeginDeactivationTest()
    {
        CreateTest<ScnDeactivationProcedure>();
    }

    Q_INVOKABLE void NObjIsdnTestRunner::BeginReactivationL3Test()
    {
        CreateTest<ReActivationL3Test>();
    }

    Q_INVOKABLE void NObjIsdnTestRunner::BeginWrongL3PacketTest()
    {
        CreateTest<WrongL3PacketTest>();
    }

    Q_INVOKABLE void NObjIsdnTestRunner::BeginRestartTest()
    {
        CreateTest<ScnRestartProcedure>();
    }

    // ------------------------------------------------------------------------------------

    Q_INVOKABLE void NObjIsdnTestRunner::Run( DRI::IAsyncCmd *pAsyncCmd )
    {
        AsyncBegin(pAsyncCmd, boost::bind(&TMy::AbortTest, this));

        if (!m_test) AsyncComplete(false, "No test assigned");

        EnableThreadHook();

        m_test->Run(*this);

        LogDri( QString("Test %1 started...").arg(m_test->TestName()) );
    }

    // ------------------------------------------------------------------------------------
    // Internal methods:

    void NObjIsdnTestRunner::LogDri( QString msg )
    {
        AsyncOutput(msg);
        AsyncFlush();
    }

    // ------------------------------------------------------------------------------------

    void NObjIsdnTestRunner::AbortTest() /* esc prc */
    {
        if (m_test) m_test->AbortTest();
        AsyncComplete(false, "Aborted by user");
    }

    // ------------------------------------------------------------------------------------    

    struct NObjIsdnTestRunner::TestResult
    {
        TestResult(bool ok, QString info) : Ok(ok), Info(info)
        {}
        bool Ok;
        QString Info;
    };

    // ------------------------------------------------------------------------------------

    void NObjIsdnTestRunner::OnTestFinished( const TestResult& result )
    {
        m_test.reset();
        DisableThreadHook();
        AsyncComplete(result.Ok, result.Info);
    }

    // ------------------------------------------------------------------------------------

    void NObjIsdnTestRunner::TestFinished( bool ok, QString info )
    {
        PutMsg(this, &TMy::OnTestFinished, TestResult(ok, info) );
    }

    // ------------------------------------------------------------------------------------

    void NObjIsdnTestRunner::PrintInfo( QString info )
    {
        AsyncOutput(info);
        AsyncFlush();
    }   
   
} // namespace IsdnTest
