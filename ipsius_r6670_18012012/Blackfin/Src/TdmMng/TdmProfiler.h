#ifndef __TDMPROFILER__
#define __TDMPROFILER__

#include "BfTdm/TdmProfile.h"
#include "BfDev/BfTimerCounter.h"
#include "BfDev/SysProperties.h"
#include "iVDK/CriticalRegion.h"
#include "Utils/StatisticElement.h"

namespace TdmMng
{
    using Platform::dword;
    
    class TdmProfiler : public BfTdm::ITdmProfiler, boost::noncopyable
    {
        BfDev::BfTimerCounter m_timer;
        Utils::StatElementForInt m_statsBase, m_statsExt;
        dword m_extStartTime;
        bool m_beginFixed, m_beginExtFixed;

    // BfTdm::ITdmProfiler impl
    private:

        void OnProcessingBegin()
        {
            ESS_ASSERT(!m_beginFixed);

            m_beginFixed = true;
            m_timer.Reset();
        }

        void OnProcessingEnd()
        {
            ESS_ASSERT(m_beginFixed);

            m_beginFixed = false;
            m_statsBase.Add( m_timer.GetCounter() );
        }

    public:

        struct Info
        {
            int Count;
            int AvgTimeMcs;
            int MaxTimeMcs;

            Info()
            {
                Count = 0;
                AvgTimeMcs = 0;
                MaxTimeMcs = 0;
            }
        };

        TdmProfiler() : 
          m_timer( BfDev::BfTimerCounter::GetFreeTimer() ),
          m_beginFixed(false),
          m_beginExtFixed(false)
        {
        }

        void ExtBegin()
        {
            ESS_ASSERT(m_beginFixed);

            ESS_ASSERT(!m_beginExtFixed);

            m_beginExtFixed = true;
            m_extStartTime = m_timer.GetCounter();
        }

        void ExtEnd()
        {
            ESS_ASSERT(m_beginFixed);

            ESS_ASSERT(m_beginExtFixed);

            m_beginExtFixed = false;
            m_statsExt.Add( m_timer.GetCounter() - m_extStartTime );
        }

        Info GetInfo(bool base) const
        {
            Utils::StatElementForInt s;

            // safe copy m_stats -> s
            {
                iVDK::CriticalRegion cr;
                s = base ? m_statsBase : m_statsExt;
            }

            // make info
            Info i;
            i.Count = s.Count();
            
            if (i.Count > 0)
            {
                int freq = BfDev::SysProperties::Instance().getFrequencySys();
                i.AvgTimeMcs = BfDev::BfTimerCounter::TimerCounterToMcs(s.Average(), freq);
                i.MaxTimeMcs = BfDev::BfTimerCounter::TimerCounterToMcs(s.Max(),     freq);
            }

            return i;
        }

    };
    
}  // namespace TdmMng

#endif
