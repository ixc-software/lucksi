#ifndef _RTP_FLOW_ANALIZER_H_
#define _RTP_FLOW_ANALIZER_H_

#include "stdafx.h"
#include "iCore/MsgThread.h"
#include "iCore/MsgObject.h"
#include "iCore/MsgTimer.h"
#include "Utils/SafeRef.h"
#include "iLog/iLogSessionCreator.h"
#include "iLog/LogWrapper.h"
#include "IRtpTest.h"
#include "IRtpTestFactory.h"

namespace iCore{    class MsgThread; };
namespace iRtp {    class RtpPcSession; };

namespace TestRtpWithSip
{    
    // Выполняет запись в файл пока не прийдет отбой
	class RtpFlowAnalyzer : boost::noncopyable,
        public iCore::MsgObject,
		public IRtpTest
    {   
        typedef RtpFlowAnalyzer T;
	public:
        RtpFlowAnalyzer(iCore::MsgThread &thread, 
            Utils::SafeRef<iRtp::RtpPcSession> rtp,
            iLogW::ILogSessionCreator &log,
            int reportTimeout,
            const QByteArray &dump);
        static const std::string Name;
    // IRtpTest
    private:
		void StartSend(const Sip::Codec &sendCodec);
		void StartReceive(const Sip::Codec &receiveCodec);
		void ReceiveData(iRtp::SharedBdirBuff buff, iRtp::RtpHeaderForUser header);
        std::string Reset();
    private:
        void OnTimeout(iCore::MsgTimer *pT);
        std::string Report(Platform::ddword timeout) const;
        void ResetStat();
    private:
        iCore::MsgTimer m_timer;
        Utils::SafeRef<iRtp::RtpPcSession> m_rtp;
        boost::scoped_ptr<iLogW::LogSession>  m_log;
        iLogW::LogRecordTag m_infoTag;
        iLogW::LogRecordTag m_errorTag;
		bool m_isStarted;
        Sip::Codec m_codec;
        int m_reportTimeout;
        QByteArray m_model;
        QByteArray m_recDump;
        int m_pos;
       
        class TimeoutsStat : boost::noncopyable
        {
        public:
            TimeoutsStat() {    Reset();            }
            Platform::ddword &Min() { return m_min; }
            Platform::ddword Min() const { return m_min; }
            Platform::ddword &Max() { return m_max; }
            Platform::ddword Max() const { return m_max; }
            Platform::ddword &Average() { return m_average; }
            Platform::ddword Average() const { return m_average; }
            
            void Reset()
            {
                m_min = ~0;
                m_max = 0;
                m_average = 0;
            }
        private:
            Platform::ddword m_min;
            Platform::ddword m_max;
            Platform::ddword m_average;
        };
        

        Platform::ddword m_startTick;
        Platform::ddword m_prevTick;
        
        TimeoutsStat m_timeouts;
        Platform::ddword m_blockCount;
        Platform::ddword m_allBytes;
        Platform::ddword m_maxCountGoodBytes;
        Platform::ddword m_minCountGoodBytes;
        Platform::ddword m_maxCountBadBytes;
        Platform::ddword m_minCountBadBytes;
        Platform::ddword m_countGoodBytes;
        Platform::ddword m_countBadBytes;
    };

    class RtpFlowAnalyzerFactory : boost::noncopyable,
        public IRtpTestFactory
    {   
    public:
        RtpFlowAnalyzerFactory(iCore::MsgThread &thread,
            iLogW::ILogSessionCreator &log, 
            int reportTimeout,
            const QByteArray &dump);
    private:
        IRtpTest *CreateTest(iCore::MsgThread& thread, 
            Utils::SafeRef<iRtp::RtpPcSession> rtp);
    private:
        iCore::MsgThread &m_thread;
        iLogW::ILogSessionCreator &m_log;
        int m_reportTimeout;
        QByteArray m_dump;
    };

};

#endif

