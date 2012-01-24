#ifndef __ICOREOVERLOAD__
#define __ICOREOVERLOAD__

namespace iCoreTests
{
	    
	void TestBlockedMsgHalt();

    void TestMaxMsgInThreadHalt();
    void TestMaxTimerLag();
    void TestMsgGrow();
    
    template<class T>
    class DebugPrint
    {
    public:
        static void Print(T val);
    }; 
	
}  // namespace iCoreTests

#endif
