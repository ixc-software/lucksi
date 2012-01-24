
#include "stdafx.h"
#include "TestFwOutputtersList.h"

namespace 
{
    using namespace TestFw;
    
    class TestOutput : public ITestFwOutputStream
    {
        int m_calls;
        
    // ITestFwOutputStream impl
    private:
        void GroupCompleted(ITestGroup &group) {}
        void GroupStarted(ITestGroup &group) {}
        void RunCompleted(TestLauncher &launcher) {}
        void RunStarted(TestLauncher &launcher) {}
        
        void TestCompleted(ISingleTest &test, const TestResult &result)
        {
            ++m_calls;
        }
        
        void TestStarted(ISingleTest &test)
        {
            ++m_calls;
        }

    public:
        TestOutput() : m_calls(0) {}

        int getCalls() const { return m_calls; }
    };

    // -----------------------------------------------

    class OutputterListTest
    {
        TestOutput m_t;
        TestOutput *m_pT;
        
    public:
        OutputterListTest()
        : m_pT(new TestOutput)
        {
            TestFwOutputtersList list;

            // test adding
            list.Add(&m_t);
            list.Add(m_pT);

            /*
            list.Add(&m_t); // shouldn't work(dublicate)
            TestOutput *pZero = 0;
            list.Add(pZero); // shouldn't work(zero ptr)
            */

            // test running methods
            SingleTestDummi test;
            ISingleTest *pTest = &test; // dummi
            TestResult res;             // dummi
            
            list.RunMethodForEach(&ITestFwOutputStream::TestStarted, *pTest);
            TUT_ASSERT(m_t.getCalls() == 1);
            TUT_ASSERT(m_pT->getCalls() == 1);
            
            list.RunMethodForEach(&ITestFwOutputStream::TestCompleted, *pTest, res);
            TUT_ASSERT(m_t.getCalls() == 2);
            TUT_ASSERT(m_pT->getCalls() == 2);
        }

        ~OutputterListTest()
        {
            delete m_pT;
        }
        
    };
    
} // namespace

// -----------------------------------------------

namespace TestFw
{
    void TestFwOutputtersListTest()
    {
        OutputterListTest test;
    }
    
} // namespace TestFw
