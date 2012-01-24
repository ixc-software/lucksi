#ifndef __APPLED__
#define __APPLED__

#include "ModeLed.h"
#include "E1AppBody.h"

namespace E1App
{

    template<int CLedNumber>
    struct TLed
    {
        static void Set(bool on)
        {
            DevIpTdm::BfLed::Set(CLedNumber, on);   
        }
    };

    // ---------------------------------------------------------------
    
    // states: idle -> PC connected -> PC connected + Device OK
    class LedE1
    {
        enum State
        {
            StIdle,
            StConnected,
            StConnectedDevice,
        };

        ModeLed< TLed<0> > m_led;
        State m_state;
        Utils::TimerTicks m_t;

        void EnterState(State state)
        {
            m_state = state;

            if (state == StIdle)                m_led.ModeOff();
            if (state == StConnected)           m_led.Mode(1000, 1000);
            if (state == StConnectedDevice)     m_led.ModeOn();
        }

        State GetState(E1AppBody *pBody)
        {
            if (pBody == 0) return StIdle;
            return pBody->DeviceState() ? StConnectedDevice : StConnected;
        }

    public:

        LedE1() 
        {
            EnterState(StIdle);
            m_t.Set(500, true);
        }

        void Update(dword currTicks, E1AppBody *pBody)
        {
            if (m_t.TimeOut())
            {
                State curr = GetState(pBody);
                if (curr != m_state) EnterState(curr);
            }

            m_led.Update(currTicks);
        }
    };

    // ------------------------------

    // states: no calls -> calls
    class LedCalls
    {
        ModeLed< TLed<1> > m_led;
        bool m_calls;
        Utils::TimerTicks m_t;

        void EnterState(bool calls)
        {
            m_calls = calls;

            if (m_calls) m_led.Mode(400, 200);
                    else m_led.Mode(200, 3000);
        }

    public:

        LedCalls()
        {
            EnterState(false);
            m_t.Set(500, true);
        }

        void Update(dword currTicks, E1AppBody *pBody)
        {
            if (m_t.TimeOut())
            {
                bool calls = false;
                if ((pBody != 0) && (pBody->ActiveChannelsCount() > 0)) calls = true;

                if (calls != m_calls) EnterState(calls);
            }

            m_led.Update(currTicks);
        }

    };
    
}  // namespace E1App

#endif
