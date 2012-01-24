#ifndef __TESTFWGROUP__
#define __TESTFWGROUP__

#include "TestFw/TestFw.h"

namespace TestFw
{

    // full self-test
	void SelfTest();	

    // register self-test in launcher
    void RegisterTestGroup(ILauncher &launcher);
	
}  // namespace TestFw

#endif

