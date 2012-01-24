
#ifndef __SBPTIMEOUTSPROCESSOR__
#define __SBPTIMEOUTSPROCESSOR__

// SbpTimeoutsProcessor.h

#include "Utils/TimerTicks.h"
#include "SbpTypes.h"

namespace SBProto
{
    // Using for prosess SafeBiProto timeouts
    // If timeouts is 0 they're not processed
    class SbpTimeoutsProcessor : boost::noncopyable
    {
        const SbpSettings &m_settings;
        
        Utils::TimerTicks m_waitRecvEndTimer;
        Utils::TimerTicks m_waitRespTimer;

        bool m_recvEndTimerStarted;
        bool m_respTimerStarted;
        
    public:
        /*
        SbpTimeoutsProcessor(dword recvEndTimeoutMs, dword respTimeoutMs) : 
            m_recvEndTimeoutMs(recvEndTimeoutMs), m_respTimeoutMs(respTimeoutMs), 
            m_recvEndTimerStarted(false), m_respTimerStarted(false)
        {
            
        }
        */

        SbpTimeoutsProcessor(const SbpSettings &settings) : 
            m_settings(settings), m_recvEndTimerStarted(false), m_respTimerStarted(false)
        {
        }

        // start if !started
        void StartRecvEndTimer()
        {
            if (m_recvEndTimerStarted) return;
            
            // m_waitRecvEndTimer.Set(m_recvEndTimeoutMs, false);
            m_waitRecvEndTimer.Set(m_settings.getReceiveEndTimeoutMs(), false);
            m_recvEndTimerStarted = true;
        }
        
        void StopRecvEndTimer()
        {
            m_waitRecvEndTimer.Stop();
            m_recvEndTimerStarted = false;
        }

        bool RecvEndTimeout()
        { 
            // if ((m_recvEndTimeoutMs == 0) || (!m_recvEndTimerStarted)) return false;
            if ((m_settings.getReceiveEndTimeoutMs() == 0) || (!m_recvEndTimerStarted)) return false;

            return m_waitRecvEndTimer.TimeOut();
        }

        // assert that !started
        void StartResponceTimer()
        {
            ESS_ASSERT(!m_respTimerStarted);
            // m_waitRespTimer.Set(m_respTimeoutMs, false);
            m_waitRespTimer.Set(m_settings.getResponceTimeoutMs(), false);
            m_respTimerStarted = true;
        }
        
        void StopResponceTimer()
        {
            m_waitRespTimer.Stop();
            m_respTimerStarted = false;
        }
        
        bool ResponceTimeout()
        {
            // if ((m_respTimeoutMs == 0) || (!m_respTimerStarted)) return false;
            if ((m_settings.getResponceTimeoutMs() == 0) || (!m_respTimerStarted)) return false;
            
            return m_waitRespTimer.TimeOut();
        }

        void ResetAll()
        {
            m_waitRecvEndTimer.Reset();
            m_waitRespTimer.Reset();

            m_recvEndTimerStarted = false;
            m_respTimerStarted = false;
        }
    };
    
} // namespace SBProto

#endif
