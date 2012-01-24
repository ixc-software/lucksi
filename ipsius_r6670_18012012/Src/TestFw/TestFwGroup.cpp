#include "stdafx.h"
#include "TestFw/TestFwGroup.h"
#include "TestFw/TestFw.h"
#include "TestFw/TestFwOutput.h"
#include "TestFw/TestFwGroupExt.h"

// ----------------------------------------------

using namespace TestFw;

namespace
{
    const bool CAddTestsWithExceptions = false;

    int GCounter;

    void TestFn1()
    {
        GCounter++;
    }

    void TestFn2(int val)
    {
        GCounter += val;
    }

    void TestFn3(const char *p)
    {
        GCounter++;
    }

    class Group : public TestGroup<Group>
    {
        int &m_i;
        const char *m_pString;

        void TestFn4()
        {
            m_i++;
        }

        void TestFn5(int val)
        {
            m_i += val;
        }

        void TestStopWithTUT()
        {
            TUT_ASSERT(0 && "Some throuble");
        }
        
        ESS_TYPEDEF(TestException);
        
        void TestStopByThrow()
        {
            ESS_THROW(TestException);
        }

        void OnFullGroupCompleted()  // override
        {
            ESS_ASSERT(GCounter == 5); 
        }

        template<class TGroup>
        friend void RegisterExtendedTests(TGroup &g);

        Group(ILauncher &launcher) 
            : TestGroup<Group>(launcher, "TestFwSelf"), 
            m_i(GCounter)
        {
            GCounter = 0;
            m_pString = "Zoo";

            if (CAddTestsWithExceptions)
            {
                Add("TUT assert",                   &T::TestStopWithTUT);
                Add("Exception throw",              &T::TestStopByThrow);            
            }
            Add("Simple method",                &TestFn1);
            Add("Method with one param",        &TestFn2, 1);
            Add("Method with one param ptr",    &TestFn3, m_pString);
            Add("Method of object",             &T::TestFn4);
            Add("Method of object with param",  &T::TestFn5, 1);

            // other tests
            Add("TestFwOutputtersList test", &TestFwOutputtersListTest);

            // register platform specific tests
            RegisterExtendedTests(*this);
        }

    public:

        ~Group()
        {
            // ESS_ASSERT(m_verifyCalled);
        }

        static void AddToLauncher(ILauncher &launcher)
        {
            new Group(launcher);  // free at launcher
        }
    };

}  // namespace


// ----------------------------------------------

namespace TestFw
{

    void RegisterTestGroup(ILauncher &launcher)
    {
        Group::AddToLauncher(launcher);
    }

    void SelfTest()
    {
        TestFw::ArgStringList sl;
        TestFw::StdOutput output;
        
        TestFw::TestLauncher launcher(output);

        // TestVerification testVer;
        // launcher.AddOutputter(testVer);

        RegisterTestGroup(launcher);

        launcher.Run(sl);
    }
    
}  // namespace TestFw
