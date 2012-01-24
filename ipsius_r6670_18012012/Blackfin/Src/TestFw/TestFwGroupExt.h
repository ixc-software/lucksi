#ifndef __TESTFWGROUPEXT__
#define __TESTFWGROUPEXT__

/*
     Special file, platform specific
     Include only in TestFwGroup.cpp for add extended PC test
*/


namespace
{
	template<class TGroup>
    void RegisterExtendedTests(TGroup &g)
    {
    	// no special tests for Blackfin
    }
}

#endif

