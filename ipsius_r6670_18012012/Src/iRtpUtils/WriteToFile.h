#ifndef _WRITE_TO_FILE_RTP_TEST_H_
#define _WRITE_TO_FILE_RTP_TEST_H_

#include "stdafx.h"
#include "IRtpTest.h"
#include "IRtpTestFactory.h"

namespace iCore{    class MsgThread; };
namespace iRtp {    class RtpPcSession; };

namespace iRtpUtils
{    
  
    // Выполняет запись в файл пока не прийдет отбой
	class WriteToFile : public boost::noncopyable,
		public IRtpTest
    {   
	public:
		WriteToFile(const QString &name, bool convertToWave);
        ~WriteToFile();
        static const std::string Name;
    // IRtpTest
    private:
		void StartSend(const iMedia::Codec &sendCodec);
		void StartReceive(const iMedia::Codec &receiveCodec);
		void ReceiveData(iRtp::SharedBdirBuff buff, iRtp::RtpHeaderForUser header);
    private:
        iMedia::Codec m_receiveCodec;
		QFile m_file;
		bool m_isStarted;
        bool m_convertToWave;
        std::vector<int> m_linCash;
    };

    class WriteToFileFactory : boost::noncopyable,
        public IRtpTestFactory
    {   
    public:
        WriteToFileFactory(const QString &name, bool convertToWave) : 
          m_convertToWave(convertToWave),
          m_name(name){}
    private:
        IRtpTest *CreateTest(iCore::MsgThread& thread, 
            Utils::SafeRef<iRtp::RtpPcSession> rtp)
        {
            return new WriteToFile(m_name, m_convertToWave);
        }
    private:
        QString m_name;
        bool m_convertToWave;
    };

};

#endif

