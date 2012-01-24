#pragma once

#include <sys/types.h>

#include "PlatformTypes.h"
#include "Utils/ManagedList.h"

namespace POSIXProcessInfo
{
    using namespace Platform;

    using std::string;

    //linux's process info
    class ProcessInfo
    {
        string m_name;
        string m_state;
        pid_t m_pid;
        pid_t m_ppid;
        pid_t m_pgrp;
        int m_threadCount;
        QFileInfoList m_opennedFiles;

        ProcessInfo(pid_t pid);
        
    public:

        const string& getName() const { return m_name;}
        const string& getState() const { return m_state;}
        pid_t getPID() const { return m_pid;}
        pid_t getPPID() const { return m_ppid;}
        pid_t getPGrp() const { return m_pgrp;}
        int getThreadCount() const { return m_threadCount;}
        // all are symlinks
        QFileInfoList OpennedFiles() const { return m_opennedFiles; }
        bool ContainsLinkToFile(const QString &absFilePath) const;
        bool ContainsLinkToComPort(int number) const;

        static ProcessInfo* TryCreate(const string &pid);
    };

    // -------------------------------------------

    // list of process from /proc
    class ProcessList
    {
        Utils::ManagedList<ProcessInfo> m_list;
        
    public:
        ProcessList(); // all processes in system

        dword getProcessCount() { return m_list.Size();}
        dword GetThreadCount();

        ProcessInfo* FindProcess(pid_t pid);
        static ProcessInfo* FindCurrentProcess();
        dword FindProcessChildrenCount(pid_t pid);
        const ProcessInfo& operator[](int pos);

        // look through processes /fd dirrectory, 
        bool FileIsOpen(const string &absFilePath);
        bool ComPortIsOpen(int number);
    };

} // POSIXProcessInfo



