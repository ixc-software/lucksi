#ifndef SIMULLIVESTATISTIC_H
#define SIMULLIVESTATISTIC_H

#include "stdafx.h"
#include "Platform/PlatformTypes.h"

namespace IsdnTest
{
    using Platform::dword;

    class SimulLiveStatistic
    {        
        enum State
        {
            st_initial,
            st_run,
            st_finish
        };

        State m_state;
        dword m_startTime;
        dword m_finishTime;

        int m_numberTalkCall;  
        int m_totalTalkTime;

        int m_numberDropedCall; // количество созданных вызовов заведомо завершающихся на этапе установки

        int m_numberRefused;        

        friend class NObjSimulLiveSettings;

        SimulLiveStatistic()
            : m_state(st_initial),
            m_numberTalkCall(0),
            m_totalTalkTime(0),
            m_numberDropedCall(0),
            m_numberRefused(0)
        {}
        
    public:                

        void Start();

        void IncrTalkCall(dword talkTime);

        void IncrDropedCall();

        void IncrRefused();

        void Finish();

        std::string getAsString();
    };
} // namespace IsdnTest

#endif


