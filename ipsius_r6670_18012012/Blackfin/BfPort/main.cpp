// BfPort.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#include "Utils/ErrorsSubsystem.h"
#include "Utils/ErrorsSubsystemTests.h"
#include "Utils/VirtualInvokeTest.h"
#include "Utils/BidirBuffer.h"
#include "Utils/MemoryPool.h"
#include "Utils/TimerBase.h"
#include "Utils/DateTimeCapture.h"
#include "Utils/DeleteIfPointer.h"

#include "TestFw/TestFwGroup.h"
#include "TestFw/TestFwOutput.h"
#include "TestFw/TestFw.h"

#include "iCore/MsgObject.h"
#include "iCore/MsgThread.h"
#include "iCore/MsgTimer.h"
#include "iCore/ThreadRunner.h"

#include "iLog/LogTest.h"

#include "RecursiveThrow.h"
#include "SpecializationTest.h"
#include "RefTest.h"

// ------------------------------------------

ESS_TYPEDEF(RecursiveException);

// ------------------------------------------

template<class TIter, class TVal>
TIter RangeFind(TIter begin, TIter end, TVal val)
{
    while(begin != end)
    {
        if (*begin == val) return begin;
    }

    return end;
}

class X
{
    typedef std::vector<int> List;

    List m_data;

    List::const_iterator Find(int val) const
    {
        return RangeFind(m_data.begin(), m_data.end(), val);
    }

    List::iterator Find(int val)
    {
        return RangeFind(m_data.begin(), m_data.end(), val);        
    }

public:

    void Fn(int x)
    {
        List::iterator i = Find(x);
    }

    void FnConst(int x) const
    {
        List::const_iterator i = Find(x);
    }

};

// ------------------------------------------
// exceptions payload test

namespace
{
    volatile bool GDoThrow = false;
    volatile int  GDestructionCounter;

    class ConstructorCanThrow
    {
    public:
        ConstructorCanThrow()
        {
            if (GDoThrow) throw std::exception();
        }

        ~ConstructorCanThrow()
        {
            GDestructionCounter++;
        }
    };

    void ExceptionsPayloadTest()
    {
        ConstructorCanThrow a;
        ConstructorCanThrow b;
        ConstructorCanThrow c;
    }

}

// ------------------------------------------

namespace
{
    void TestStream()
    {
        std::string s;

        {
            std::ostringstream ss;
            ss << "123";
            ss.flush();
            ss.str("");

            s = ss.str();
        }

        int i = 10;
    }
}

// ------------------------------------------

/*
struct A {};
struct I1 : public  A {};
struct I2 : public  A {};
struct D : public I1, public I2 {};

A *foo( D *p ) 
{
    return (A*)( p ); // ill-formed static_cast interpretation
}
*/

/*
namespace XXX
{
    class A {};
    class B {};

    B* foo(A *pA)
    {
        return static_cast<B*>(pA);
    }
} */

// ------------------------------------------

void RunTestFW(const TestFw::StringList &sl)
{
    // test framework
    TestFw::StdOutput output;

    TestFw::TestLauncher launcher(output);

    {
        TestFw::RegisterTestGroup(launcher);
        UtilsTests::ESSTests::RegisterTestGroup(launcher);
        iLog::RegisterTestGroup(launcher);
        // ...
    }

    launcher.Run(sl);
}

// ------------------------------------------

class SomeType;

class Trouble
{
    SomeType &m_ref;

    Trouble(SomeType &ref) : m_ref(m_ref)  
    {}
};

// ------------------------------------------

int main(int argc, char *argv[])
{
    Utils::DeletePointerTest();

    // RunTestFW( TestFw::StringList(argc, argv) );

    // Utils::DtCaptureVisualCheck();

    // iLog::iLogTestWrap();

    /*
    iLog::iLogWrapperSimpleTest(true);
    iLog::iLogWrapperBenchmark(true);
    iLog::iLogWrapperBenchmarkAsTest(); */

	return 0;
}

