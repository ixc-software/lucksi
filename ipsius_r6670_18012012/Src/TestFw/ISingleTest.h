
#ifndef __ISINGLETEST__
#define __ISINGLETEST__

#include "Utils/IBasicInterface.h"
#include "Utils/ErrorsSubsystem.h"

// ISingleTest.h

namespace TestFw
{
    class ITestGroup;

    // single test interface
    class ISingleTest : public Utils::IBasicInterface
    {
    public:
        virtual void Run() = 0;
        virtual const std::string& getName() const = 0;
        virtual int getTestNumber() = 0;
        virtual std::string getGroupName() const = 0;
        virtual ITestGroup& getGroup() = 0;
    };

    // -----------------------------------------------------

    class SingleTestDummi : public ISingleTest
    {
        const std::string m_name;

    // ISingleTest impl
    private:

        void Run() { ESS_UNIMPLEMENTED; }
        const std::string& getName() const { return m_name; }
        int getTestNumber() { return 0; }
        std::string getGroupName() const { return ""; }
        ITestGroup& getGroup() { ESS_UNIMPLEMENTED; return *((ITestGroup*)0); }

    public:

        SingleTestDummi() {}

    };
    
} // namespace TestFw 

#endif
