#ifndef __LOGTEST__
#define __LOGTEST__

#include "stdafx.h"
#include "TestFw/TestFw.h" 

namespace iLog
{

    // simply test for iLogWStr impl
	void iLogTest();  

    // advanced test for iLogCommon impl
    void iLogTest2(bool useCout);  

    // tests for wrapper
    void iLogWrapperSimpleTest(bool useCout);
    void iLogWrapperBenchmark(bool useCout);
    void iLogWrapperBenchmarkAsTest();

    // register test group
    void RegisterTestGroup(TestFw::ILauncher &launcher);

}

#endif

