#include "stdafx.h"

#include "VDK.h"
#include <lwip/inet.h>

#include "Platform/PlatformBlackfinMini.h"
#include "Platform/Platform.h"
#include "Utils/StackString.h"
#include "iVDK/CriticalRegion.h"
#include "iVDK/Mutex.h"
#include "iVDK/VdkUtils.h"
#include "BfDev/SysProperties.h"


namespace 
{
	  bool isSysPropertiesInit = false;
	  
	  iVDK::Mutex *PGInetAddrMutex = 0;
};

// -------------------------------------

namespace Platform
{

    void GetStackTrace(std::string &trace)
    {
        trace.clear();
    }

    
    //----------------------------------------------
    
    dword GetSystemTickCount()
    {
      if(!isSysPropertiesInit)
      {
        BfDev::SysProperties::Instance().VerifyInit();
        isSysPropertiesInit = true;
      }
      return VDK::GetUptime();
    }
    
    // ---------------------------------------------------

    void ThreadSleep(dword msInterval)
    {
        iVDK::Sleep(msInterval);
    }

    // ---------------------------------------------------

    void ThreadMinimalSleep()
    {
        ThreadSleep(1);
    }
        
    // ----------------------------------------------
        
    std::string FormatTypeidName(const char *pName)
    {
        return pName;
    }
    
    // ----------------------------------------------

    void ThreadWaitUS(int microSec)
    {
        // this is not implemented yet
        ThreadSleep(1);
    }

    // ----------------------------------------------
    
    dword GetCurrentProcessThreadCount()
    {
        return VDK::GetAllThreads(0, 0);
    }

    // -----------------------------------------------------
    
    // VDK kernel panic handle
    extern "C" void KernelPanic(VDK::PanicCode code, VDK::SystemError err, const int value)
	{
		Utils::StackString s;
		
		s += "VDK kernel panic, code ";
		s.AddInt(code);
		s += ", error ";
		s.AddInt(err);
		s += ", value ";
		s.AddInt(value);

		// add description	
		if (err == VDK::kNewFailure) s += "; Out of heap!";
		
		ExitWithMessageExt(s.c_str(), false);
	}
        
        
    // -----------------------------------------------------

    dword InetAddr(const char *pAddr, bool *pFail)
    {
        dword result = INADDR_NONE;
        bool fail = true;

        struct in_addr val;

        if (inet_aton(pAddr, &val)) 
        {
            result = (val.s_addr);
            fail = false;
        }

        if (pFail != 0) *pFail = fail;
        return result;
    }
    
    // -----------------------------------------------------
    
    void InetAddrToString(dword ip, std::string &result)
    {
    	// clear
        result.clear();
        result.reserve(32);
    	
    	// mutex create
    	if (PGInetAddrMutex == 0) PGInetAddrMutex = new iVDK::Mutex();
    	
    	// convert
    	PGInetAddrMutex->Lock();

        in_addr addr;
        addr.s_addr = ip;
        result += inet_ntoa(addr);
                
    	PGInetAddrMutex->Unlock();        
    }
    
    // -----------------------------------------------------

    Thread::ThreadID GetCurrentThreadID()
    {
        return Thread::GetCurrentThreadID();
    }
    
    // -----------------------------------------------------

    HeapState GetHeapState()
    {
        return HeapState(0, 0, heap_space_unused(0));
    }

    // -----------------------------------------------------

    IProcSnapshot* CreateProcSnapshot()
    {
        return 0;
    }
    
    
}  // namespace Platform
