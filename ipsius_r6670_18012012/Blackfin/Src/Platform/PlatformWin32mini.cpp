#include "stdafx.h"

#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers
#include <windows.h>

#include "Platform/Platform.h"
#include "iVDK/CriticalRegion.h"

// -------------------------------------

namespace Platform
{

    void GetStackTrace(std::string &trace)
    {
        trace.clear();
    }

    // ----------------------------------------------

    void ExitWithException(const std::exception *pException)
    {
        if (pException) 
        {
            std::cerr << std::endl << std::endl
                << "Unexpected exception, aborted" << std::endl
                << pException->what() << std::endl;
        }

        std::exit(-1);  // std::abort();
    }

    // ----------------------------------------------

    std::string FormatTypeidName(const char *pName)
    {
        return pName;
    }

    // ----------------------------------------------

    dword GetSystemTickCount()
    {
        return GetTickCount();
    }

    // ----------------------------------------------

    void ThreadSleep(dword msInterval)
    {
        Sleep(msInterval);
    }





}  // namespace Platform

// ----------------------------------------------

namespace iVDK
{

    void CriticalRegion::Enter() {}
    void CriticalRegion::Leave() {}

    Mutex::Mutex()          {}
    Mutex::~Mutex()         {}
    void Mutex::Lock()      {}
    void Mutex::Unlock()    {}

    int GetCurrentThreadID() { return 0; }



}  // namespace iVDK

