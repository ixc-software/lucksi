#ifndef _CHECK_GENERATOR_PROFILE_H_
#define _CHECK_GENERATOR_PROFILE_H_

#include "stdafx.h"

namespace TestRtpWithSip
{   
    struct CheckGenProfile
    {
        CheckGenProfile()
        {
            m_countAnalisSample = 0;
            m_countSkipSample = 0;
            m_freq = 0;
            m_power = 0;  
            m_useAlaw = true;
        }
        int m_countAnalisSample;
        int m_countSkipSample;
        int m_freq;
        int m_power; 
        bool m_useAlaw;
    };
};

#endif

