
#include "stdafx.h"

#include "Utils/StringParser.h"
#include "Utils/IntToString.h"
#include "Platform.h"
#include "PlatformPOSIX.h"
#include "POSIXProcessInfo.h"
#include "PlatformShared.h"

// #include "iCore/MsgThread.h"
#include "PlatformThread.h"
#include "PlatformMutex.h"


//#include <demangle.h>

#include <sys/types.h>
#include <sys/time.h>

// for _kbhit()
#include <sys/select.h>
#include <termios.h>
//#include <stropts.h>
#include <sys/ioctl.h>

extern "C" char *cplus_demangle (const char *mangled, int options);

namespace Platform
{
    /*
    dword GetSystemTickCount()
    {
        static timeval ZeroTime = {0, 0};
    
        if(ZeroTime.tv_sec == 0)
        {
            gettimeofday(&ZeroTime,NULL);
            return 0;
        }
    
        timeval currentTime;
        gettimeofday(&currentTime, NULL);
    
        long secDiff = currentTime.tv_sec - ZeroTime.tv_sec;
        long usDiff = currentTime.tv_usec - ZeroTime.tv_usec;
    
        return (secDiff * 1000 + usDiff / 1000);       //milisec
    }
    */

    dword GetSystemTickCount()
    {
        return (GetSystemMicrosecTickCount() / 1000);
    }

    // -----------------------------------------------------

    /*
    // thread unsafe
    ddword GetSystemMicrosecTickCount()
    {
        static timeval ZeroTime = {0, 0};
    
        if(ZeroTime.tv_sec == 0)
        {
            gettimeofday(&ZeroTime,NULL);
            return 0;
        }
    
        timeval currentTime;
        gettimeofday(&currentTime, NULL);
    
        long secDiff = currentTime.tv_sec - ZeroTime.tv_sec;
        long usDiff = currentTime.tv_usec - ZeroTime.tv_usec;
    
        return (secDiff * 1000 * 1000 + usDiff);       // micro sec
    }
    */

    // ---------------------------------------------------

    namespace 
    {
        using namespace Platform;

        class ZeroTime
        {
            timeval m_time;
            Mutex m_mutex;

        public:
            ZeroTime()
            {
                m_mutex.Lock();
                gettimeofday(&m_time, NULL);
                m_mutex.Unlock();
            }

            dword Sec() const { return m_time.tv_sec; }
            // always less than 1 000 000
            dword Usec() const { return m_time.tv_usec; }
        };

        
        ZeroTime GZeroTime;
        
        
    } // namespace 
    
    ddword GetSystemMicrosecTickCount()
    {
        // TimeValue currTime;
        timeval currTime;
        gettimeofday(&currTime, NULL);
        
        long secDiff = currTime.tv_sec - GZeroTime.Sec();
        long usDiff = currTime.tv_usec - GZeroTime.Usec();

        ddword resUsec = secDiff;
        resUsec *= 1000;
        resUsec *= 1000;
        resUsec += usDiff;

        return resUsec;        
    }
    
    // ---------------------------------------------------
    
    namespace
    {
        const char CBeginSep = '(';
        const char CEndSep = ')';
        const int CFuncAddrLen = 5;
    
        std::string ExtractMangledString(char *symbols)
        {
            std::string res;
            std::string data(symbols);
    
            int iBegin = 0;
            int iEnd = 0;
            while (iEnd < data.size())
            {
                if (data[iEnd] == CBeginSep) iBegin = iEnd + 1;
                if (data[iEnd] == CEndSep)
                {
                    break;
                }
                ++iEnd;
            }
    
            res = data.substr(iBegin, (iEnd - iBegin- CFuncAddrLen));
    
            return res;
        }
    
    } // nmesapce 
    
    
    void GetStackTrace(std::string &trace)
    {
        const int count = 32;
        void* funcList[count];
        
        trace = "";
    
        int size  = detail::backtrace(funcList, count);
        char** symbols = detail::backtraceSymbols(funcList, size);
    
        std::string outString; 
    
        for (int i = 0; i < size; ++i)
        {
            std::string mangledName = ExtractMangledString(symbols[i]);
    
            char *p = detail::cplusDemangle(mangledName.c_str(), 1);   // 1 - option
            if (p != 0)
            {
                outString += std::string(p);
                outString += "\n";
            }
            
            // delete p;
            free(p);
        }
    
        free(symbols);
        
        // set result
        trace = outString;
    }
    
    
    // ---------------------------------------------------
    
    void ThreadWaitUS(int microsec)
    {
        class SleepTimerMicro: public QThread 
        {
        public:
            static void Run(int microSec)
            {
                usleep(microSec); 
            }
        };
    
        SleepTimerMicro::Run(microsec);
    }
    
    // ---------------------------------------------------
    
    void ThreadMinimalSleep()
    {
        class ThreadSleep : public QThread
        {
        public:
    
            static void Sleep()
            {
                usleep(50);
            }
        };
    
        ThreadSleep::Sleep();
    }
    
    // ---------------------------------------------------

    void ThreadSleep(dword msInterval)
    {
        PlatformShared::ThreadSleep(msInterval);    	
    }
        
    // ---------------------------------------------------
    
    dword GetCurrentProcessThreadCount()
    {
        using namespace POSIXProcessInfo;
    
        //need to wait for get "clear" information  

        // iCore::MsgThread::Sleep(100);
        ThreadSleep(100);
    
        //ProcessInfo* p = ProcessList::FindCurrentProcess();
    
        return ProcessList::FindCurrentProcess()->getThreadCount();
    }
    
    // --------------------------------------------------
    
    namespace
    {
        enum TypeInfo
        {
            TSimple = 0,
            TPtr = 1,
            // TRef = 2,
        };
        
        using std::string;
        
        TypeInfo CutBeginEnd(string& inStr)
        {
            // starts from digit -- class outside namespace
            // 'N' and 'E' -- begin-end -- using when class inside namespace or other class
            // 'P' means pointer type
            
            TypeInfo res = TSimple;

            if (*(inStr.begin()) == 'P')
            {
                // erase 'P'
                inStr.erase(0, 1);
                res = TPtr;
            }

            /*
            if ((*(inStr.begin()) != 'N') 
                && (*(inStr.end() - 1) != 'E')) return res;
            */

            if ((*(inStr.begin()) == 'N') 
                && (*(inStr.end() - 1) == 'E'))
            {
                // erase 'N'
                inStr.erase(0, 1);
    
                // erase 'E'
                inStr.erase(inStr.end()-1, inStr.end());
            }
        
            return res;
        }

        // --------------------------------------------------

        string CaseSimpleTypes(const string &str)
        {
            if (typeid(int).name() == str) return "int";
            if (typeid(bool).name() == str) return "bool";
            if (typeid(string).name() == str) return "std::string";

            return str; // if can't parse return original string
        }
        
        // --------------------------------------------------
    
        string RemoveName(string& inStr)
        {
            size_t pos = 0;
            string lenStr;
    
            //find number of name's letters
            while (isdigit(inStr[pos])) 
            {
                lenStr += inStr[pos];
                ++pos;
            }
    
            if (lenStr == "") return "";
            
            size_t len = boost::lexical_cast<size_t>(lenStr);
    
            string name = inStr.substr(pos, len);
    
            inStr = inStr.substr(pos + len);
    
            return name;
        }
    
    } // namespace
    
    // --------------------------------------------------
    // "N13namespaceName6ClassAE" ==> "namespaceName::ClassA"
    // "PN13namespaceName6ClassAE" ==> "namespaceName::ClassA*"
    // Don't use this for T&, because typeid() recognizes T& as T.
    // To use more simple types add their names to CaseSimpleTypes().
    std::string FormatTypeidName(const char *pName)
    {
        using std::string;
        
        string inStr(pName); 
        string outStr;

        TypeInfo info = CutBeginEnd(inStr);

        while (true)
        {
            string name = RemoveName(inStr);

            if ((name == "") && (outStr == "")) return CaseSimpleTypes(inStr);

            if (name == "") break;
            if (outStr != "") outStr +="::";
            outStr += name;
        }

        if (info == TPtr) outStr += '*';
        
        return outStr;
    }
    
    // --------------------------------------------------
    
    std::string MakeEnumTypeid(const std::string &className, const std::string &enumName)
    {
        
        struct X
        {
            static void MakeEnumTypeidStep(const std::string &item, std::string &out)
            {
            	out += Utils::IntToString(item.size()) + item;
            }
        };
        
    	Utils::ManagedList<std::string> items;
    	Utils::StringParser(className, "::", items, true);
    
    	std::string result("N");
    
    	for(int i = 0; i < items.Size(); ++i)
    	{
    		X::MakeEnumTypeidStep(*items[i], result);
    	}
    
    	X::MakeEnumTypeidStep(enumName, result);
    	    
    	return result + "E";
    }
    

    // --------------------------------------------------
    
    size_t PlatformDataAlign(size_t size)
    {
    	return size;
    }

    // ---------------------------------------------------

    char PathSeparator()
    {
        return '/';
    }

    // -----------------------------------------------------

    namespace 
    {
        // _kbhit for Linux
        int _kbhit()
        {
            // FIONREAD
            // int cmdFionread = 0x541B;  // any char
            
            static const int STDIN = 0;
            static bool initialized = false;
        
            if (! initialized) 
            {
                // Use termios to turn off line buffering
                termios term;
                tcgetattr(STDIN, &term);
                term.c_lflag &= ~ICANON;
                tcsetattr(STDIN, TCSANOW, &term);
                setbuf(stdin, NULL);
                initialized = true;
            }
        
            int bytesWaiting;
            ioctl(STDIN, FIONREAD, &bytesWaiting);

            return bytesWaiting;
        }

    } // namespace 

    int KeyWasPressed()
    {
    	if (_kbhit() == 0) return -1;    	
        return std::cin.get();
    }
    
    // -----------------------------------------------------

    void ExitWithMessage(const char *pMsg)
    {
        PlatformShared::ExitWithMessage(pMsg);
    }
    
    // -----------------------------------------------------

    bool InsideIrqContext()
    {
        return false;
    }

    // -----------------------------------------------------

    dword InetAddr(const char *pAddr, bool *pFail)
    {
        return PlatformShared::InetAddr(pAddr, pFail);
    }

    void InetAddrToString(dword ip, std::string &result)
    {
        return PlatformShared::InetAddrToString(ip, result);
    }

    // -----------------------------------------------------
    
    ExitHandleFn SetExitHandle(ExitHandleFn newHandle)
    {
        return PlatformShared::SetExitHandle(newHandle);
    }

    // -----------------------------------------------------
    
    ddword GetCurrentThreadTimesMcs()
    {
        return GetSystemMicrosecTickCount();
    }

    // -----------------------------------------------------

    Thread::ThreadID GetCurrentThreadID()
    {
        return Thread::GetCurrentThreadID();
    }

    // -----------------------------------------------------

    HeapState GetHeapState()
    {
        return HeapState(0, 0, 0);
    }
    
    // -----------------------------------------------------

    IProcSnapshot* CreateProcSnapshot()
    {
        return 0;
    }
    
    
           
} // namespace Platform
