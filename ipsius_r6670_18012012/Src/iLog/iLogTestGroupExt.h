#ifndef __ILOGTESTGROUPEXT__
#define __ILOGTESTGROUPEXT__

/*
     Special file, platform specific
     Include only in iLogTestGroup.cpp for add extended PC test
*/

namespace
{
	template<class TGroup>
    void RegisterExtendedTests(TGroup &g)
    {
        g.Add("iLogCommon impl", &iLog::iLogTest2, false);
    }
}

#endif

