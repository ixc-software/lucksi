
#ifndef __TESTFW_TESTRESULT__
#define __TESTFW_TESTRESULT__

#include "Utils/ErrorsSubsystem.h"

namespace TestFw
{
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

} // namespace TestFw

#endif
