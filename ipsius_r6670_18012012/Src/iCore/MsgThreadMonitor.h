#ifndef __MSGTHREADMONITOR__
#define __MSGTHREADMONITOR__

#include "Utils/ErrorsSubsystem.h"
#include "Utils/IBasicInterface.h"
#include "MsgThread.h"
#include "MsgTimer.h"
#include "MsgObject.h"

namespace iCore
{

    class IMsgThreadMonitorCallback : public Utils::IBasicInterface
    {
    public:

        virtual void CallbackForException(const std::exception *pE, bool *pSuppressIt) = 0;
    };
	
    /*
         ласс, отслеживающий состо€ние MsgThread.
        √лавна€ задача -- обнаружение "перегрузки" системы.

         ритерии оценки

        #1. ¬ очереди трида находитс€ количество сообщений, превышающее некий
        (заданный извне) порог

        #2. ѕревышение в течении N опросов подр€д задержки таймера на величину MaxTimerLag

        #3. –ост числа сообщений в течении N запросов, при привышении некоторого максимального порога,
        (при этом поток не должен ложитьс€ спать)

    */
    class MsgThreadMonitor :  public MsgObject, boost::noncopyable
	{       

    public:

        MsgThreadMonitor(MsgThread &thread, 
                         int pollInterval = CDefaultPollInterval, 
                         IMsgThreadMonitorCallback *pCallback = 0) : 
          MsgObject(thread),
          m_thread(thread), 
          m_pCallback(pCallback),
          m_pollingInterval(pollInterval), 
          m_timer(this, &MsgThreadMonitor::OnTimer),
		  m_assertExceptions(false)
        {
            ESS_ASSERT(thread.InCurrentThreadContext());

            m_pollCounter = 0;
            m_timer.Start(m_pollingInterval, true);

            Clear();
        }

        void Clear()
        {
            // #1 mode
            // m_maxMsgQueueSize = -1;
            ResetMaxMsgQueueSize();

            // #2 mode
            // m_timerLagCount = -1;
            // m_maxTimerLag = -1;
            ResetMaxTimerLag();

            m_startTime = SyncTimer::Ticks();
            m_insideLagCounter = 0;
            m_sumLag = 0;

            // #3 mode
            // m_msgGrowCount = -1;
            // m_msgGrowLimit = -1;
            ResetMsgGrowDetection();

            // #3 mode runtime
            m_prevMsgCountInQueue = 0;
            m_prevSleepCounter = 0;
            m_insideMsgGrowCounter = 0;
        }

        void Disable()
        {
            m_timer.Stop();
        }
        
        void SetMaxMsgQueueSize(int msgCount) // #1 mode
        {
            m_maxMsgQueueSize = msgCount;
        }

        void ResetMaxMsgQueueSize()
        {
            m_maxMsgQueueSize = -1;
        }

        int MaxMsgQueueSize() const { return m_maxMsgQueueSize; }

        void SetMaxTimerLag(int timerLagCount, int maxTimerLag)  // #2 mode
        {
            m_timerLagCount = timerLagCount;
            m_maxTimerLag = maxTimerLag;
        }
        
        void ResetMaxTimerLag()
        {
            m_timerLagCount = -1;
            m_maxTimerLag = -1;
        }

        int MaxTimerLagCount() const { return m_timerLagCount; }
        int MaxTimerLag() const { return m_maxTimerLag; }


        void SetMsgGrowDetection(int msgGrowCount, int msgGrowLimit)
        {
            m_msgGrowCount = msgGrowCount;
            m_msgGrowLimit = msgGrowLimit;
        }

        void ResetMsgGrowDetection()
        {
            m_msgGrowCount = -1;
            m_msgGrowLimit = -1;
        }

        int MsgGrowCount() const { return m_msgGrowCount; }
        int MsgGrowLimit() const { return m_msgGrowLimit; }

        std::string getInfo() const
        {
            return m_thread.GetStats().ToString();
        }

        ESS_TYPEDEF(BasicException);

        ESS_TYPEDEF_FULL(MaxMessages,   BasicException);
        ESS_TYPEDEF_FULL(TimerLag,      BasicException);
        ESS_TYPEDEF_FULL(MsgGrow,       BasicException);

    private:

        enum
        {
            CDefaultPollInterval = 1000,
        };

        void CheckMaxTimerLag()
        {
            int lag = (SyncTimer::Ticks() - m_startTime) - CDefaultPollInterval;
            if (lag < 0) lag = -lag;

            m_sumLag += lag;

            if (lag > m_maxTimerLag)
            {
                m_insideLagCounter++;
                if (m_insideLagCounter >= m_timerLagCount) ESS_THROW(TimerLag);
            }
            else
            {
                m_insideLagCounter = 0;
            }
        }

        void CheckMsgGrow(const MsgThread::Stats &stats)
        {
            bool msgOverLimit = (stats.MsgInQueue > m_msgGrowLimit);

            if (msgOverLimit)
            {
                if (m_insideMsgGrowCounter > 0)
                {
                    if (m_prevSleepCounter != stats.WakeUps)
                    {
                        m_insideMsgGrowCounter = 0;
                        return;
                    }
                }

                m_insideMsgGrowCounter++;

                if (m_insideMsgGrowCounter >= m_msgGrowCount) ESS_THROW(MsgGrow);
            }
            else
            {
                m_insideMsgGrowCounter = 0;
            }
        }


        void ProcessChecks(const MsgThread::Stats &stats)
        {
            // #1 mode
            if (m_maxMsgQueueSize > 0)
            {
                if (stats.MsgInQueue >= m_maxMsgQueueSize) ESS_THROW(MaxMessages);
            }

            // #2 mode
            if (m_timerLagCount > 0)
            {
                CheckMaxTimerLag();
            }

            // #3 mode
            if (m_msgGrowCount > 0)
            {
                CheckMsgGrow(stats);

                m_prevMsgCountInQueue = stats.MsgInQueue;
                m_prevSleepCounter = stats.WakeUps;
            }
        }

        void OnTimer(MsgTimer *pT)
        {
            try
            {
                MsgThread::Stats stats = m_thread.GetStats();

                ProcessChecks(stats);

                // update poll variables
                m_pollCounter++;
                m_startTime = SyncTimer::Ticks();
            }
            catch(BasicException &e)
            {
                bool ignore = false;

                if (m_pCallback != 0)
                {
                    m_pCallback->CallbackForException(&e, &ignore);
                }

                if (!ignore) 
                {
                    OnMonitoringError( getMsgThread() );
                    ESS_UNEXPECTED_EXCEPTION(e);
                }
            }

        }

        void OnMonitoringError(iCore::MsgThread &t);

        MsgThread &m_thread;
        IMsgThreadMonitorCallback *m_pCallback;
        int m_pollingInterval;      // MsgThread polling interval
        MsgTimer m_timer;
        bool m_assertExceptions;
        int m_pollCounter;

        // #1 mode
        int m_maxMsgQueueSize;      // limit messages in thread queue

        // #2 mode
        int m_timerLagCount;        // repeated polls count for timer lag
        int m_maxTimerLag;          // max timer lag

        // #2 mode, runtime
        int m_startTime;
        int m_insideLagCounter;
        long m_sumLag;

        // #3 mode
        int m_msgGrowCount;         // repeated polls count for message grow in queue
        int m_msgGrowLimit;         // minimal msg in queoe limit for detection msg grow

        // #3 mode runtime
        int m_prevMsgCountInQueue;
        long m_prevSleepCounter;
        int m_insideMsgGrowCounter;

	};
	
	
}  // namespace iCore

#endif

