#ifndef __TESTFWOUTPUT__
#define __TESTFWOUTPUT__

#include "Utils/ErrorsSubsystem.h"
#include "ITestFwOutputStream.h"
#include "TestResult.h"

namespace TestFw
{
    /*
    class TestLauncher;
    class ITestGroup;
    class ISingleTest;

    
    // результат выполнения теста
    class TestResult
    {
        ISingleTest *m_pTest;
        bool m_testOK;
        std::string m_exceptionMsg;
        std::string m_exceptionTypeID;

    public:

        TestResult(ISingleTest *pTest = 0) : m_pTest(pTest), m_testOK(true) {}

        void AbortedByException(const std::exception &e)
        {
            m_testOK = false;
            m_exceptionMsg = e.what();
            m_exceptionTypeID = typeid(e).name();
        }

        bool TestOK() const 
        { 
            return m_testOK; 
        }

        const std::string& ExceptionMsg() const    
        { 
            ESS_ASSERT(!m_testOK);
            return m_exceptionMsg; 
        }

        const std::string& ExceptionTypeID() const 
        { 
            ESS_ASSERT(!m_testOK);
            return m_exceptionTypeID; 
        }

        ISingleTest& Test() 
        { 
            ESS_ASSERT(m_pTest != 0);
            return *m_pTest; 
        }

        std::string getFullInfo();
    };

    // ------------------------------------------------------------

    // output interface
    class IOutputStream : public Utils::IBasicInterface
    {
    public:

        
        virtual void RunStarted(TestLauncher &launcher) = 0;
        virtual void RunCompleted(TestLauncher &launcher) = 0;
        
        virtual void GroupStarted(ITestGroup &group) = 0;
        virtual void GroupCompleted(ITestGroup &group) = 0;
        
        virtual void TestStarted(ISingleTest &test) = 0;
        virtual void TestCompleted(ISingleTest &test, const TestResult &result) = 0;
    };
    */

    // ------------------------------------------------------

    // default output interface impl for std::cout
    class StdOutput : public ITestFwOutputStream, boost::noncopyable
    {
        typedef std::vector<TestResult> ResultList;

        std::ostream &m_os;
        ResultList m_resultList;
        int m_okCount;
        int m_exceptionsCount;

        void Init()
        {
            m_resultList.clear();
            m_okCount = 0;
            m_exceptionsCount = 0;
        }

        void PrintExceptionsList();

    // ITestFwOutputStream impl
    private:

        void RunStarted(TestLauncher &launcher);
        void RunCompleted(TestLauncher &launcher);        
        void GroupStarted(ITestGroup &group);
        void GroupCompleted(ITestGroup &group);
        void TestStarted(ISingleTest &test);
        void TestCompleted(ISingleTest &test, const TestResult &result);

    public:

        StdOutput() : m_os(std::cout)
        {
            Init();
        }
    };

		
}  // namespace TestFw

#endif

