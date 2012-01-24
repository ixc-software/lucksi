#ifndef __BFWATCHDOG__
#define __BFWATCHDOG__

namespace BfDev
{
    
    class BfWatchdog
    {

    public:
        
        BfWatchdog(int timeoutMs) {}
        ~BfWatchdog() {}
        
        void Reset() {}
        
        static void SwReset() {}
                
    };
    
    
}   // namespace BfDev


#endif
