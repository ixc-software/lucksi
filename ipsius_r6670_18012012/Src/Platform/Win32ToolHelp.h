#ifndef __WIN32TOOLHELP__
#define __WIN32TOOLHELP__

#include "stdafx.h"

#include <Windows.h>
#include <TlHelp32.h>

#include "Utils/ErrorsSubsystem.h"

namespace Win32
{

    ESS_TYPEDEF(ToolHelpException);

    // ------------------------------------------------------------

    class ProcessList : boost::noncopyable
    {
        std::vector<PROCESSENTRY32> m_list;

    public:

        ProcessList(HANDLE h)
        {
            PROCESSENTRY32 entry;
            entry.dwSize = sizeof(entry);

            if (!Process32First(h, &entry)) ESS_THROW(ToolHelpException);

            do
            {
                m_list.push_back(entry);

            } while (Process32Next(h, &entry) == (BOOL)TRUE);
        }

        int Size() const { return m_list.size(); }
        bool Empty() const { return m_list.empty(); }

        const PROCESSENTRY32& operator[] (int indx) const
        {
            return m_list.at(indx);
        }

        const PROCESSENTRY32* FindByProcessID(DWORD id = 0) const
        {
            if (id == 0) id = GetCurrentProcessId();

            for(int i = 0; i < m_list.size(); ++i)
            {
                if (m_list[i].th32ProcessID == id) return &m_list[i];
            }

            return 0;
        }

    };

    // ------------------------------------------------------------

    class ThreadList : boost::noncopyable
    {
        std::vector<THREADENTRY32> m_list;

    public:

        ThreadList(HANDLE h)
        {
            DWORD currId = GetCurrentProcessId();
            int currThreadCounter = 0;

            THREADENTRY32 entry;
            entry.dwSize = sizeof(entry);

            if (!Thread32First(h, &entry)) ESS_THROW(ToolHelpException);

            do
            {
                if (entry.th32OwnerProcessID == currId) 
                {
                    ++currThreadCounter;
                }

                m_list.push_back(entry);
                entry.dwSize = sizeof(entry);

            } while (Thread32Next(h, &entry));
        }

        ThreadList(const ThreadList &other, DWORD parentId = 0)
        {
            if (parentId == 0) parentId = GetCurrentProcessId();

            for(int i = 0; i < other.Size(); ++i)
            {
                if (other[i].th32OwnerProcessID == parentId)
                {
                    m_list.push_back( other[i] );
                }
            }
        }

        int Size() const { return m_list.size(); }
        bool Empty() const { return m_list.empty(); }

        const THREADENTRY32& operator[] (int indx) const
        {
            return m_list.at(indx);
        }

        const THREADENTRY32* FindByID(DWORD threadId) const
        {
            for(int i = 0; i < m_list.size(); ++i)
            {
                if (m_list[i].th32ThreadID == threadId) return &m_list[i];
            }

            return 0;
        }

        static std::string ThreadPrioToStr(LONG prio)
        {
            if (prio == THREAD_PRIORITY_IDLE)               return "idle";
            if (prio == THREAD_PRIORITY_LOWEST)             return "lowest";
            if (prio == THREAD_PRIORITY_BELOW_NORMAL)       return "normal-";
            if (prio == THREAD_PRIORITY_NORMAL)             return "normal";
            if (prio == THREAD_PRIORITY_ABOVE_NORMAL)       return "normal+";
            if (prio == THREAD_PRIORITY_HIGHEST)            return "highest";
            if (prio == THREAD_PRIORITY_TIME_CRITICAL)      return "real-time";

            return "?";
        }

    };

    // ------------------------------------------------------------

	class ToolHelp
	{

        class Snapshot  // RAII
        {
            HANDLE m_h;

        public:
            Snapshot(DWORD flags = (TH32CS_SNAPPROCESS | TH32CS_SNAPTHREAD), DWORD processID = 0)
            {
                m_h = CreateToolhelp32Snapshot(flags, processID);
                if (m_h == INVALID_HANDLE_VALUE) ESS_THROW(ToolHelpException);
            }

            HANDLE Handle() const { return m_h; }

            ~Snapshot()
            {
                CloseHandle(m_h);
            }
        };

        Snapshot m_snap;
        boost::scoped_ptr<ProcessList> m_processes;
        boost::scoped_ptr<ThreadList> m_threads;

    public:

        ToolHelp()
        {
        }

        const ProcessList& Processes()
        {
            if (m_processes == 0)
            {
                m_processes.reset( new ProcessList( m_snap.Handle() ) );
            }

            return *m_processes;
        }

        const ThreadList& Threads()
        {
            if (m_threads == 0)
            {
                m_threads.reset( new ThreadList( m_snap.Handle() ) );
            }

            return *m_threads;
        }

	};	
	
};

#endif


