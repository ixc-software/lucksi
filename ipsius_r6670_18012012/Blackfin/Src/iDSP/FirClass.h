#ifndef __FIRCLASS__
#define __FIRCLASS__

#include <fract.h>
#include <filter.h>

#include "Platform/PlatformTypes.h"

#include "fir.h"

namespace iDSP
{
    using Platform::int16;
    
    class FirClass
    {
        fir16_state_t m_state;

        std::vector<int16_t> m_coef;

    public:
        
        FirClass(const std::vector<int> &coef)
        {
            // copy coef -> m_coef
            m_coef.reserve( coef.size() );
            for(int i = 0; i < coef.size(); ++i) m_coef.push_back( coef.at(i) );

            // create filter
            fir16_create(&m_state, &m_coef[0], m_coef.size());
        }

        int16 Do(int16 x)
        {
            return fir16(&m_state, x);
        }

        void DoBlock(int16 *pIn, int16 *pOut, int count)
        {
            while(count--)
            {
                *pOut = Do(*pIn);

                pIn++;
                pOut++;
            }
        }

    };  
    
    // -------------------------------------------------------------

    class BfFirClass
    {
        fir_state_fr16 m_state;

        std::vector<fract16> m_coef;
        std::vector<fract16> m_history;

    public:

        BfFirClass(const std::vector<int> &coef)
        {
            // copy coef -> m_coef
            m_coef.reserve( coef.size() );
            for(int i = 0; i < coef.size(); ++i) m_coef.push_back( coef.at(i) );

            // reserve history
            m_history.reserve( coef.size() );
            for(int i = 0; i < coef.size(); ++i) m_history.push_back(0);
            
            // create filter
            fir_init(m_state, &m_coef[0], &m_history[0], m_coef.size(), 1);
        }

        int16 Do(int16 x)
        {
            fract16 valIn = x;
            fract16 valOut;

            fir_fr16(&valIn, &valOut, 1, &m_state);
            
            // if (valOut != 0) valOut--;

            return valOut;
        }

        void DoBlock(int16 *pIn, int16 *pOut, int count)
        {
            fir_fr16(pIn, pOut, count, &m_state);            
        }


    };
    
    
}  // namespace iDSP
 

#endif

