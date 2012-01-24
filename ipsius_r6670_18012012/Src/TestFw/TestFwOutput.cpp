#include "stdafx.h"
#include "TestFwOutput.h"
#include "TestFw.h"

// ------------------------------------

namespace TestFw
{

    std::string TestResult::getFullInfo()
    {
        std::ostringstream ss;

        ss << "---> " << "group: " << Test().getGroupName()
            << ", test: #" << Test().getTestNumber() << " '" << Test().getName() << "'"
            << std::endl;

        if (!m_testOK)
        {
            ss << "Exception typeid: " << m_exceptionTypeID << std::endl;
            ss << "Exception message: " << m_exceptionMsg << std::endl;
        }

        return ss.str();
    }

    // -------------------------------------------------------------------
	
    void StdOutput::RunStarted(TestLauncher &launcher)
    {
        Init();
    }

    // -------------------------------------------------------------------

    void StdOutput::RunCompleted(TestLauncher &launcher)
    {
        if (m_resultList.size() > 0)
        {
            m_os << std::endl;
            PrintExceptionsList();
        }

        m_os << std::endl;

        m_os << "tests summary:";
        if (m_exceptionsCount > 0)
        {
            m_os << " exceptions:" << m_exceptionsCount;
        }

        m_os << " ok:" << m_okCount << std::endl;
    }

    // -------------------------------------------------------------------

    void StdOutput::GroupStarted(ITestGroup &group)
    {
        m_os << group.getName() << ": ";
    }

    // -------------------------------------------------------------------

    void StdOutput::GroupCompleted(ITestGroup &group)
    {
        m_os << std::endl;
    }

    // -------------------------------------------------------------------

    void StdOutput::TestStarted(ISingleTest &test)
    {
        // nothing
    }

    // -------------------------------------------------------------------

    void StdOutput::TestCompleted(ISingleTest &test, const TestResult &result)
    {
        if (result.TestOK())
        {
            m_os << "." << std::flush;
            m_okCount++;
            return;
        }

        m_os << "[" << test.getTestNumber() << "=X]" << std::flush;
        m_exceptionsCount++;
        m_resultList.push_back(result);
    }

    // -------------------------------------------------------------------

    void StdOutput::PrintExceptionsList()
    {
        ResultList::iterator i = m_resultList.begin();

        while (i != m_resultList.end())
        {
            m_os << (*i).getFullInfo() << std::endl;

            ++i;
        }
    }

}  // namespace TestFw

