#ifndef _CHECK_GENERATOR_STAT_H_
#define _CHECK_GENERATOR_STAT_H_

#include "stdafx.h"
#include "Platform/PlatformTypes.h"

namespace TestRtpWithSip
{   
    struct CheckGenStat
    {
        CheckGenStat()
        {
            m_skippedSample = 0;
            m_processedSample = 0;
            m_checkCount = 0;
            m_errorCheck = 0;
            m_errorFreq = 0;
            m_errorPower = 0;
        }
            
        std::string ToString() const
        {
            std::ostringstream out;            
            out << "Skipped samples: " << m_skippedSample 
                << "; Processed samples: " << m_processedSample 
                << "; Check count: " << m_checkCount << "; Check error: " << m_errorCheck
                << "; Error frequency: " << m_errorFreq 
                << "; Error power: " << m_errorPower << std::endl;
            return out.str();
        }
        int m_checkCount;
        Platform::dword m_skippedSample;
        Platform::dword m_processedSample;
        int m_errorCheck;
        int m_errorFreq;
        int m_errorPower;
    };
};

#endif

