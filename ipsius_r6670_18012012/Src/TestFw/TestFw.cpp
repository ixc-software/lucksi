#include "stdafx.h"
#include "TestFw/TestFw.h"
#include "TestFw/TestResult.h"

using namespace std;

namespace 
{
    string MakeErrMsg(const string &err, const string &groupName, const string &testName)
    {
        string res = "TestFw raised exception";
        if ( !groupName.empty() ) res += string(" in group \'") + groupName + string("\' ");
        if ( !testName.empty() ) res += string("in test \'") + testName + string("\'");
        res += string("\n") + err;

        return res;
    }
    
} // namespace  

// --------------------------------------

namespace TestFw
{
    const std::string CProjName("ProjAllTests");
    
    const std::string CCmdAll("all");
    const std::string CCmdList("list");
    const std::string CCmdListFull("listfull");

    // ---------------------------------------------------

    void TestLauncher::ProcessSingleParam(const std::string &param)
    {
        if (param == CCmdAll)
        {
            Utils::VIMethod<T> fn(&T::RunAll, *this);
            Run(fn);
            return;
        }

        if (param == CCmdList)
        {
            ListGroups(true);
            return;
        }

        if (param == CCmdListFull)
        {
            ListGroups(false);
            return;
        }

        // try to run group by name
        ITestGroup *pGroup = FindByName(param);
        if (pGroup == 0)
        {
            cout << "Group " << param << " not found" << endl;
        }
        else
        {
            Utils::VIMethodWithParam<T, ITestGroup*> fn(&T::RunAllInGroup, *this, pGroup);
            Run(fn);
        }
    }

    // ---------------------------------------------------

    void TestLauncher::ProcessTwoParams(const std::string &group, const std::string &name)
    {
        // find group
        ITestGroup *pGroup = FindByName(group);
        if (pGroup == 0)
        {
            cout << "Group " << group << " not found" << endl;
            return;
        }

        // find test
        ISingleTest *pTest = 0;
        int number = std::atoi(name.c_str());
        if (number != 0)
        {
            int indx = number - 1;
            if ((indx >= 0) && (indx < pGroup->getTestsCount())) pTest = pGroup->getTest(indx);
        }
        else
        {
            pTest = pGroup->FindByName(name);
        }

        // test not found
        if (pTest == 0)
        {
            cout << "Test '" << name << "' not found" << endl;
            return;
        }

        // run
        Utils::VIMethodWithParam<T, ISingleTest*> fn(&T::RunSingleTestInGroup, *this, pTest);
        Run(fn);
    }

    // ---------------------------------------------------

    void TestLauncher::Run(const ArgStringList &arg)
    {
        int count = arg.size();

        try
        {
            if (count == 0)
            {
                Utils::VIMethod<T> fn(&T::RunAll, *this);
                Run(fn);
                return;
            }

            if (count == 1)
            {
                ListHelp();
                return;
            }

            if (count == 2)
            {
                std::string param = arg.at(1);
                ProcessSingleParam(param);
                return;
            }

            if (count == 3)
            {
                ProcessTwoParams(arg.at(1), arg.at(2));
            }
            else
            {
                ListHelp();
            }
        }
        catch (/*const*/ VerificationException &ex)
        {
            string msg = MakeErrMsg(ex.getTextMessage(), m_executedGroupName, 
                                    m_executedTestName);
            ESS_HALT(msg);
        }
        catch (/*const*/ exception& ex)
        {
            string msg = MakeErrMsg(ex.what(), m_executedGroupName, 
                                    m_executedTestName);
            ESS_HALT(msg);
        }
    }

    // ---------------------------------------------------

    void TestLauncher::ListHelp()
    {
        cout << "Usage for " << CProjName << ": " << endl;
        cout << "  Run all tests: "    << CProjName << " " << CCmdAll << endl;
        cout << "  List all groups: "  << CProjName << " " << CCmdList << endl;
        cout << "  List all groups and tests: "  << CProjName << " " << CCmdListFull << endl;
        cout << "  Run one group: "    << CProjName << " 'GroupName'" << endl;
        cout << "  Run one test: "     << CProjName << " 'GroupName' 'TestNumber'/'TestName'" << endl;
        cout << endl;
    }

    // ---------------------------------------------------

    void TestLauncher::ListGroups(bool shortForm)
    {
        for(size_t i = 0; i < m_tests.size(); ++i)
        {
            ITestGroup *p = m_tests.at(i);

            cout << p->getName() << " (tests " <<  p->getTestsCount() << ")" << endl;

            if (!shortForm)
            {
                for(int j = 0; j < p->getTestsCount(); ++j)
                {
                    cout << "    #" << (j + 1) << " \"" <<  p->getTest(j)->getName() << "\"" << endl;
                }
            }
        }            
    }

    // ---------------------------------------------------

    void TestLauncher::RunSingleTest( ISingleTest &test )
    {
        TestResult result(&test);

        m_outputters.RunMethodForEach(&ITestFwOutputStream::TestStarted, test);
        
        try
        {   
            m_executedGroupName = test.getGroup().getName();
            m_executedTestName = test.getName();
            
            test.Run();
        }
        catch(std::exception &e)
        {
            result.AbortedByException(e);
        }

        m_outputters.RunMethodForEach(&ITestFwOutputStream::TestCompleted, test, result);

        m_executedGroupName.clear();
        m_executedTestName.clear();
    }

    // ---------------------------------------------------

    void TestLauncher::RunAllInGroup(ITestGroup *pGroup)
    {
        RunGroup(pGroup, 0);
    }

    // ---------------------------------------------------

    void TestLauncher::RunGroup(ITestGroup *pGroup, ISingleTest *pTest)
    {
        ESS_ASSERT(pGroup != 0);

        m_outputters.RunMethodForEach(&ITestFwOutputStream::GroupStarted, *pGroup);
        
        if (pTest == 0)
        {
            for(int i = 0; i < pGroup->getTestsCount(); ++i)
            {
                ISingleTest *pTest = pGroup->getTest(i);
                RunSingleTest(*pTest);
            }

            pGroup->OnFullGroupCompleted();
        }
        else
        {
            RunSingleTest(*pTest);            
        }

        m_outputters.RunMethodForEach(&ITestFwOutputStream::GroupCompleted, *pGroup);
    }

    // ---------------------------------------------------

    void TestLauncher::RunSingleTestInGroup(ISingleTest *pTest)
    {
        RunGroup(&(pTest->getGroup()), pTest);
    }

}  // namespace TestFw

