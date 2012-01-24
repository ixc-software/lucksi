#ifndef __AOZTDMOUTPUT__
#define __AOZTDMOUTPUT__

#include "Platform/PlatformTypes.h"
#include "Utils/StatisticElement.h"

#include "iCmp/ChMngProto.h"

namespace DrvAoz
{
    using Platform::byte;
    using Platform::word;
    using Platform::dword;

    using iCmp::AbCofidecParams;

    // ----------------------------------------------------------

    /*

        ѕодача вызова происходит с применением алгоритма оптимизации,
        который пытаес€ минимизировать количество одновременно звон€щих 
        каналов. 

        ѕараметры алгоритма оптимизации:

            int maxRingDelay -- максимальный временной интервал (мс), на который
            может быть задержано начало звонка в цел€х минимизации нагрузки
            
            int searchRingPoint -- количество точек, на который будет разбит
            maxRingDelay; именно через поиск в этих точках будет пытатьс€
            производитьс€ минимизаци€ нагрузки

            int maxActiveRings -- максимальное число активных звонков при
            которых будет предприниматьс€ попытка оптимизации

       ѕример:

       maxRingDelay = 500
       searchRingPoint = 3
       maxActiveRings = 6

       ≈сли число активных звонков больше 6, то оптимизационный поиск 
       примен€тьс€ Ќ≈ будет.

       ƒл€ временных точек 0, 250, 500 (три точки) будет расчитано количество
       активных звонков в них. ƒл€ нового звонка будет выбрана така€ временна€
       точка (т.е. задержка начала звонка), в которой количество активных
       звоноков минимально. 

    */

    // ----------------------------------------------------------

    class Ring
    {
        const int m_chNum;
        const int m_ringMs;
        const int m_silenceMs;

        bool m_firstUpdate;

        // current state
        bool m_ringing;
        int m_timeRemain;

        // debug
        boost::shared_ptr<std::string> m_trace;
        Utils::StatElementForInt m_callStats;
        Platform::dword m_prevCallTime, m_prevStateChange;

        void AddDebugInfo(bool changed)
        {
            Platform::dword t = Platform::GetSystemTickCount();

            // call time
            if (!m_firstUpdate)
            {
                m_callStats.Add(t - m_prevCallTime);
            }
            m_prevCallTime = t;

            // trace
            if (changed)
            {
                std::ostringstream oss;
                oss << (m_ringing ? "^" : "v") << (t - m_prevStateChange) << "; ";
                *m_trace += oss.str();
                m_prevStateChange = t;
            }
        }

    public:

        Ring(int chNum, int ringMs, int silenceMs, int initialDelayMs, bool debugTrace) :
          m_chNum(chNum), m_ringMs(ringMs), m_silenceMs(silenceMs), 
          m_firstUpdate(true)
        {
            ESS_ASSERT(chNum < 16);
            ESS_ASSERT(ringMs > 0);
            ESS_ASSERT(silenceMs > 0);

            if (initialDelayMs > 0)
            {
                m_ringing = false;
                m_timeRemain = initialDelayMs;
            }
            else
            {
                m_ringing = true;
                m_timeRemain = m_ringMs;
            }

            if (debugTrace) 
            {
                m_trace.reset( new std::string() );
                m_trace->reserve(1024);
                m_prevStateChange = Platform::GetSystemTickCount();
            }
        }

        // return true on state change
        bool Update(int timeIntervalMs)
        {
            bool prevState = m_ringing;

            int fullPeriod = m_ringMs + m_silenceMs;
            if (timeIntervalMs >= fullPeriod)
            {
                int div = timeIntervalMs / fullPeriod;
                timeIntervalMs -= (div * fullPeriod);
            }

            while(timeIntervalMs > 0)
            {
                if (timeIntervalMs < m_timeRemain)
                {
                    m_timeRemain -= timeIntervalMs;
                    break;
                }

                // change state
                timeIntervalMs -= m_timeRemain;

                if (m_ringing)
                {
                    m_ringing = false;
                    m_timeRemain = m_silenceMs;
                }
                else
                {
                    m_ringing = true;
                    m_timeRemain = m_ringMs;
                }
            }

            bool stateChanged = (prevState != m_ringing);

            // debug
            if (m_trace != 0)
            {
                AddDebugInfo(stateChanged);
            }

            // result
            bool first = m_firstUpdate;
            m_firstUpdate = false;

            return (stateChanged || first);
        }

        bool Ringing() const { return m_ringing; }
        int Channel() const { return m_chNum; }

        boost::shared_ptr<std::string> DebugTrace()
        {
            // add stats
            if (m_trace != 0)
            {
                *m_trace += "; " + m_callStats.ToString(Utils::StatElementForInt::LevFull);
            }

            return m_trace;
        }

    };

    // ----------------------------------------------------------

    // TDM output for AOZ board
    class AozTdmOutput
    {
        enum 
        {
            COutputSize = 16,   // of words

            CTotalChannels = 16,

            // TDM channels map
            CChControlDW = 0,
            CChTskLo = 5,
            CChTskHi = 13,
            CChRingLo = 6,
            CChRingHi  = 14,
            CChTalLo = 7,
            CChTalHi = 15,
        };

        enum CacheState
        {
            CsEmpty,
            CsRingChanded,
            CsOK,
        };

        const int m_ringUpdateIntervalMs;

        // ring optimisation
        const int m_maxRingDelay;
        const int m_searchRingPoint;
        const int m_maxActiveRings;

        // original data
        AbCofidecParams m_cofidec;
        word m_TSK;
        word m_RING;
        word m_TAL;

        // cached TDM output (from original data)
        CacheState m_cached;
        word m_output[COutputSize];

        // active rings
        typedef std::list<Ring> RingList;
        RingList m_rings;

        static byte WordLo(word val)
        {
            return val & 0xff;
        }

        static byte WordHi(word val)
        {
            return (val >> 8);
        }

        void OutputWord(word val, int indxLo, int indxHi)
        {
            ESS_ASSERT(indxLo < COutputSize);
            ESS_ASSERT(indxHi < COutputSize);

            m_output[indxLo] = WordLo(val);
            m_output[indxHi] = WordHi(val);            
        }

        void OutputDword(dword val, int ch)
        {
            ESS_ASSERT(ch + 4 <= COutputSize);

            byte *pSrc = (byte*)&val;
            word *pDst = &m_output[ch];

            *pDst++ = *pSrc++;
            *pDst++ = *pSrc++;
            *pDst++ = *pSrc++;
            *pDst++ = *pSrc++;
        }

        void UpdateCache()
        {
            ESS_ASSERT(m_cached != CsOK);

            // update RING
            OutputWord(m_RING, CChRingLo, CChRingHi);

            // update all stuff
            if (m_cached == CsEmpty)
            {
                OutputWord(m_TAL, CChTalLo, CChTalHi);
                OutputWord(m_TSK, CChTskLo, CChTskHi);
                OutputDword(m_cofidec.TS1_4, CChControlDW);
            }

            m_cached = CsOK;
        }

        void UpdateStateWord(word &w, int chNum, bool bitSet)
        {
            ESS_ASSERT(chNum < CTotalChannels);
            int mask = 1 << chNum;

            word prev = w;

            if (bitSet) w |= mask;
            else        w &= ~mask;

            // value don't changed
            if (w == prev) return;

            // cache already completly incorrect
            if (m_cached == CsEmpty) return;

            // m_cached in [CsRingChanded, CsOK]
            m_cached = (&w == &m_RING) ? CsRingChanded : CsEmpty;
        }

        int RingsForDelay(int delay)
        {
            int rings = 0;

            RingList::const_iterator i = m_rings.begin();

            while(i != m_rings.end())
            {
                Ring r = *i;  // copy ring!
                r.Update(delay);
                if (r.Ringing()) ++rings;

                ++i;
            }


            return 0;
        }

        int FindBestNewRingOffset()
        {
            if ( m_maxRingDelay == 0 || m_searchRingPoint <= 1 || 
                m_rings.size() > m_maxActiveRings ) return 0;

            int step = m_maxRingDelay / (m_searchRingPoint - 1);
            if (step <= 0) return 0;

            int currDelay = 0;

            int bestDelay = -1;
            int bestDelayRings = 0;

            while(currDelay <= m_maxRingDelay)
            {
                int currDelayRings = RingsForDelay(currDelay);

                if ((bestDelay < 0) || (currDelayRings < bestDelayRings))
                {
                    bestDelay = currDelay;
                    bestDelayRings = currDelayRings;
                }

                currDelay += step;
            }

            return (bestDelay < 0) ? 0 : bestDelay;
        }

        RingList::iterator FindRing(int chNum)
        {
            RingList::iterator i = m_rings.begin();

            while(i != m_rings.end())
            {
                if (i->Channel() == chNum) return i;

                ++i;
            }

            return m_rings.end();
        }

    public:

        // ringUpdateIntervalMs is TDM IRQ freq
        AozTdmOutput(const AbCofidecParams &cofidec, int ringUpdateIntervalMs, 
            int maxRingDelay, int searchRingPoint, int maxActiveRings) : 
          m_ringUpdateIntervalMs(ringUpdateIntervalMs),
          m_maxRingDelay(maxRingDelay),
          m_searchRingPoint(searchRingPoint),
          m_maxActiveRings(maxActiveRings),
          m_cofidec(cofidec),
          m_TSK(0),
          m_RING(0),
          m_TAL(0),
          m_cached(CsEmpty)
        {
            // clear m_output
            for(int i = 0; i < COutputSize; ++i) m_output[i] = 0;
        }

        enum
        {
            CTdmOutputFrameSize = COutputSize,
        };

        void WriteOutput(word *pOutput)  // pOutput point to buffer word[CTdmOutputFrameSize]
        {
            if (m_cached != CsOK) UpdateCache();
            std::memcpy(pOutput, &m_output, sizeof(m_output));
        }

        // called with m_ringUpdateIntervalMs freq
        void UpdateRings()
        {
            RingList::iterator i = m_rings.begin();

            while(i != m_rings.end())
            {
                if (i->Update(m_ringUpdateIntervalMs))
                {
                    UpdateStateWord(m_RING, i->Channel(), i->Ringing());
                }

                ++i;
            }
        }

        void StartRing(int chNum, int ringMs, int silenceMs, bool debugTrace)
        {
            if (FindRing(chNum) != m_rings.end())
            {
                StopRing(chNum);
            }

            int delayMs = FindBestNewRingOffset();
            m_rings.push_back( Ring(chNum, ringMs, silenceMs, delayMs, debugTrace) );
        }

        // return debug trace
        boost::shared_ptr<std::string> StopRing(int chNum)
        {
            boost::shared_ptr<std::string> debugTrace;

            RingList::iterator i = FindRing(chNum);
            if (i == m_rings.end()) return debugTrace;

            UpdateStateWord(m_RING, i->Channel(), false);  // turn ring off
            debugTrace = i->DebugTrace();                  // get trace 

            m_rings.erase(i);

            return debugTrace;
        }

        void StartTsk(int chNum)
        {
            UpdateStateWord(m_TSK, chNum, true);
        }

        void StopTsk(int chNum)
        {
            UpdateStateWord(m_TSK, chNum, false);
        }

        void StartTal(int chNum)
        {
            UpdateStateWord(m_TAL, chNum, true);            
        }

        void StopTal(int chNum)
        {
            UpdateStateWord(m_TAL, chNum, false);            
        }

        void SetCofidec(const AbCofidecParams &cofidec)
        {
            m_cofidec = cofidec;
            m_cached = CsEmpty;
        }

    };
       
}  // namespace DrvAoz


#endif
