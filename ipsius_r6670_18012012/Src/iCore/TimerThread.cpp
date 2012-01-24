
#include "stdafx.h"

#include "ProjConfigLocal.h"

#include "Utils/ILockable.h"
#include "Utils/ErrorsSubsystem.h"
#include "Utils/DelayInit.h"

#include "TimerThread.h"
#include "MsgBase.h"

using iCore::TimerThreadUser;

enum
{
    CWatchDogIntervalMs = 1000,

    CDisableHaltOnWatchdog = false,

    CTryExtractInfoFromMsg = true, // for watchog thread, can be dangerous!
};

namespace
{
    typedef std::vector<TimerThreadUser*> UsersList;

    Utils::ILockable *PInstanceLock; // мютекс дл€ нижележащих переменных
    Utils::DelayInit MutexInit(PInstanceLock);

    iCore::TimerThread *pInstance;
    int UseCount;
    UsersList Users;

}

namespace
{
    iCore::TimerThread& GetTimerThread(TimerThreadUser *pUser)  
    {
        ESS_ASSERT(PInstanceLock != 0);
        Utils::Locker locker(*PInstanceLock);

        if (pInstance == 0) pInstance = new iCore::TimerThread();
        UseCount++;

        // register user
        {
            ESS_ASSERT(pUser != 0);
            UsersList::iterator i = std::find(Users.begin(), Users.end(), pUser);
            ESS_ASSERT(i == Users.end());
            Users.push_back(pUser);
        }

        return *(pInstance);
    }

    void FreeTimerThread(TimerThreadUser *pUser)     // static
    {
        ESS_ASSERT(PInstanceLock != 0);
        Utils::Locker locker(*PInstanceLock);

        ESS_ASSERT(UseCount);
        UseCount--;

        // remove user
        {
            UsersList::iterator i = std::find(Users.begin(), Users.end(), pUser);
            ESS_ASSERT(i != Users.end());
            Users.erase(i);
        }

        if (UseCount) return;

        // UseCount == 0, delete pInstance
        ESS_ASSERT(pInstance);
        pInstance->BreakAndWait();
        delete pInstance;
        pInstance = 0;
    }


}


// ---------------------------------------------

namespace iCore
{
	
	
    void TimerThread::run()  // override QThread::run
    {
        setPriority(HighestPriority);

        try
        {
            while(!m_break)
            {
                Sleep(ProjConfig::CfgTimerThread::CSleepTimeMs);

                ProcessTimersList();
                ProcessWatchdog();
            }
        }
        catch (std::exception &e) 
        {
            ESS_UNEXPECTED_EXCEPTION(e);
        }

        ESS_ASSERT(m_list.empty());
    }

    // ---------------------------------------------------------

    int GetMaxMessageTimeMs()
    {
        return ProjConfig::CfgTimerThread::CMaxMessageTimeMs;
    }

    // ---------------------------------------------------------

    void TimerThread::WatchdogForThread(TimerThreadUser *pUser, const std::string &extInfo)
    {
        if (CDisableHaltOnWatchdog) return;

        try
        {
            ESS_HALT("Watchdog for thread!! " + extInfo);
        }
        catch(SuppressWatchdogException &e)
        {
            // ok, ignore what...
        }
    }


    // ---------------------------------------------------------

    void TimerThread::ProcessWatchdog()
    {       
        ESS_ASSERT(PInstanceLock != 0);

        // watchdog disabled
        if (CWatchDogIntervalMs <= 0) return;

        dword currTicks = SyncTimer::Ticks();
        if (currTicks - m_lastWatchdogTime < CWatchDogIntervalMs) return;

        {
            Utils::Locker locker(*PInstanceLock);

            std::string extInfo;

            for(int i = 0; i < Users.size(); ++i)
            {
                TimerThreadUser *pUser = Users.at(i);
                if (pUser->MsgFreeze(currTicks, extInfo))
                {
                    WatchdogForThread(pUser, extInfo);                 
                }
            }
        }

        m_lastWatchdogTime = currTicks;
    }


    // ---------------------------------------------------------

    void TimerThread::ProcessTimersList()
    {
        Utils::Locker lock(m_listLock);

        dword currTicks = SyncTimer::Ticks();

        int timersDone = 0;
        List::iterator i = m_list.begin();

        while(i != m_list.end())
        {
            if ((*i)->Process(currTicks))
            {
                timersDone++;
                ++i;
                continue;
            }

            break;
        }

        // delete/reload inactive timers
        if (timersDone) RemoveTimers(timersDone);
    }

    // ---------------------------------------------------------

    void TimerThread::RemoveTimers(int timersDone)
    {
        List copyList;

        // move timersDone elements from m_list to copyList
        while(timersDone--)
        {
            ESS_ASSERT(!m_list.empty());
            copyList.push_back(m_list.front());
            m_list.pop_front();
        }

        // process copyList
        List::iterator i = copyList.begin();
        dword currTicks = SyncTimer::Ticks();

        while(i != copyList.end())
        {
            if ((*i)->TryRestart(currTicks))
            {
                AddTimerToList((*i));                    
            }

            ++i;
        }

    }

    // ---------------------------------------------------------

    void TimerThread::AddTimerToList(IMsgTimer *pT)
    {
        dword currTicks = SyncTimer::Ticks();
        dword waitTime = pT->GetWaitTime(currTicks);

        List::iterator i = m_list.begin();

        // таймера в списке упор€дочены в пор€дке возрастани€ времени срабатывани€
        while(i != m_list.end())
        {
            dword currWaitTime = (*i)->GetWaitTime(currTicks);
            if (currWaitTime >= waitTime)
            {
                m_list.insert(i, pT);
                return;
            }

            ++i;
        }

        m_list.push_back(pT);
    }

    // ---------------------------------------------------------

    void TimerThread::StopTimer(IMsgTimer *pT)
    {
        List::iterator i = Find(pT);

        if (i != m_list.end())
        {
            (*i)->SafeStop();
            m_list.erase(i);
        }
    }

    // ---------------------------------------------------------

    void TimerThread::TimerStart(IMsgTimer *pT, dword interval, bool repeated)  // override
    {
        // thread-safety
        Utils::Locker lock(m_listLock);

        // stop timer
        StopTimer(pT);

        // start timer
        dword currTicks = SyncTimer::Ticks();
        pT->SafeStart(currTicks, interval, repeated);

        // add to list
        AddTimerToList(pT);
    }

    // ---------------------------------------------------------

    void TimerThread::TimerStop(IMsgTimer *pT)  // override
    {
        // thread-safety
        Utils::Locker lock(m_listLock);

        // stop timer
        StopTimer(pT);            
    }

    // ---------------------------------------------------------

    void TimerThread::RemoveTimersFor(MsgObjectBase *pObject)
    {
        // thread-safety
        Utils::Locker lock(m_listLock);

        // process list
        List::iterator i = m_list.begin();

        while(i != m_list.end())
        {
            (*i)->NotifyMsgObjectDestroyed(pObject);

            ++i;
        }
    }


    // ---------------------------------------------------------

    void TimerThread::BreakAndWait()
    {
        if (!isRunning()) return;

        m_break = true;
        wait();
    }

    // ---------------------------------------------------------

    TimerThreadUser::TimerThreadUser(int msgMaxTime) :         
        m_lastCounter(0),
        m_lastTime(0),
        m_msgMaxTime(msgMaxTime),
        m_thread(GetTimerThread(this))
    {
        CounterClear();
    }

    TimerThreadUser::~TimerThreadUser() 
    { 
        FreeTimerThread(this); 
    }

    bool TimerThreadUser::MsgFreeze( dword currTicks, std::string &extraInfo )
    {        
        if (m_msgMaxTime <= 0) return false;

        int currCounter = m_msgProcessCounter.Get();

        if (currCounter == 0)
        {
            m_lastCounter = 0;
            return false;
        }

        // counter don't change, check timeout
        if (currCounter == m_lastCounter)
        {
            if (currTicks - m_lastTime > m_msgMaxTime)
            {
                m_msgMaxTime = -1;  // disable timeout (for test mode only)
                extraInfo = "";
                if ((CTryExtractInfoFromMsg) && (m_pMsg != 0))
                {
                    extraInfo = m_pMsg->TypeName(); // typeid(*m_pMsg).name();
                }
                return true;
            }
        }
        else // update counter and it time
        {
            m_lastCounter = currCounter;
            m_lastTime = currTicks;
        }

        return false;
    }

}  // namespace iCore

