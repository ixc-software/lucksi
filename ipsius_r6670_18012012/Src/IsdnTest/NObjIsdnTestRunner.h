#ifndef NOBJISDNTESTRUNNER_H
#define NOBJISDNTESTRUNNER_H

//#include "Utils/IExecutor.h"
#include "stdafx.h"
#include "Utils/IBasicInterface.h"

#include "Domain/NamedObject.h"

#include "ITestResult.h"

namespace IsdnTest
{    
    class ITestShell : public Utils::IBasicInterface
    {
    public:
        virtual void Run(ITestResult& callback) = 0;
        virtual void AbortTest() = 0;
        virtual QString TestName() = 0;
    };

    

    class NObjIsdnTestRunner 
        : public Domain::NamedObject,
        public ITestResult
    {
        Q_OBJECT;
        typedef NObjIsdnTestRunner TMy;

        boost::scoped_ptr<ITestShell> m_test;

        template<class T>
        void CreateTest();

        void LogDri(QString msg);
        void AbortTest();
        struct TestResult;        
        void OnTestFinished(const TestResult& result);

    // overrides:
    private:
        bool SuppressDomainExceptionHook(const std::exception &e)
        {
            const ESS::TutException* pE = dynamic_cast<const ESS::TutException*>(&e);

            if (pE)
            {            
                TestFinished(false, pE->getTextAndPlace().c_str());
                return true;
            }            

            return false;
        }

    // ITestResult
    private:
        void TestFinished(bool ok, QString info);        
        void PrintInfo(QString info);
        
    // DRI interface:
    public:
        NObjIsdnTestRunner(Domain::IDomain *pDomain, const Domain::ObjectName &name)
            : NamedObject(pDomain, name)
        {}

        //Sample use:
        //  .BeginXXX()
        //  .Profile.XXX.X = ... // optional
        //  .Run


        Q_INVOKABLE void BeginLapdTest();
        Q_INVOKABLE void BeginCheckBchannelsBusy();
        Q_INVOKABLE void BeginCheckConnectionDisconnectionRoutine();
        Q_INVOKABLE void BeginCheckReloadDrv();
        Q_INVOKABLE void BeginCheckReloadLapd();
        Q_INVOKABLE void BeginCheckReloadUser();
        Q_INVOKABLE void BeginSimulLiveTest();
        Q_INVOKABLE void BeginDeactivationTest();
        Q_INVOKABLE void BeginReactivationL3Test();
        Q_INVOKABLE void BeginWrongL3PacketTest();
        Q_INVOKABLE void BeginRestartTest();

        //... other tests

        Q_INVOKABLE void Run(DRI::IAsyncCmd *pAsyncCmd);        
    };

} // namespace IsdnTest

#endif
