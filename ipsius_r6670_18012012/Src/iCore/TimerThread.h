#ifndef __TIMERTHREAD__
#define __TIMERTHREAD__

#include "stdafx.h"
#include "Platform/PlatformThread.h"
#include "Utils/ILockable.h"
#include "Utils/AtomicTypes.h"
#include "Utils/ErrorsSubsystem.h"
#include "ITimerManager.h"
#include "SyncTimer.h"

namespace iCore
{

    class TimerThread;
    class MsgObjectBase;
    class MsgBase;

    int GetMaxMessageTimeMs();
	
    /* 
        RAII �������, ������� ��������� "�����������" ������������� TimerThread

     ����� ����, ����� �������� ����������� ������� ��� ��������� MsgThread. 
     �� ����������, ���� ����� ������� �����, ���������������� ����� ���������� ������� ���������
     ������������ ��� ����������� �������, ������� ������� �� ��������� ������ ��������� --
     ��� ����� ����� ������� ��� ���� ������������������ TimerThreadUser ������������ �������� MsgFreeze()

     ��������! MsgFreeze() ����� ���� ������ �� ������� ���������� ���� ������������ TimerThreadUser
     */
    class TimerThreadUser
    {
        Utils::AtomicInt m_msgProcessCounter; 
        MsgBase *m_pMsg;  // warning! access to object thru this pointer is really dangerous!

        int m_lastCounter;
        dword m_lastTime;
        int m_msgMaxTime;
        TimerThread &m_thread; // must be last

    public:
        TimerThreadUser(int msgMaxTime = GetMaxMessageTimeMs());
        ~TimerThreadUser();

        bool MsgFreeze(dword currTicks, std::string &extraInfo);
        TimerThread& getThread() { return m_thread; }

        void SetMaxMessageProcessingTimeMs(int interval)
        {
            m_msgMaxTime = interval;
        }

        void CounterNext(MsgBase *pMsg)
        {
            m_pMsg = pMsg;
            m_msgProcessCounter.Inc();

            // ����������� �������������� -- ���� �������� ��� ������� �� ������������ ���������
            // if (m_msgProcessCounter.Get() == 0) m_msgProcessCounter.Inc();
        }

        void CounterClear()
        {
            m_pMsg = 0;
            m_msgProcessCounter.Set(0);
        }
    };

    // ---------------------------------------------------

    // �����, ������������� ��� ������� (MsgTimer) � �������
    // �����-��������, � ������� ���������� ������ ���� ���������, ���� ����� 
    // ����������� ���� �� ���� MsgThread
    class TimerThread : 
        Platform::Thread,
        public ITimerManager
    {

        // class List : public std::list<IMsgTimer*>, public Utils::ILockable {};
        typedef std::list<IMsgTimer*> List;

        List m_list;
        Utils::ILockable m_listLock;

        volatile bool m_break;
        dword m_lastWatchdogTime;
        
        List::iterator Find(IMsgTimer *p)
        {
            return std::find(m_list.begin(), m_list.end(), p);
        }
        
        void run();  // override QThread::run

        void ProcessTimersList();
        void ProcessWatchdog();
        void RemoveTimers(int timersDone);
        void AddTimerToList(IMsgTimer *pT);
        void StopTimer(IMsgTimer *pT);
        void WatchdogForThread(TimerThreadUser *pUser, const std::string &extInfo);
       
    // ITimerManager implementation
    private:

        void TimerStart(IMsgTimer *pT, dword interval, bool repeated); 
        void TimerStop(IMsgTimer *pT);  

    public:

        ESS_TYPEDEF_FULL(SuppressWatchdogException, ESS::HookRethrow);

        TimerThread() : 
          Platform::Thread("iCore::TimerThread"),
          m_break(false),
          m_lastWatchdogTime(SyncTimer::Ticks())
        {            
            start(Platform::Thread::HighPriority);
        }

        ~TimerThread()
        {
            BreakAndWait();
        }

        void BreakAndWait();

        void RemoveTimersFor(MsgObjectBase *pObject);

    };


}  // namespace iCore


#endif

