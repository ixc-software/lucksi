
#ifndef __PLATFORM__
#define __PLATFORM__

#include "stdafx.h"

#include "Utils/IBasicInterface.h"

#include "Platform/PlatformTypes.h"
#include "Platform/PlatformMemory.h"
#include "Platform/PlatformAtomic.h"
#include "Platform/PlatformMutex.h"
#include "Platform/PlatformMutexLocker.h"
#include "Platform/PlatformThread.h"
#include "Platform/PlatformWaitCondition.h"

#include "PlatformExitHandle.h"

namespace Platform
{
    // get msec ticks
    dword GetSystemTickCount();

    // get microsec tick
    ddword GetSystemMicrosecTickCount(); 
    
    // get stack trace
    void GetStackTrace(std::string &trace);

    void ThreadSleep(dword msInterval);

    // do minimal thread sleep
    void ThreadMinimalSleep();

    // thread sleep in micro seconds
    void ThreadWaitUS(int microSec);

    // return thread count for current process
    dword GetCurrentProcessThreadCount();

    // pName = typeid(T).name() 
    // result must be formated as "Namespace::Namespace::ClassTypeName"
    std::string FormatTypeidName(const char *pName);

    // make typeid(T).name() for enum className::enumName
    std::string MakeEnumTypeid(const std::string &className, const std::string &enumName);

    // align size to platform requriments
    size_t PlatformDataAlign(IntPtrValue size);

    // '/' in Posix and '\' in Win32
    char PathSeparator();

    // non blocking, return key code or -1
    int KeyWasPressed();

    // exit from application with exception
    void ExitWithMessage(const char *pMsg);

    // return true if called from IRQ context (can be on Blackfin only)
    bool InsideIrqContext();

    //  Ascii internet address interpretation routine. The value returned is in network order.
    dword InetAddr(const char *pAddr, bool *pFail = 0);

    // Convert IP in network order to string 
    void InetAddrToString(dword ip, std::string &result);

    // for thread profiling -- thread working time
    // use only as difference
    // default implementation -- return GetSystemTickCount() * 1000
    ddword GetCurrentThreadTimesMcs();

    Thread::ThreadID GetCurrentThreadID();

    // -----------------------------------------------------

    struct HeapState
    {
        size_t BlockCount; 
        size_t BytesAllocated;
        size_t BytesFree;

        HeapState(size_t blockCount, size_t bytesAllocated, size_t bytesFree)
        {
            BlockCount      = blockCount;
            BytesAllocated  = bytesAllocated;
            BytesFree       = bytesFree;
        }

        bool Availble() const
        {
            return (BlockCount > 0) || (BytesAllocated > 0) || (BytesFree > 0);
        }

        std::string ToString() const
        {
            std::ostringstream oss;

            if (Availble())
            {
                if (BlockCount > 0)         oss << "blocks = " << BlockCount << " ";
                if (BytesAllocated > 0)     oss << "allocated = " << BytesAllocated << " ";
                if (BytesFree > 0)          oss << "free = " << BytesFree << " ";
            }
            else
            {
                oss << "<not availble>";
            }

            return oss.str();
        }
    };

    HeapState GetHeapState();

    // -----------------------------------------------------

    class IProcSnapshot;

    IProcSnapshot* CreateProcSnapshot();

}  // namespace Platform

#endif

