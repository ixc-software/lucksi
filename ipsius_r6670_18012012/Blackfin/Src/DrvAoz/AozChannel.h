#ifndef __AOZCHANNEL__
#define __AOZCHANNEL__

#include "AozTdmOutput.h"
#include "AozShared.h"
#include "AozChStats.h"
#include "AozChPulse.h"

namespace DrvAoz
{
    using iCmp::AbLineParams;

    // ----------------------------------------------------
    
    // сигнальный канал абонентской платы AOZ
    class AozChannel : public IAozChannel
    {
        IChannelOwner &m_owner;
        AozChStats m_stats;
        const int m_chNumber;
        const int m_chNumberMask;

        bool m_enabled; 
        AozChPulse m_pulse;

        bool ChannelInMask(int mask) const
        {
            return (mask & m_chNumberMask) ? true : false;
        }

    // IAozChannel impl
    private:

        void OnLineConditionChanged(bool damaged)
        {
            using namespace iCmp;
            std::string params = ChMngProtoParams::ToParams(damaged);
            AozEvent ev(m_chNumber, BfAbEvent::CLineCondition(), params);
            m_owner.PushChannelEvent(ev);
        }

        void OnLineReset()
        {
            using namespace iCmp;
            AozEvent ev(m_chNumber, BfAbEvent::CLineReset(), "");
            m_owner.PushChannelEvent(ev);
        }

        void OnDigit(int pulseCount, int digitCount)
        {
            // pulse count to digit
            if (pulseCount > 10 || pulseCount == 0)
            {
                // log - ?
                // ...
                return;
            }
            int digit = pulseCount;
            if (pulseCount == 10) digit = 0;

            // send
            using namespace iCmp;
            std::string params = ChMngProtoParams::ToParams(digit);
            AozEvent ev(m_chNumber, BfAbEvent::CLinePulseDial(), params);
            m_owner.PushChannelEvent(ev);
        }

        void OnDigitBegin(int digitCount)
        {
            using namespace iCmp;
            std::string params = ChMngProtoParams::ToParams(digitCount);
            AozEvent ev(m_chNumber, BfAbEvent::CLinePulseDialBegin(), params);
            m_owner.PushChannelEvent(ev);
        }

        void OnHook(bool up)
        {
            using namespace iCmp;
            std::string params = ChMngProtoParams::ToParams(up);
            AozEvent ev(m_chNumber, BfAbEvent::CLineState(), params);
            m_owner.PushChannelEvent(ev);

            // forced stop ringing
            if (up)
            {
                m_owner.TdmOutput().StopRing(m_chNumber);
            }
        }


        void PulseTrace(const std::string &s)
        {
            // nothing
            // ... 
        }

    public:

        AozChannel(IChannelOwner &owner, const AbLineParams &lineSetup, 
            int chNumber, int pulsePeriodMs) : 
          m_owner(owner),
          m_chNumber(chNumber),
          m_chNumberMask(1 << chNumber),
          m_pulse(*this, pulsePeriodMs, lineSetup)
        {
            m_enabled = false;
        }

        int Number() const { return m_chNumber; }

        bool SetLineParams(int chMask, const iCmp::AbLineParams &params)
        {
            if (!ChannelInMask(chMask)) return false;

            m_pulse.SetupParams(params);

            return true;
        }

        bool Command(int chMask, const std::string &cmd, const std::string &params)
        {
            if (!m_enabled) return false;
            if (!ChannelInMask(chMask)) return false;

            typedef iCmp::PcCmdAbCommand Cmds;
            typedef iCmp::ChMngProtoParams Params;

            if (cmd == Cmds::CStartRing())
            {
                int ringMs, silenceMs;

                if (!Params::FromParams(params, ringMs, silenceMs))
                {
                    ESS_THROW_T(TdmMng::TdmException, TdmMng::terBadAbChCommand); 
                }

                m_owner.TdmOutput().StartRing( m_chNumber, ringMs, silenceMs, 
                                               m_owner.DebugTraceEnabled() );

                return true;
            }

            // all other commands don't have params
            if (!params.empty())
            {
                ESS_THROW_T(TdmMng::TdmException, TdmMng::terBadAbChCommand); 
            }

            if (cmd == Cmds::CStopRing())
            {
                boost::shared_ptr<std::string> trace;
                trace = m_owner.TdmOutput().StopRing(m_chNumber);

                if (trace != 0)
                {
                    m_owner.ChannelLog(*trace);
                }

                return true;
            }

            if (cmd == Cmds::CStartTest())
            {
                m_owner.TdmOutput().StartTsk(m_chNumber);
                return true;
            }

            if (cmd == Cmds::CStopTest())
            {
                m_owner.TdmOutput().StopTsk(m_chNumber);
                return true;
            }

            if (cmd == Cmds::CSwitchToTestLineOn())
            {
                m_owner.TdmOutput().StartTal(m_chNumber);
                return true;
            }

            if (cmd == Cmds::CSwitchToTestLineOff())
            {
                m_owner.TdmOutput().StopTal(m_chNumber);
                return true;
            }

            ESS_THROW_T(TdmMng::TdmException, TdmMng::terBadAbChCommand); 
            return false;  // dummi
        }

        void Enable(bool enabled)
        {
            m_enabled = enabled;

            if (m_enabled) m_pulse.InitRecv();
        }

        void ProcessPulse(bool hook, bool pulse)
        {
            if (!m_enabled) return;

            m_pulse.Process(hook, pulse);
        }

    };
    
    
}  // namespace DrvAoz

#endif
