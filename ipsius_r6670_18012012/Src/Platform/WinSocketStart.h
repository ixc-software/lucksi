#ifndef __WINSOCKSTART__
#define __WINSOCKSTART__

#include <windows.h>
#include "Utils/ErrorsSubsystem.h"

namespace WinSocket
{
    class WinSoketStart
    {
    public:
        WinSoketStart(int major = 2, int minor = 2)
        {
            WSADATA wsaData;
            int err = ::WSAStartup(MAKEWORD(minor, major), &wsaData);

            if (err) 
            {
                std::stringstream ss;
                ss << "Failed to initialize the Windows Sockets library, error code ";
                ss << err;
                ESS_HALT(ss.str());
            }
        }

        ~WinSoketStart()
        {
            ::WSACleanup();    
        }
    };

    //typedef WinSoketStartT<2,2> WinSocketStart;

} // namespace WinSocket

#endif
