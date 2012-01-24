#include "stdafx.h"
#include "iLog/LogBinaryUtils.h"
#include "iLog/LogWrapperLoggers.h"
#include "iDSP/get_power.h"
#include "CheckGenerator.h"

namespace 
{    
    const int CBinaryMaxBytes = 50;
};

namespace TestRtpWithSip
{    
    CheckGenerator::CheckGenerator(CheckGenStat &stat, const CheckGenProfile &profile) :
        m_stat(stat),
        m_profile(profile),
        m_buffer(m_profile.m_countAnalisSample),
        m_currentPos(0)
    {
        m_isStarted = false;
        m_skipState = true;
    }

    const std::string CheckGenerator::Name("Check generator");

	// ------------------------------------------------
    // IRtpTest

	void CheckGenerator::StartSend(const iMedia::Codec &sendCodec)
	{
		/*ignore*/
	}

	// ------------------------------------------------

	void CheckGenerator::StartReceive(const iMedia::Codec &receiveCodec)
	{
		m_isStarted = true;
	}

	// ------------------------------------------------

	void CheckGenerator::ReceiveData(iRtp::SharedBdirBuff buff, iRtp::RtpHeaderForUser header)
	{
		if (!m_isStarted) return;
        
        int firstIndex = 0;

        if(m_skipState)
        {
            firstIndex = m_profile.m_countSkipSample - m_currentPos;
            m_currentPos += buff->Size();

            if(m_currentPos <= m_profile.m_countSkipSample) 
            {
                m_stat.m_skippedSample += buff->Size();
                return;
            }
            m_stat.m_skippedSample += firstIndex;
            m_skipState = false;
            m_currentPos = 0;
        }
        int size = m_buffer.size() - m_currentPos;
        if(size > buff->Size() - firstIndex) size = buff->Size() - firstIndex;

        for(int i = 0; i < size; ++i)
        {
            m_buffer.at(m_currentPos + i) = (*buff)[firstIndex + i];
        }
        m_currentPos += size;
        
        if(m_currentPos < m_buffer.size()) return;

        ESS_ASSERT(m_currentPos == m_buffer.size());

        m_stat.m_processedSample += m_buffer.size();
        m_stat.m_checkCount++;


        m_skipState = true;
        m_currentPos = 0;

        int freqRes;
        float powerRes;
        ESS_ASSERT(m_buffer.size());
        bool res = iDSP::GP_ProcessBuff(&m_buffer.at(0), m_buffer.size(), m_profile.m_useAlaw, &freqRes, &powerRes); 

        if(!res)
        {
            ++m_stat.m_errorCheck;
            return;
        }
        
        if(std::abs(m_profile.m_freq - freqRes) > 5) ++m_stat.m_errorFreq;
        
        if(std::abs(m_profile.m_power - powerRes) > 0.1) ++m_stat.m_errorPower;
	}
    
};

