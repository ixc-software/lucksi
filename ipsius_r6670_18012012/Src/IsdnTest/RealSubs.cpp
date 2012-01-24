#include "stdafx.h"
#include "RealSubs.h"
#include "NObjSimulLiveSettings.h"

namespace IsdnTest
{
    
    void SubTalkByTime::RegCalls() // override
    {            
        if ( getOwner().GetRandom().Next(2) ) // 50% drop side
            SetCalls(new OutTalkByTime(*this, GenRndTalkTime(), m_prof), new InTalkByTime(*this, 0, m_prof));                
        else
            SetCalls(new OutTalkByTime(*this, 0, m_prof), new InTalkByTime(*this, GenRndTalkTime(), m_prof));

    }

    int SubTalkByTime::GenRndTalkTime()
    {
        Utils::Random& rnd = getOwner().GetRandom();
        return rnd.Next( m_prof.m_MaxTalkTime - m_prof.m_MinTalkTime ) + m_prof.m_MinTalkTime;
    }   
} // namespace

