
#include "stdafx.h"

#include "FormatTypeidNameTest.h"
#include "Platform/Platform.h"
#include "Utils/ErrorsSubsystem.h"

namespace Namespace_Name
{
    class Class12Name{
    public:
        class InnerClass13E {};
    };

    // ------------------------------

    class ClassWithEnum
    {
    public:
        enum States
        {
            First,
            Second,
        };
    };
}

// ------------------------------

class NonNamespaceClassE
{
public:
    struct InnerStruct {};
    
};

// ------------------------------

enum NonNamespaceState
{
    Init = 1,
};

// ------------------------------

namespace
{
    bool NamesCompare(const char *pClassName, const char *pClassTypeIDName)
    {
        std::string s1(pClassName);
        std::string s2 = Platform::FormatTypeidName(pClassTypeIDName);

        // std::cout << "in -- " << s1 << std::endl << "out-- " << s2 << "\n\n";
        
        return (s1 == s2);
    }

    #define DO_COMPARE(_name) TUT_ASSERT( NamesCompare(#_name, typeid(_name).name()) )
}

// ------------------------------

namespace PlatformTests
{
    void FormatTypeidNameTest()
    {
        // namespace::class
        DO_COMPARE(Namespace_Name::Class12Name::InnerClass13E);
        DO_COMPARE(Namespace_Name::ClassWithEnum::States);
        DO_COMPARE(Namespace_Name::Class12Name*);

        // class
        DO_COMPARE(NonNamespaceClassE);
        DO_COMPARE(NonNamespaceClassE::InnerStruct);
        DO_COMPARE(NonNamespaceClassE*);
        DO_COMPARE(NonNamespaceClassE::InnerStruct*);

        // enum
        DO_COMPARE(NonNamespaceState);
        
        // other
        DO_COMPARE(bool);
        DO_COMPARE(int);
        DO_COMPARE(std::string);
    }

} // namespace PlatformTests


