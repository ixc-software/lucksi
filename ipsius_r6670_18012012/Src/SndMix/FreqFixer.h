#ifndef __FREQFIXER__
#define __FREQFIXER__

#include "Utils/IntToString.h"

#include "CyclicBuff.h"

namespace SndMix
{
    
    class FreqFixer : public IFreqFixer
    {
        const bool m_debug; 
        const int m_workPeriodSamples;

        int m_periodCount;
        int m_nextPeriodLevel;
        int m_currRwDiff;

        static std::string Value(int samples)
        {
            std::ostringstream oss;
            oss << samples << " samples (" << (samples / 8) << " ms)";
            return oss.str();
        }

        void Trace(const std::string &s)
        {
            std::cout << s << std::endl;
        }

    // IFreqFixer impl
    private:

        void OnReadCompleted(const StateInfo &info)
        {
            if (info.ReadCount < m_nextPeriodLevel) return;

            int prevRwDiff = m_currRwDiff;

            // change state
            ++m_periodCount;
            m_nextPeriodLevel = info.ReadCount + m_workPeriodSamples;
            m_currRwDiff = info.ReadCount - info.WriteCount;

            // process
            int diff = m_currRwDiff - prevRwDiff;

            if (m_debug)
            {
                Trace(Utils::IntToString(m_periodCount) + ": diff " + Value(diff));
            }

            if (m_periodCount == 1) return;  // skip first

        }

    public:

        FreqFixer(bool debug, int workPeriodMs) : 
          m_debug(debug), 
          m_workPeriodSamples(workPeriodMs * 8), 
          m_periodCount(0),
          m_nextPeriodLevel(m_workPeriodSamples),
          m_currRwDiff(0)
        {
        }
    };
    
    
}  // namespace SndMix

#endif
