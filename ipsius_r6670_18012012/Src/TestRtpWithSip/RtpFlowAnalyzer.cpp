#include "stdafx.h"
#include "iLog/LogWrapperLibrary.h"
#include "iRtp/RtpPcSession.h"
#include "RtpFlowAnalyzer.h"

namespace TestRtpWithSip
{    
    const std::string RtpFlowAnalyzer::Name("Analizator");

    RtpFlowAnalyzer::RtpFlowAnalyzer(iCore::MsgThread &thread, 
        Utils::SafeRef<iRtp::RtpPcSession> rtp,
        iLogW::ILogSessionCreator &log,
        int reportTimeout,
        const QByteArray &dump) : 
        iCore::MsgObject(thread),
        m_rtp(rtp),
        m_log(log.CreateLogSesion(Name, true)),
        m_infoTag(m_log->RegisterRecordKind(L"Info")),
        m_errorTag(m_log->RegisterRecordKind(L"Error")),
        m_timer(this, &T::OnTimeout),
        m_isStarted(false),
        m_codec(Sip::Codec::ALaw_8000),
        m_reportTimeout(reportTimeout),
        m_model(dump),
        m_pos(0)
	{
        ResetStat();
    }

	// ------------------------------------------------
    // IRtpTest

	void RtpFlowAnalyzer::StartSend(const Sip::Codec &sendCodec)
	{
		/*ignore*/
	}

	// ------------------------------------------------

	void RtpFlowAnalyzer::StartReceive(const Sip::Codec &receiveCodec)
	{
		m_isStarted = true;
        m_codec = receiveCodec;
        m_startTick = Platform::GetSystemTickCount();
        m_timer.Start(m_reportTimeout, true);
	}

    // ------------------------------------------------

	void RtpFlowAnalyzer::ReceiveData(iRtp::SharedBdirBuff buff, iRtp::RtpHeaderForUser header)
	{
		if (!m_isStarted) return;

        Platform::ddword currentTick = Platform::GetSystemTickCount();

        if(m_blockCount == 0) 
        {
            m_prevTick = currentTick;
        }
        else
        {
            Platform::ddword time = currentTick - m_prevTick;

            if (m_timeouts.Max() < time) m_timeouts.Max() = time;
            if (m_timeouts.Min() > time) m_timeouts.Min() = time;
            m_timeouts.Average() += time;
			
            m_prevTick = currentTick; 
        }

        ++m_blockCount;
        m_allBytes += buff->Size();

        for(int i = 0; i < buff->Size(); ++i)
        {
            m_recDump.push_back(buff->At(i));
        }
        
        int pos = m_recDump.indexOf(m_model);
        int nextPos = 0;
        while(pos != -1)
        {
            nextPos = pos + m_model.size();
            if(nextPos > m_recDump.size()) 
            {
                m_recDump = m_recDump.right(m_recDump.size() - pos);                
                break;
            }
            if(nextPos == m_recDump.size()) 
            {
                m_countGoodBytes += m_model.size();
                m_recDump.clear();
                break;
            }

            pos = m_recDump.indexOf(m_model, nextPos);
            if (pos == nextPos)
            {
                m_countGoodBytes += m_model.size();

                if(m_maxCountBadBytes == 0 ||
                    m_countBadBytes > m_maxCountBadBytes) m_maxCountBadBytes = m_countBadBytes;
                
                if(m_minCountBadBytes == 0 || 
                    m_countBadBytes < m_minCountBadBytes) m_minCountBadBytes = m_countBadBytes;
                
                m_countBadBytes = 0;
            }
            else
            {
                m_countBadBytes += pos - nextPos;

                if(m_maxCountGoodBytes ||
                    m_countGoodBytes > m_maxCountGoodBytes) m_maxCountGoodBytes = m_countGoodBytes;
                if(m_minCountGoodBytes == 0 ||
                    m_countGoodBytes < m_minCountGoodBytes) m_minCountGoodBytes = m_countGoodBytes;
                m_countGoodBytes = 0;
            }

        }
	}

    // ------------------------------------------------

    void RtpFlowAnalyzer::OnTimeout(iCore::MsgTimer *pT)
    {
        Platform::ddword currentTick = Platform::GetSystemTickCount();
    
        if(m_maxCountGoodBytes == 0) m_maxCountGoodBytes = m_countGoodBytes;
        if(m_minCountGoodBytes == 0) m_minCountGoodBytes = m_countGoodBytes;
        
        if(m_maxCountBadBytes == 0) m_maxCountBadBytes = m_countBadBytes;
        if(m_minCountBadBytes == 0) m_minCountBadBytes = m_countBadBytes;

        *m_log << m_infoTag << Report(currentTick - m_startTick);
        
        ResetStat();
            
        m_startTick = currentTick;
    }

    // ------------------------------------------------

    std::string RtpFlowAnalyzer::Report(Platform::ddword timeout) const
    {
        std::ostringstream out;
        // out << "Analysis information:" << std::endl;
        out << std::endl;
        out // << "Timeout size (ms) " << timeout << std::endl
            << "Block: " << m_blockCount << "; Receive bytes: " << m_allBytes << ";" << std::endl;

        out << "Timeout between block: Max = " << m_timeouts.Max()
            << ", Min = " << m_timeouts.Min();

        if (m_blockCount)
        {
            out << " Average = " << m_timeouts.Average()/m_blockCount;
        }
        out << ";" << std::endl
            << "Count good bytes: Max = " << m_maxCountGoodBytes 
            << ", Min = " << m_minCountGoodBytes << std::endl
            << "Count bad bytes:  Max = " << m_maxCountBadBytes
            << ", Min = " << m_minCountBadBytes << std::endl;

        out << "Rtp session information all stat: " << std::endl
            << m_rtp->getAllTimeStats().ToString() << std::endl;

        out << "Rtp session information LastSsrc: " << std::endl
            << m_rtp->getLastSsrcStats().ToString() << std::endl;


        return out.str();
    }

    // ------------------------------------------------

    void RtpFlowAnalyzer::ResetStat()
    {
        m_startTick = 0;
        m_prevTick = 0;
        m_blockCount = 0;
        m_allBytes = 0;
        m_maxCountGoodBytes = 0;
        m_minCountGoodBytes = 0;
        m_maxCountBadBytes = 0;
        m_minCountBadBytes = 0;
        m_countGoodBytes = 0;
        m_countBadBytes = 0;
        m_timeouts.Reset();
    }

    // ------------------------------------------------

    RtpFlowAnalyzerFactory::RtpFlowAnalyzerFactory( iCore::MsgThread &thread, iLogW::ILogSessionCreator &log, int reportTimeout, const QByteArray &dump ) : 
        m_thread(thread),
        m_log(log),
        m_reportTimeout(reportTimeout),
        m_dump(dump)
    {}

    // ------------------------------------------------

    IRtpTest *RtpFlowAnalyzerFactory::CreateTest(iCore::MsgThread& thread, Utils::SafeRef<iRtp::RtpPcSession> rtp)
    {
        return new RtpFlowAnalyzer(m_thread, rtp, m_log, m_reportTimeout, m_dump);
    }

};

