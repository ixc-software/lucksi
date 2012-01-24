#ifndef __TESTFWGROUPEXT__
#define __TESTFWGROUPEXT__

/*
     Special file, platform specific
     Include only in TestFwGroup.cpp for add extended PC test
*/

#include "TestFw/TestVerification.h" 

namespace
{
	template<class TGroup>
    void RegisterExtendedTests(TGroup &g)
    {
        // g.Add("iLogCommon impl", &iLogTest2, false);
        
        g.Add("TestVerification tests", &TestFw::TestVerification::SelfTests);        
    }
}

#endif

