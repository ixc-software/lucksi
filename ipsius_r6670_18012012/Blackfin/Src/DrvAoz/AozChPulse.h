#ifndef __AOZCHPULSE__
#define __AOZCHPULSE__

#include "Utils/IBasicInterface.h"
#include "iCmp/ChMngProto.h"

#include "AozChLineFilter.h"

namespace DrvAoz
{
    using iCmp::AbLineParams;

    class IAozChannel : public Utils::IBasicInterface
    {
    public:
        virtual void OnLineConditionChanged(bool damaged) = 0;
        virtual void OnLineReset() = 0;
        virtual void OnDigit(int pulseCount, int digitCount) = 0;
        virtual void OnHook(bool up) = 0;
        virtual void OnDigitBegin(int digitCount) = 0;

        virtual void PulseTrace(const std::string &s) = 0;
    };

    // ---------------------------------------------------------
    
    // PULSE processing
    class AozChPulse
    {

        struct FsmPulse
        {
            enum FsmState
            {
                St0     = 0,    // начальное
                St1     = 1,
                St11    = 11,
                St12    = 12,
                St13    = 13,
                St2     = 2,
                St21    = 21,   // отработка дребезга замыкания 
                St22    = 22,
                St23    = 23,   // свободен
            };

            FsmState State;
            FsmState PrevState;
            int TBreakMs;
            int TCloseMs;

            FsmPulse()
            {
                Clear();
            }

            void Clear()
            {
                State = St23;
                PrevState = St0;
                TBreakMs = 0;
                TCloseMs = 0;
            }


            void GotoState(FsmState state)
            {
                PrevState = state;
                State = state;
            }

        };

        struct FsmLine
        {
            enum FsmState
            {
                St1 = 1,
                St2,
                St3,
                St4,
                St5,
                St6,
            };

            FsmState State;
            int PulseCount;
            int DigitCount;

            FsmLine()
            {
                Clear();
            }

            void Clear()
            {
                State = St1;
                PulseCount = 0;
                DigitCount = 0;
            }
        };

        IAozChannel &m_owner;
        const int m_pulsePeriodMs;
        AbLineParams m_lineSetup;
        bool m_enableTrace;

        AozChLineFilter m_filter;
        FsmPulse m_pulse;
        FsmLine m_line;

        void FsmPulseMsgTrace(const char *p)
        {
            if (m_enableTrace) m_owner.PulseTrace(p);
        }

        void FsmLineMsgTrace(const char *p)
        {
            if (m_enableTrace) m_owner.PulseTrace(p);
        }

        void FsmPulseProcess(bool lineBreaked);

        void FsmPulseChangeState(FsmPulse::FsmState newState)
        {
            if (m_enableTrace) 
            {
                std::ostringstream oss;
                oss << "Pulse FSM " << (int)m_pulse.State << " -> " << (int)newState;
                m_owner.PulseTrace(oss.str());
            }

            m_pulse.GotoState(newState);
        }

        void FsmLineChangeState(FsmLine::FsmState newState)
        {
            if (m_enableTrace) 
            {
                std::ostringstream oss;
                oss << "Line FSM " << (int)m_line.State << " -> " << (int)newState;
                m_owner.PulseTrace(oss.str());
            }

            m_line.State = newState;
        }

    private:

        // устойчивое замыкания
        void MsgGoesClosed()
        {
            FsmPulseMsgTrace("MsgGoesClosed");

            if (m_line.State == FsmLine::St3)
            {
                m_line.PulseCount++;
                FsmLineChangeState(FsmLine::St4);

                if (m_line.PulseCount == 1)
                {
                    FsmPulseMsgTrace("OnDigitBegin");
                    m_owner.OnDigitBegin(m_line.DigitCount);
                }

                return;
            }

            if (m_line.State == FsmLine::St6)
            {
                m_line.DigitCount = 0;
                m_line.PulseCount = 0;

                // сброс линии
                {
                    FsmLineMsgTrace("LineReset!");
                    m_owner.OnLineReset();
                }

                FsmLineChangeState(FsmLine::St2);
                return;
            }
        }

        // межсерийная пауза
        void MsgDialPause()
        {
            FsmPulseMsgTrace("MsgDialPause");

            if (m_line.State == FsmLine::St4)
            {
                {
                    FsmLineMsgTrace("SendDigit");
                    m_owner.OnDigit(m_line.PulseCount, m_line.DigitCount);
                }

                m_line.DigitCount++;
                m_line.PulseCount = 0;
                FsmLineChangeState(FsmLine::St5);
                return;
            }

        }

        // трубка снята
        void MsgHookUp()
        {
            FsmPulseMsgTrace("MsgHookUp");

            if (m_line.State == FsmLine::St1)
            {
                m_line.DigitCount = 0;
                m_line.PulseCount = 0;

                {
                    FsmLineMsgTrace("HookUp");
                    m_owner.OnHook(true);
                }

                FsmLineChangeState(FsmLine::St2);
                return;
            }

        }

        // устойчивое размыкание
        void MsgGoesBreaked()
        {
            FsmPulseMsgTrace("MsgGoesBreaked");

            FsmLine::FsmState st = m_line.State;

            if (st == FsmLine::St2 || st == FsmLine::St4 || st == FsmLine::St5)
            {
                FsmLineChangeState(FsmLine::St3);
                return;
            }
        }

        // импульс
        void MsgDialPulse()
        {
            FsmPulseMsgTrace("MsgDialPulse");

            if (m_line.State == FsmLine::St3)
            {
                FsmLineChangeState(FsmLine::St6);
                return;
            }

        }

        // трубка уложена (на свое место)
        void MsgHookDown()
        {
            FsmPulseMsgTrace("MsgHookDown");

            if (m_line.State == FsmLine::St6)
            {
                {
                    FsmLineMsgTrace("HookDown");
                    m_owner.OnHook(false);
                }

                FsmLineChangeState(FsmLine::St1);                
                return;
            }
       }

        // ложное замыкание
        void MsgFalseCloseImpulse()
        {
            FsmPulseMsgTrace("MsgFalseCloseImpulse");

            // nothing
        }

        // ложное размыкание
        void MsgFalseBreakImpulse()
        {
            FsmPulseMsgTrace("MsgFalseBreakImpulse");

            // nothing
        }

    public:

        AozChPulse(IAozChannel &owner, int pulsePeriodMs, const AbLineParams &lineSetup,
            bool enableTrace = false) : 
          m_owner(owner),
          m_pulsePeriodMs(pulsePeriodMs),
          m_lineSetup(lineSetup),
          m_enableTrace(enableTrace)
        {
            InitRecv();
        }

        void SetupParams(const AbLineParams &lineSetup)
        {
            m_lineSetup = lineSetup;
            InitRecv();
        }

        void InitRecv()
        {
            m_filter.InitRecv();
            m_pulse.Clear();
            m_line.Clear();
        }

        void Process(bool hook, bool pulse)
        {
            // filter
            bool outPulse;

            if (m_filter.Process(m_pulsePeriodMs, m_lineSetup, hook, pulse, outPulse))
            {
                m_owner.OnLineConditionChanged( m_filter.LineDamaged() );
            }

            if (m_filter.LineDamaged()) return;

            // pulse
            FsmPulseProcess(outPulse);
        }
                
    };
    
}  // namespace DrvAoz


#endif
