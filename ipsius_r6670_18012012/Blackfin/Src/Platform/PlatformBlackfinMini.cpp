
#include <cstdio>
#include <cstdlib> 
#include <cstring>

#include <cplbtab.h> 
#include <cdefBF537.h>
#include <ccblkfn.h>

#include "boost/static_assert.hpp"

#include "PlatformBlackfinMini.h"
#include "PlatformTypes.h"

// ----------------------------------------------------------

namespace
{

    volatile Platform::ExitHandleFn GFn = 0;

}  // namespace

// ----------------------------------------------------------

namespace Platform
{

    ExitHandleFn SetExitHandle(ExitHandleFn newHandle)
    {
        ExitHandleFn prev = GFn;
        GFn = newHandle;
        return prev;
    }

    // ---------------------------------------------------------

    void ExitWithMessageExt(const char *pMsg, bool userContext)
    {        
        if (userContext)
        {
	        if (pMsg != 0)
	        {
				std::printf("Halted %s \n", pMsg);
				//std::cout << "Halted! " << pMsg << std::endl;
	        }
        
			cli();  // disable interrupts        	        	
        }

        // call user handle 
        if (GFn != 0)
        {
            GFn(pMsg);
        }
        
        static volatile int dummi = 0;
                      
        // loop forever    	  
        while(true)
        {
 	        	dummi++;
        }
    }

    // ---------------------------------------------------------
            
    void ExitWithMessage(const char *pMsg)
    {
    	ExitWithMessageExt(pMsg, (!InsideIrqContext()) );
    }  

    // ---------------------------------------------------------

    bool InsideIrqContext()
    {
        dword mask = 0xffff7fef;
        dword reg = *((volatile dword *)IPEND); // & mask;
        reg &= mask;
        return reg != 0;                    	
    }

    // ---------------------------------------------------------

    bool HandleAssert(const char *pMsg, const char *pFile, int line)
    {
    	const int CBuffSize    = 120;
    	const int CBuffMaxMsg  = 80;
    	    	
    	const char *p = (pMsg == 0) ? "<none>" : pMsg;
    	if (std::strlen(p) > CBuffMaxMsg) p = "<too long>";
    	
    	char buff[CBuffSize];
    	buff[0] = 0;
    	std::sprintf(buff, "ASSERT %s @ %s:%i", p, pFile, line);
    	
    	ExitWithMessage(buff);

        // never goes here    	    	
    	return true;    	
    }
        
}  // namespace Platform

