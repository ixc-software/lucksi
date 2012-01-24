
#ifndef __ITESTFWOUTPUTSTREAM__
#define __ITESTFWOUTPUTSTREAM__

// ITestFwOutputStream.h

#include "TestResult.h"
#include "ITestGroup.h"
#include "ISingleTest.h"

#include "Utils/IBasicInterface.h"

namespace TestFw
{
    class TestLauncher;
    /*
    class ITestGroup;
    class ISingleTest;
    class TestResult;
    */
    
    // output interface
    class ITestFwOutputStream : public Utils::IBasicInterface
    {
    public:
        virtual void RunStarted(TestLauncher &launcher) = 0;
        virtual void RunCompleted(TestLauncher &launcher) = 0;
        
        virtual void GroupStarted(ITestGroup &group) = 0;
        virtual void GroupCompleted(ITestGroup &group) = 0;
        
        virtual void TestStarted(ISingleTest &test) = 0;
        virtual void TestCompleted(ISingleTest &test, const TestResult &result) = 0;
    };
    
} // namespace TestFw


#endif
