#include "stdafx.h"

#include "Utils/ErrorsSubsystem.h"
#include "Utils/StatisticElement.h"
#include "Utils/ManagedList.h"
#include "echo.h"

#include "EchoCancel.h"

using boost::scoped_ptr;

// --------------------------------------

namespace
{

    void CoefToVector(const int16_t *pData, int dataCount, std::vector<int> &result)
    {
        result.clear();
        result.reserve(dataCount);

        for(int i = 0; i < dataCount; ++i)
        {
            result.push_back( *pData++ );
        }
    }

}  // namespace

// --------------------------------------

namespace iDSP
{

    class EchoCancel::EchoState
    {
        echo_can_state_t *m_p;

    public:

        EchoState(IEchoDebug &owner, int len, int adaption_mode, bool enableDebug)
        {
            m_p = echo_can_create(len, adaption_mode);

            if (enableDebug)
            {
                m_p->EchoDebug = &owner;
            }
        }

        echo_can_state_t* State() { return m_p; }

        virtual ~EchoState()
        {            
            echo_can_free(m_p);
        }
    };

    // --------------------------------------

    class AdoptStatsItem
    {
        scoped_ptr< std::vector<int> > m_coefBefore, m_coefAfter;

        int m_number;
        int m_factor;

        Utils::StatisticElement<int, Platform::int64> m_diff;
        Utils::StatisticElement<double, double> m_diffPercent;

        static double CoefDiffPercent(int valBefore, int valAfter)
        {
            ESS_ASSERT(valBefore != 0);

            return 100 * ((valAfter - valBefore) / (double)valBefore);
        }

        void CalcParams()
        {
            ESS_ASSERT( m_coefBefore->size() == m_coefAfter->size() );

            for(int i = 0; i < m_coefBefore->size(); ++i)
            {
                int valBefore = m_coefBefore->at(i);
                int valAfter  = m_coefAfter->at(i);

                // just diff
                m_diff.Add(valAfter - valBefore);

                // percent
                if (valBefore != 0)
                {
                    m_diffPercent.Add( CoefDiffPercent(valBefore, valAfter) );                
                }
                
            }
        }

    public:

        AdoptStatsItem(int number, int factor, 
            std::vector<int> *pCoefBefore, std::vector<int> *pCoefAfter) :
            m_coefBefore(pCoefBefore), m_coefAfter(pCoefAfter),
            m_number(number), m_factor(factor)
        {
            CalcParams();
        }

        double GetDiffPercentAvg() const
        {
            if (m_diffPercent.Empty()) return 0;

            return std::abs( m_diffPercent.Average() );
        }

        void ToString(std::string &s)
        {
            using std::endl;
            using std::setw;

            std::ostringstream ss;

            ss << "#" << m_number << "; factor " << m_factor << endl;

            for(int i = 0; i < m_coefBefore->size(); ++i)
            {
                int valBefore = m_coefBefore->at(i);
                int valAfter  = m_coefAfter->at(i);

                ss << "[" << setw(3) << i << "]   ";
                ss << setw(8) << valBefore << "  ";
                ss << setw(8) << valAfter  << "  ";

                if (valBefore != 0)
                {
                    ss << setw(16) << CoefDiffPercent(valBefore, valAfter) << "% ";
                }

                ss << endl;
            }

            s = ss.str();
        }
    };

    // ------------------------------------------------------

    class EchoCancel::AdoptStats : boost::noncopyable
    {
        Utils::ManagedList<AdoptStatsItem> m_items;
        int m_maxItems;

        static double GetItemValue(AdoptStatsItem *pItem)
        {
            return pItem->GetDiffPercentAvg();
        }

        // Algoritm not optimised. We can store min value in object state
        bool TryAdd(AdoptStatsItem *pItem)
        {
            if (m_items.Size() < m_maxItems)
            {
                m_items.Add(pItem);
                return true;
            }

            // find min item
            double minValue = GetItemValue(m_items[0]);
            int minIndx = 0;

            for(int i = 1; i < m_items.Size(); ++i)
            {
                double curr = GetItemValue(m_items[i]);
                if (curr < minValue)
                {
                    minValue = curr;
                    minIndx = i;
                }
            }

            // compare agaist new item
            if (GetItemValue(pItem) < minValue) return false;

            // add
            m_items.Set(minIndx, pItem);

            return true;
        }


    public:

        AdoptStats(int maxItems = 32) : m_maxItems(maxItems)
        {
            // ...
        }

        void Add(AdoptStatsItem *pItem)
        {
            if (!TryAdd(pItem)) delete pItem;          
        }

        void ToString(std::string &s)
        {
            for(int i = 0; i < m_items.Size(); ++i)
            {
                std::string item;
                m_items[i]->ToString(item);

                s += item;
                s += "\n";
            }
        }
    };
        
}  // namespace Utils


// --------------------------------------

namespace iDSP
{
        
    EchoCancel::EchoCancel(int tapsSize, bool enableAdoptDebug) :
        m_adoptCounter(0), m_pCoefBeforeAdopt(0)
    {
        m_echo.reset( new EchoState(*this, tapsSize, ECHO_CAN_USE_ADAPTION, enableAdoptDebug) );
        //m_echo.reset();

        if (enableAdoptDebug)
        {
            m_adoptStats.reset( new AdoptStats() );
        }
    }

    EchoCancel::~EchoCancel()
    {
        delete m_pCoefBeforeAdopt;        
    }

    int EchoCancel::Process( int tx, int rx )
    {
        return echo_can_update(m_echo->State(), tx, rx);
    }

    void EchoCancel::GetFirCoeffs(std::vector<int> &foreground, std::vector<int> &background)
    {
        echo_can_state_t *p = m_echo->State();
        CoefToVector(p->fir_state.coeffs,    p->taps, foreground);
        CoefToVector(p->fir_state_bg.coeffs, p->taps, background);
    }

    int EchoCancel::BgAdoptCalled() const
    {
        return m_echo->State()->BgAdoptCalled;
    }

    void EchoCancel::OnAdoptEnter( int factor )
    {
        m_adoptFactor = factor;

        ESS_ASSERT(m_pCoefBeforeAdopt == 0);

        echo_can_state_t *p = m_echo->State();
        m_pCoefBeforeAdopt = new std::vector<int>();
        CoefToVector(p->fir_state_bg.coeffs, p->taps, *m_pCoefBeforeAdopt);
    }

    void EchoCancel::OnAdoptLeave()
    {
        echo_can_state_t *p = m_echo->State();
        std::vector<int> *pCoefAfter = new std::vector<int>();
        CoefToVector(p->fir_state_bg.coeffs, p->taps, *pCoefAfter);

        // AdoptStatsItem delete before and after coefs
        // AdoptStats delete item
        m_adoptStats->Add( new AdoptStatsItem(m_adoptCounter, m_adoptFactor, m_pCoefBeforeAdopt, pCoefAfter));
        m_pCoefBeforeAdopt = 0;

        m_adoptCounter++;
    }

    void EchoCancel::AdoptStatToString( std::string &s )
    {
        ESS_ASSERT(m_adoptStats.get() != 0);

        m_adoptStats->ToString(s);
    }

}  // namespace iDSP


