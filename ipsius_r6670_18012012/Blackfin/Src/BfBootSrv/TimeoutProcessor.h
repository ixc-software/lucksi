#ifndef TIMEOUTPROCESSOR_H
#define TIMEOUTPROCESSOR_H

#include "Utils/TimerTicks.h"

namespace BfBootSrv
{
    class TimeoutProcessor
    {
        Utils::TimerTicks m_timer;
        bool m_started;
        const int m_timeout;

    public:

        TimeoutProcessor(int msecTimeout) : m_started(false), m_timeout(msecTimeout)
        {}
        
        void Start()
        {
            ESS_ASSERT(!m_started);            
            m_timer.Set(m_timeout, false);
            m_started = true;
        }

        void Stop()
        {
            m_timer.Stop();
            m_started = false;
        }

        bool TimeoutExpired()
        {            
            if (m_timeout == 0 || !m_started) return false;
            return m_timer.TimeOut();
        }
    };
} // namespace BfBootSrv

#endif
