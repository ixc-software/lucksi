
#include "stdafx.h"

#include "POSIXProcessInfo.h"
#include "Utils/ErrorsSubsystem.h"
#include "Utils/ManagedList.h"
#include "iCore/MsgThread.h"
#include "Utils/StringParser.h"

namespace boost
{
    void assertion_failed_msg(char const*, char const*, char const*, char const*, long)
    {}
}

namespace
{
    using std::string;

    bool ConvertableToInt(const string& str)
    {
        for (size_t i = 0; i < str.size(); ++i)
        {
            if (!isdigit(str[i])) return false;
        }

        return true;
    }
    
    // ----------------------------------------------
       
    string ReadFromFile(const string& path)
    {
        QFile iFile(path.c_str());

        if (!iFile.exists()) return "";

        iFile.open(QIODevice::ReadOnly);
        QTextStream ts(&iFile);
        QString data = ts.readAll();
        iFile.close();

        return data.toStdString();
    }
    	
} // namespace

// ----------------------------------------------

namespace POSIXProcessInfo
{
    using std::stringstream;

    // ----------------------------------------------
    // ProcessInfo impl
    
    enum Separators
    {
        CPid = 0,
        CName= 1,
        CState = 2,
        CPpid = 3,
        CPgrp = 4,
        CThreadCount = 19,
    };

    // ---------------------------------------------

    ProcessInfo* ProcessInfo::TryCreate(const string &pid)
    {
        if (!ConvertableToInt(pid)) return 0;
        try
        {
            return new ProcessInfo(boost::lexical_cast<pid_t>(pid));
        }
        catch(...)
        {
            return 0;
        }
    }

    // ---------------------------------------------

    ProcessInfo::ProcessInfo(pid_t pid)
    {
        stringstream fPath;
        fPath << "/proc/" << pid <<"/stat";

        string inStr = ReadFromFile(fPath.str());

        Utils::ManagedList<string> data;
        StringParser(inStr, " ", data);

        //m_pid = boost::lexical_cast<pid_t>(*data[CPid]);
        m_pid = pid;
        m_name = *data[CName];
        m_state = *data[CState];
        m_ppid = boost::lexical_cast<pid_t>(*data[CPpid]);
        m_pgrp = boost::lexical_cast<pid_t>(*data[CPgrp]);
        m_threadCount = boost::lexical_cast<int>(*data[CThreadCount]);

        // fd list 
        stringstream fdPath;
        fdPath << "/proc/" << pid <<"/fd";
        
        QDir fdDir(fdPath.str().c_str());
        ESS_ASSERT(fdDir.exists());
        m_opennedFiles = fdDir.entryInfoList();
    }

    // ---------------------------------------------

    bool ProcessInfo::ContainsLinkToFile(const QString &absFilePath) const
    {
        QFileInfo info(absFilePath);
        ESS_ASSERT(info.exists());
        ESS_ASSERT(info.isAbsolute());

        for (int i = 0; i < m_opennedFiles.size(); ++i)
        {
            if (!m_opennedFiles.at(i).isSymLink()) continue;
            if (m_opennedFiles.at(i).symLinkTarget() == absFilePath) return true;
            std::cout << m_opennedFiles.at(i).symLinkTarget().toStdString() << std::endl;
        }

        return false;
    }

    // ---------------------------------------------

    bool ProcessInfo::ContainsLinkToComPort(int number) const
    {
        // /dev/tts/0
        QString portDev = QString("/dev/tts/%1").arg(number);

        for (int i = 0; i < m_opennedFiles.size(); ++i)
        {
            if (!m_opennedFiles.at(i).isSymLink()) continue;
            if (m_opennedFiles.at(i).symLinkTarget() == portDev) return true;
            std::cout << m_opennedFiles.at(i).symLinkTarget().toStdString() << std::endl;
        }

        return false;
    }
    
    // ---------------------------------------------
    // ProcessList impl
    
    ProcessList::ProcessList()
    {
        using namespace boost::filesystem;
        
        path mainDirPath("/proc/");

        typedef directory_iterator DI;
        DI endItr; 

        // get list of all process
        for (DI itr(mainDirPath); itr != endItr; ++itr)
        {
            if (is_directory(itr->status())) 
            {
                ProcessInfo* p = ProcessInfo::TryCreate(itr->path().filename().string()); // boost_1_47_0
                if (p != 0) m_list.Add(p);
            }
        } 
    }

    // ---------------------------------------------

    dword  ProcessList::FindProcessChildrenCount(pid_t pid)
    {
        dword counter = 0;
        for (size_t i = 0; i < m_list.Size(); ++i)
        {
            if (m_list[i]->getPPID() == pid) ++counter;
        }

        return counter;
    }

    // ---------------------------------------------

    const ProcessInfo& ProcessList::operator[](int posInList)
    { 
       return *(m_list[posInList]);
    }

    // ---------------------------------------------

    ProcessInfo* ProcessList::FindProcess(pid_t pid)
    {
        for (size_t i = 0; i < m_list.Size(); ++i)
        {
            if (m_list[i]->getPID() == pid) return m_list[i];
        }
        return 0;
    }

    // ---------------------------------------------

    dword ProcessList::GetThreadCount()
    {
        dword processCount = 0; 

        for (size_t i = 0; i < m_list.Size(); ++i)
        {
            ESS_ASSERT(m_list[i] != 0);
            processCount += m_list[i]->getThreadCount();
        }

        return processCount;
    }

    // ---------------------------------------------

    ProcessInfo* ProcessList::FindCurrentProcess()
    {
        pid_t selfPID = getpid();

        return ProcessInfo::TryCreate(boost::lexical_cast<string>(selfPID));
    }

    // ---------------------------------------------

    bool ProcessList::FileIsOpen(const string &absFilePath)
    {
        for (int i = 0; i < m_list.Size(); ++i)
        {
            if (m_list[i]->ContainsLinkToFile(absFilePath.c_str())) return true;
        }

        return false;
    }

    // ---------------------------------------------

    bool ProcessList::ComPortIsOpen(int number)
    {
        ESS_ASSERT(number >= 0);
        for (int i = 0; i < m_list.Size(); ++i)
        {
            if (m_list[i]->ContainsLinkToComPort(number)) return true;
        }

        return false;
    
    }
    

} // namespace POSIXProcessInfo


namespace 
{
    // /proc/pid
    bool ComPortIsOpenedByProcess(const QString &procPath, int portNum)
    {
        QString fdPath = QString("%1/fd").arg(procPath);

        QFileInfoList files = QDir(fdPath).entryInfoList();

        QString portDev = QString("/dev/tts/%1").arg(portNum);

        for (int i = 0; i < files.size(); ++i)
        {
            if (!files.at(i).isSymLink()) continue;
            if (files.at(i).symLinkTarget() == portDev) return true;
        }

        return false;
    }

    // ---------------------------------------------
    
    bool ComPortIsOpen(int num)
    {
        QDir proc("/proc");
        QFileInfoList procDirs = proc.entryInfoList(QDir::Dirs);

        for (int i = 0; i < procDirs.size(); ++i)
        {
            if (ComPortIsOpenedByProcess(procDirs.at(i).absoluteFilePath(), num)) return true;
        }

        return false;
    }

    // ---------------------------------------------

    
    
} // namespace 


