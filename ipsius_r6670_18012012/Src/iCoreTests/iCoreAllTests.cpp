#include "stdafx.h"

#include "iCore/MsgThread.h"
#include "iCore/SyncTimer.h"
// #include "Utils/TestLog.h"


#include "iCoreSimpleMsgTest.h"
#include "iCoreMsgBenchmarks.h"
#include "iCoreSynchronSendingMsgTest.h"
#include "iCoreTimerTest.h"
#include "iCoreTimerTestAdvanced.h"
#include "iCoreThreadPoolTest.h"
#include "iCoreThreadPoolTestA.h"
#include "iCoreAsyncTests.h"
#include "iCoreMsgObjectSafeDelete.h"

// ----------------------------------------------

namespace
{
    // Utils::TestLog Log("MsgBenchmarks.txt");

    enum { CBenchTime = 50 };

    void XLog(std::string s)
    {
        std::cout << s << std::endl;
    }

}  // namespace

// ---------------------------------

namespace iCoreTests
{
    void Big_iCoreAllTestsRun()
    {
        {
            iCore::MsgThread m("Big_iCoreAllTestsRun");
            Platform::Thread::Sleep(100);
        }
    
        // XLog("iCoreMsgObjectSafeDelete");
        // iCoreMsgObjectSafeDelete();  
    
        XLog("iCoreTimerTest");
        iCoreTimerTest();
    
        //XLog("iCoreSignatureCheck");
        //iCoreSignatureCheck();
    
        XLog("iCoreAsyncTests");
        iCoreAsyncTests();
    
        XLog("iCoreSimpleMsgTest");
        iCoreSimpleMsgTest();
    
        {
            XLog("Benchmarks");
            XLog(iCoreBenchmarkSingleN(CBenchTime, 1));  
            XLog(iCoreBenchmarkMulti(CBenchTime));    
            XLog(iCoreBenchmarkSingleN(CBenchTime, 2));
        }
    
        XLog("iCoreSynchronSendingMsgTest");
        iCoreSynchronSendingMsgTest();  // here lost in destructor
    
        XLog("iCoreTimerAdvancedTest");
        iCoreTimerAdvancedTest(); 
    
        XLog("iCoreThreadPoolTest");
        iCoreThreadPoolTest();
    
        XLog("iCoreThreadPoolTestA");
        iCoreThreadPoolTestA(); // true = show info
    
        // iCore::MsgObjectLinkTest(true);
    
    }
    
    void Small_iCoreAllTestsRun()
    {
        int i = 0;
    
        while(true)
        {
            std::stringstream ss;
            ss << i++ << " iCoreSynchronSendingMsgTest";
            XLog(ss.str());
            iCoreSynchronSendingMsgTest();  // here lost in destructor
        }
    }
    
    
    void iCoreAllTestsRun()
    {
        // Small_iCoreAllTestsRun();
        Big_iCoreAllTestsRun();
    }

} // namespace iCoreTests
