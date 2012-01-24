
#include "stdafx.h"

#include "Utils/ErrorsSubsystem.h"
#include "Utils/QtHelpers.h"
#include "Utils/ArraySize.h"
#include "Utils/IntToString.h"

#include "Platform/Win32ToolHelp.h"

#include "Platform.h"
#include "Win32ToolHelp.h"
#include "SimpleStackTracker.h"
#include "PlatformShared.h"
#include "IProcSnapshot.h"


#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers
#include <windows.h>

#include <conio.h>      // for KeyWasPressed()

// прототип функции почему-то в стандартном .h файле не находиться
extern "C"
{
    WINBASEAPI BOOL WINAPI IsDebuggerPresent(VOID);
};

// ---------------------------------------------------

namespace
{
    QMutex GMutex;
}

// ---------------------------------------------------

namespace
{
    class Win32PrecisionTimer
    {
    public:
        Win32PrecisionTimer()
        {
            timeBeginPeriod(1);
        }

        ~Win32PrecisionTimer()
        {
            timeEndPeriod(1);
        }
    };

    Win32PrecisionTimer GTimer;

    Platform::ddword FileTimeToDdw(const FILETIME &ft)
    {
        ULARGE_INTEGER i;
        i.LowPart = ft.dwLowDateTime;
        i.HighPart = ft.dwHighDateTime;
        return i.QuadPart;
    }
}

// ---------------------------------------------------

namespace
{
    class HighPerformanceTimer
    {
        LARGE_INTEGER m_frequency; //timer counts per second

        /*inline*/ Platform::ddword CountToMsec(LARGE_INTEGER count) const 
        {                
            return count.QuadPart * 1000000L / m_frequency.QuadPart;        
        }           

        HighPerformanceTimer()
        {        
            bool timerPresent = QueryPerformanceFrequency( &m_frequency );
            /*if (!timerPresent)
                ESS_THROW(NoTimerDevInSystem);*/             
            ESS_ASSERT(timerPresent && "No timer device present in system");
        }

     public:

         Platform::ddword getTickMicroSec() const 
        {
            LARGE_INTEGER count;
            ESS_ASSERT( QueryPerformanceCounter(&count) );                

            return CountToMsec(count);
        }

        static const HighPerformanceTimer& Instance()
        {
            static HighPerformanceTimer hiTimer;
            return hiTimer;
        }
    };

    
}

// ---------------------------------------------------

namespace Platform
{

    dword GetSystemTickCount()
    {
        return GetTickCount();
    }

    // ---------------------------------------------------

    ddword GetSystemMicrosecTickCount()
    {                
        return HighPerformanceTimer::Instance().getTickMicroSec();
    }

    // ---------------------------------------------------

    void GetStackTrace(std::string &trace)  // throw()
    {    	
        enum { CUseStackTrace = true, };
        
        trace.clear();

        if (!CUseStackTrace) 
        {
        	trace = "<Stack tracing disabled!>"; 
        	return;
        }
        
        if (IsDebuggerPresent()) 
        {
        	trace = "<Debug mode active!>";
        	return;        	
        }

        // get it!
        {
            // danger: deadlock or call from global object contructor
            QMutexLocker lock(&GMutex);  

            trace = SimpleStackTracker::GetCurrentTrace();
        }
        
    }

    // ---------------------------------------------------

    void ThreadMinimalSleep()
    {
        class ThreadSleep : public QThread
        {
        public:

            static void Sleep()
            {
                msleep(1);
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

    void ThreadWaitUS(int microSec)
    {
        static int64 TicksPerSec = 0;

        BOOST_STATIC_ASSERT(sizeof(int64) >= sizeof(LARGE_INTEGER));

        class Counter
        {
        public:

            static int64 Get()
            {
                LARGE_INTEGER li;
                TUT_ASSERT(QueryPerformanceCounter(&li));
                return li.QuadPart;
            }
        };

        if (TicksPerSec == 0)
        {
            LARGE_INTEGER li;
            TUT_ASSERT(QueryPerformanceFrequency(&li) != 0); 
            TicksPerSec = li.QuadPart;
        }
        ESS_ASSERT(TicksPerSec > 0);

        // time to wait in ticks
        int64 duration = (microSec * TicksPerSec) / 1000000;
        int64 finish = Counter::Get() + duration;

        // wait
        while (Counter::Get() < finish);
    }

    // ---------------------------------------------------

    dword GetCurrentProcessThreadCount()
    {
        Win32::ToolHelp th;
        const PROCESSENTRY32 *p = th.Processes().FindByProcessID();
        return (p != 0) ? p->cntThreads : 0;
    }

    // ---------------------------------------------------

    namespace
    {
        std::string FormatPrimitiveTypes(const std::string &s)
        {
            if (typeid(std::string).name() == s) return "std::string";

            return "";
        }
    }

    // выбрасывает из выражения "enum " и "class "
    std::string FormatTypeidName(const char *pName)
    {
        std::string primitive = FormatPrimitiveTypes(pName);
        if (primitive.size() > 0) return primitive;

        QString s(pName);
        QRegExp rx("\\b(class|enum|struct)\\b.");
        // + заменить еще 'anonymous namespace' ?
        s.remove(rx);
        s.replace(" ", "");
        return Utils::QStringToString(s, false);
    }

    // ---------------------------------------------------

    std::string MakeEnumTypeid(const std::string &className, const std::string &enumName)
    {
        return "enum " + className + "::" + enumName;
    }

    // ---------------------------------------------------

    /*
    namespace
    {
    // удаляет pValue как слово + символ за ним
    void RemoveByRegexp(QString &s, const char *pValue)
    {
    QString regVal(pValue);
    int len = regVal.length();
    regVal = QString("\\b") + regVal + QString("\\b");

    QRegExp rx(regVal);

    while(true)
    {
    int indx = s.indexOf(rx);
    if (indx < 0) break;
    s.remove(indx, len + 1);
    } 
    }
    }

    std::string FormatTypeidName(const char *pName)
    {
    QString s(pName);
    RemoveByRegexp(s, "class");
    RemoveByRegexp(s, "enum");
    return QStringToString(s, false);
    } */

    // ---------------------------------------------------

    /*
    std::string FormatTypeidName(const char *pName)
    {
    using std::string;

    string s(pName);

    // must contain only one or two space
    size_t pos = s.find("`anonymous namespace'");
    int spaceCount = (pos != string::npos) ? 2 : 1;
    ESS_ASSERT(std::count(s.begin(), s.end(), ' ') == spaceCount);

    // must contain '::'
    pos = s.find("::");
    ESS_ASSERT(pos != string::npos);

    // must contatain 'class' or 'enum'
    pos = s.find("class ");
    bool hasClass = (pos != string::npos);

    pos = s.find("enum ");
    bool hasEnum = (pos != string::npos);

    ESS_ASSERT(hasClass || hasEnum);

    // cut name
    pos = s.find(" ");

    return s.substr(pos + 1);
    } */


    // ------------------------------------------

    size_t PlatformDataAlign(IntPtrValue size)
    {
        // return size;
        return AlignRuntime::Calc(size);
    }

    // ------------------------------------------

    char PathSeparator()
    {
        return '\\';
    }

    // -----------------------------------------------------

    int KeyWasPressed()
    {
        if (_kbhit() == 0) return -1;
        return _getch();
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

    /*
    ddword GetCurrentThreadTimesMcs()   // очень низкая точность измерения 
    {
        HANDLE h = GetCurrentThread();

        FILETIME tCreate, tExit, tKernel, tUser;

        if ( !GetThreadTimes(h, &tCreate, &tExit, &tKernel, &tUser) ) return 0;

        ddword kernel = FileTimeToDdw(tKernel);
        ddword user =   FileTimeToDdw(tUser);

        return (kernel + user) / 100;  // 100 ns to 1 mcs
    } */

    // -----------------------------------------------------

    Thread::ThreadID GetCurrentThreadID()
    {
        return Thread::GetCurrentThreadID();
    }

    // -----------------------------------------------------

    HeapState GetHeapState()
    {
        _CrtMemState state;
        std::memset(&state, 0, sizeof(state));
        _CrtMemCheckpoint(&state);

        size_t count = Utils::ArraySize(state.lCounts);
        size_t blockCount = 0;
        for(int i = 0; i < count; ++i) blockCount += state.lCounts[i];

        count = Utils::ArraySize(state.lSizes);
        size_t bytesAllocated = 0;
        for(int i = 0; i < count; ++i) bytesAllocated += state.lSizes[i];

        return HeapState(blockCount, bytesAllocated, 0);
    }

    // -----------------------------------------------------

    class WinProcSnapshot : public IProcSnapshot
    {
        boost::scoped_ptr<Win32::ThreadList> m_threads;

    // IProcSnapshot impl
    private:

        /*
        std::string ThreadPriority(Thread::ThreadID id)
        {
            int prio = ::GetThreadPriority(id);
            if (prio == THREAD_PRIORITY_ERROR_RETURN) return "bad";
            return Utils::IntToString(prio);
        } */

        std::string ThreadPriority(Thread::ThreadID id)
        {
            const THREADENTRY32 *p = m_threads->FindByID( (DWORD)id );
            if (p == 0) return "?";
            // return Win32::ThreadList::ThreadPrioToStr( p->tpBasePri );
            return Utils::IntToString( p->tpBasePri );
        }

        int ThreadCount()
        {
            return m_threads->Size();
        }

        Thread::ThreadID ThreadAt(int index)
        {
            return (Thread::ThreadID)(*m_threads)[index].th32ThreadID;
        }

    public:

        WinProcSnapshot()
        {
            Win32::ToolHelp m_snap;

            const PROCESSENTRY32 *p = m_snap.Processes().FindByProcessID();
            if (p)
            {
                int threads = p->cntThreads;
            }

            m_threads.reset( new Win32::ThreadList( m_snap.Threads() ) );
        }
    };

    IProcSnapshot* CreateProcSnapshot()
    {
        return new WinProcSnapshot();
    }

}  // namespace Platform
