
#ifndef __ITESTGROUP__
#define __ITESTGROUP__

// ITestGroup.h

#include "Utils/IBasicInterface.h"
#include "ISingleTest.h"

namespace TestFw
{
    // test group interface
    class ITestGroup : public Utils::IBasicInterface
    {
    public:
        virtual const std::string& getName() const = 0;
        virtual int getTestsCount() const = 0;
        virtual ISingleTest* getTest(int index) = 0;
        virtual ISingleTest* FindByName(const std::string &testName) = 0;

        // events
        virtual void OnFullGroupCompleted() = 0;
    };
    
} // namespase TestFw

#endif
