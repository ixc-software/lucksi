#include  "stdafx.h"

#include "SafeRefServerTest.h"
#include "Utils/SafeRef.h"
#include "Utils/ErrorsSubsystem.h"

namespace
{
    class TestObj : public Utils::SafeRefServer {};

} // namespace

// -----------------------------

namespace UtilsTests
{
    using namespace Utils;
    
    void SafeRefServerTest()
    {
        using namespace Utils; 
        using namespace ESS;
    
        ExceptionHookRethrow<> hook;
    
        TestObj* pTObj2 = new TestObj;
        try
        {
            TestObj tObj1;
            SafeRef<TestObj> sr1(&tObj1);
            SafeRef<TestObj> sr2(pTObj2);
            {
                SafeRef<TestObj> sr3(&tObj1);
            }
    
            sr1 = sr2; // test copy assigment
    
        } // destroy tObj1 - test ok && destroy all ref to pTObj2
        catch(HookRethrow &e)
        {
            TUT_ASSERT(0 && "BAD ~SafeRef()");
        }
    
        char buf[sizeof(SafeRef<TestObj>)];
        SafeRef<TestObj>* pSr4;
        {
            SafeRef<TestObj> sr5(pTObj2);
            pSr4 = new(buf) SafeRef<TestObj>(sr5); // test copy constructor
        }
    
        bool wasException = false;
        try
        {
            delete pTObj2; // Assertion!
        }
        catch(HookRethrow &e)
        {
            wasException = true;
        }
    
        TUT_ASSERT(wasException);
    }
    
} // namespace UtilsTests
