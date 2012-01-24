#ifndef __BFUARTSIMPLECORE__
#define __BFUARTSIMPLECORE__

#include "Platform/PlatformTypes.h"

namespace BfDev
{
    using Platform::byte;
    using Platform::word;        
    using Platform::dword;
            
    class BfUartSimpleCore
    {
        
    public:
        
        BfUartSimpleCore(dword sysFreq, byte portNum, dword bitrate)
        {
  	        ESS_ASSERT(portNum < 2);                        
        }
        
        bool ReadyToSend()
        {        	
        	return true;             
        }
        
        void Send(byte val)
        {
        }
        
        void Send(const char *p)
        {
        }
        
        void Send(const void *p, int count)
        {
        }
        
        bool ReadyToRead()
        {
            return false;
        }
        
        byte Read()
        {
            while(!ReadyToRead());
            return 0;
        }
        
        void Read(void *p, int count)
        {
        }
                
    };
    
}  // namespace BfDev

#endif

