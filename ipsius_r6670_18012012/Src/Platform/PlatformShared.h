#ifndef __PLATFORMSHARED__
#define __PLATFORMSHARED__

#include "PlatformUtils.h"

namespace Platform
{
	
	class PlatformShared
	{
        
    public:

        static void ExitWithMessage(const char *pMsg)
        {
            static bool InRecursion = false;

            QString ts = QDateTime::currentDateTime().toString("hh:mm:ss dd.MM.yy");

            std::cerr << std::endl << std::endl
                << "Aborted! " << ts.toStdString() << std::endl;

        	if (pMsg != 0) 
            {
                std::cerr << std::string(pMsg) << std::endl;
            }

            if (InRecursion)
            {
                std::cerr << std::endl << "Recursive exit!" << std::endl;
                std::abort();
                return;
            }

            InRecursion = true;

            std::exit(-1);  // std::abort();
        }

        static void ThreadSleep(dword msInterval)
        {
            class Thread : QThread
            {
            public:

                static void Sleep(dword msInterval)
                {
                    QThread::usleep(msInterval * 1000UL);  //interval in microseconds        
                }
            };

            Thread::Sleep(msInterval);
        }

        static dword InetAddr(const char *pAddr, bool *pFail = 0)
        {
            QHostAddress addr;
            
            bool fail = true;
            dword val = 0xFFFFFFFF;

            if (addr.setAddress( QString(pAddr) ))
            {
                val = RevertBytesInDword( addr.toIPv4Address() );  // revert for network order
                fail = false;
            }

            if (pFail != 0) *pFail = fail;

            return val;
        }

        static void InetAddrToString(dword ip, std::string &result)
        {
            QHostAddress addr( RevertBytesInDword(ip) );
            result = addr.toString().toStdString();
        }
        
        static ExitHandleFn SetExitHandle(ExitHandleFn newHandle)
        {
            // not implemented
            return 0;
        }
                
	};
	
}  // namespace Platform

#endif
