#ifndef __AOZCHLINEFILTER__
#define __AOZCHLINEFILTER__

#include "AozChLineFilter.h"

namespace DrvAoz
{
    
    // filter PULSE + HOOK -> PULSE + line damage detection 
    class AozChLineFilter
    {
        bool m_damaged;

        // period stats
        int m_counter;
        int m_counterMs;
        int m_damageCount;

        void BeginPeriod()
        {
            m_counter = 0;
            m_counterMs = 0;
            m_damageCount = 0;
        }

    public:

        AozChLineFilter()
        {
            InitRecv();
        }

        void InitRecv()
        {
            m_damaged = false;
            BeginPeriod();
        }

        bool LineDamaged() const
        {
            return m_damaged;
        }

        // return true if line damaged state changed
        bool Process(int pulsePeriodMs,
            const iCmp::AbLineParams &lineSetup,
            bool hook, bool pulse, bool &outPulse)
        {
            outPulse = pulse; // hook ignored

            if (!lineSetup.LineDamageDetectionEnable) return false;

            // period accumulate
            ++m_counter;
            m_counterMs += pulsePeriodMs;
            if (hook != pulse) ++m_damageCount;
            if (m_counterMs < lineSetup.LineDamageSwitchPeriodMs) return false;

            // period result
            int damagePercent = (m_damageCount * 100) / m_counter;
            int percentToCompare = m_damaged ? (100 - damagePercent) : damagePercent;
            bool changed = (percentToCompare >= lineSetup.LineDamageSwitchPercent);
            BeginPeriod();

            if (changed) m_damaged = !m_damaged;

            return changed;
        }
                
    };
    
    
}  // namespace DrvAoz

#endif
