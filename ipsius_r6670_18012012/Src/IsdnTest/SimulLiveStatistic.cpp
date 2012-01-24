#include "stdafx.h"
#include "SimulLiveStatistic.h"
#include "Utils/ErrorsSubsystem.h"
#include "Platform/Platform.h"

namespace IsdnTest
{
    void SimulLiveStatistic::Start()
    {
        ESS_ASSERT(m_state == st_initial);
        m_state = st_run;
        m_startTime = Platform::GetSystemTickCount();
    }

    void SimulLiveStatistic::IncrTalkCall(dword talkTime)
    {
        ESS_ASSERT(talkTime != 0);
        if (st_finish == m_state) return; // ignore
        ESS_ASSERT(st_run == m_state);
        m_totalTalkTime += talkTime;
        ++m_numberTalkCall;
    }

    void SimulLiveStatistic::IncrDropedCall()
    {
        if (st_finish == m_state) return; // ignore
        ESS_ASSERT(st_run == m_state);
        ++m_numberDropedCall;
    }

    void SimulLiveStatistic::IncrRefused()
    {            
        if (st_finish == m_state) return; // ignore
        ESS_ASSERT(st_run == m_state);
        ++m_numberRefused;
    }

    void SimulLiveStatistic::Finish()
    {
        ESS_ASSERT(st_run == m_state);
        m_state = st_finish;
        m_finishTime = Platform::GetSystemTickCount();
    }

    std::string SimulLiveStatistic::getAsString()
    {
        ESS_ASSERT(m_state != st_initial);

        std::stringstream ss;
        ss << "\nStatistic:" 
            << "\n\tNumber of talk(connection): " << m_numberTalkCall
            << "\n\tTotall talk-time(s): " << m_totalTalkTime / 1000
            << "\n\tAverage talk-time(s): " << m_totalTalkTime / m_numberTalkCall / 1000.0
            << "\n\tRefused call (no free channel): " << m_numberRefused
            << "\n\tDroped call: " << m_numberDropedCall;
        if (m_state == st_finish)
            ss << "\n\tTotal time of test(s): " << (m_finishTime - m_startTime) / 1000 << std::endl;
        else
            ss << "\n\tElapsed time(s): " << ( Platform::GetSystemTickCount() - m_startTime ) / 1000 << std::endl;

        return ss.str();
    }
}
