
#ifndef __TESTFWOUTPUTTERSLIST__
#define __TESTFWOUTPUTTERSLIST__

// TestFwOutputtersList.h

#include "stdafx.h"
#include "ITestFwOutputStream.h"
#include "Utils/ErrorsSubsystem.h"
#include "Utils/VirtualInvoke.h"

namespace TestFw
{
    using Utils::VIExecuter;

    // List of ITestFwOutputStream.
    // Make possible to run methods for all outputters in list.
    // Do not delete pOutputters
    class TestFwOutputtersList
    {
        typedef TestFwOutputtersList T;
                
        std::vector<ITestFwOutputStream*> m_pOutputers;

        int Find(ITestFwOutputStream* pOutputter)
        {
            for (size_t i = 0; i < m_pOutputers.size(); ++i)
            {
                if (m_pOutputers.at(i) == pOutputter) return i;
            }

            return -1;
        }

    public:
        TestFwOutputtersList()
        {
        }

        void Add(ITestFwOutputStream* pOutputter)
        {
            // should not be empty
            ESS_ASSERT(pOutputter != 0);
            // check dublicats
            ESS_ASSERT(Find(pOutputter) < 0);

            m_pOutputers.push_back(pOutputter);
        }

        template<class TParam>
        void RunMethodForEach(void (ITestFwOutputStream::*pFn)(TParam&), TParam &param)
        {
            for (size_t i = 0; i < m_pOutputers.size(); ++i)
            {
                VIExecuter::Exec( Utils::VirtualInvoke::Create(pFn, *m_pOutputers.at(i), 
                                                             param) );
            }
        }

        template<class TParam1, class TParam2>
        void RunMethodForEach(void (ITestFwOutputStream::*pFn)(TParam1&, const TParam2&), 
                              TParam1 &param1, const TParam2 &param2)
        {
            for (size_t i = 0; i < m_pOutputers.size(); ++i)
            {
                VIExecuter::Exec( Utils::VirtualInvoke::Create(pFn, *m_pOutputers.at(i), 
                                                             param1, param2) );
            }
        }

        size_t Size() const { return m_pOutputers.size(); }
    };

    // -----------------------------------------

    void TestFwOutputtersListTest();
    
} // namespace TestFw

#endif
