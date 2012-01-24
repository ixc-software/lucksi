#ifndef _CHECK_GENERATOR_H_
#define _CHECK_GENERATOR_H_

#include "stdafx.h"
#include "iLog/iLogSessionCreator.h"
#include "iRtpUtils/IRtpTest.h"
#include "iRtpUtils/IRtpTestFactory.h"
#include "CheckGenStat.h"
#include "CheckGenProfile.h"

namespace iCore{    class MsgThread; };
namespace iRtp {    class RtpPcSession; };

namespace TestRtpWithSip
{   

    // Выполняет запись в файл пока не прийдет отбой
	class CheckGenerator : public boost::noncopyable,
        public iRtpUtils::IRtpTest
    {   
	public:
		CheckGenerator(CheckGenStat &stat,
			const CheckGenProfile &profile);
        static const std::string Name;
    // IRtpTest
    private:
		void StartSend(const iMedia::Codec &sendCodec);
		void StartReceive(const iMedia::Codec &receiveCodec);
		void ReceiveData(iRtp::SharedBdirBuff buff, iRtp::RtpHeaderForUser header);
    private:
        bool m_isStarted;
        CheckGenStat &m_stat;
        CheckGenProfile m_profile;
        std::vector<Platform::byte> m_buffer;
        int m_currentPos;
        bool m_skipState;
    };

    class CheckGeneratorFactory : boost::noncopyable,
        public iRtpUtils::IRtpTestFactory
    {   
    public:
        CheckGeneratorFactory(CheckGenStat &stat, const CheckGenProfile &profile) :
            m_stat(stat),
            m_profile(profile)
        {}
    private:
        iRtpUtils::IRtpTest *CreateTest(iCore::MsgThread& thread, 
            Utils::SafeRef<iRtp::RtpPcSession> rtp)
        {
            return new CheckGenerator(m_stat, m_profile);
        }
    private:
        CheckGenStat &m_stat;
        CheckGenProfile m_profile;
    };
};

#endif

