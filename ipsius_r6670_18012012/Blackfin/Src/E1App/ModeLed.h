#ifndef __MODELED__
#define __MODELED__

#include "Platform/Platform.h"
#include "Utils/TimerTicks.h"

namespace E1App
{    
    using Platform::dword;

    // TLed has method TLed::Set(bool on)
    template<class TLed>
    class ModeLed
    {
        int m_onMs; 
        int m_offMs;

        bool m_stateInOn;
        dword m_stateEnterTime;

        void EnterState(bool stateIsOn)
        {
            m_stateInOn = stateIsOn;
            m_stateEnterTime = Platform::GetSystemTickCount();

            TLed::Set(stateIsOn);
            // DevIpTdm::BfLed::Set(m_ledNum, stateIsOn);   
        }

    public:

        ModeLed()
        {
            ModeOff();
        }

        void ModeOn()
        {
            Mode(1, 0);
        }

        void ModeOff()
        {
            Mode(0, 1);
        }

        void Mode(int onMs, int offMs)
        {
            m_onMs = onMs; 
            m_offMs = offMs;

            EnterState((onMs > 0) ? true : false);
        }

        void Update(dword currTicks)
        {
            // locked in on or off state
            if ((m_onMs == 0) || (m_offMs == 0)) return;

            dword timeInState = currTicks - m_stateEnterTime;
            if (timeInState < (m_stateInOn ? m_onMs : m_offMs)) return;

            // switch state
            EnterState(!m_stateInOn);
        }

        void Update()
        {
            Update(Platform::GetSystemTickCount());
        }
    };

    
}  // namespace E1App

#endif
