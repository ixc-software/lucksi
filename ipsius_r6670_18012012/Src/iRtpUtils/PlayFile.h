#ifndef _PLAY_FILE_RTP_TEST_H_
#define _PLAY_FILE_RTP_TEST_H_

#include "stdafx.h"
#include "Utils/ErrorsSubsystem.h"
#include "Utils/MsWave.h"
#include "iLog/iLogSessionCreator.h"
#include "IDataSrcForRtpTest.h"
#include "IRtpTestFactory.h"
#include "PlayData.h"



namespace iRtpUtils
{        
    class PlayRawFile : boost::noncopyable,
        public IDataSrcForRtpTest
    {   
    public:      
        PlayRawFile(const QString &name);
        static const std::string Name;

    // IDataSrcForRtpTest
    private:
        void Setup(const iMedia::Codec &codec);
        QByteArray Read(int size);
    private:
        QFile m_file;
    };

    // ------------------------------------------------------------------------------------


    class PlayWavFile : boost::noncopyable,
        public IDataSrcForRtpTest
    {   
    public:      
        PlayWavFile(const QString &name);
        static const std::string Name;

    // IDataSrcForRtpTest
    private:
        void Setup(const iMedia::Codec &codec);
        QByteArray Read(int size);
    private:
        Utils::MsWaveReader m_wave;
        Utils::MsWaveReader::Iterator m_pos;
        iMedia::Codec m_codec;
    };

    // ------------------------------------------------------------------------------------


    class PlayFileFactory : boost::noncopyable,
        public IRtpTestFactory
    {   
    public:
        PlayFileFactory(const QString &name, bool waveFormat) : 
          m_waveFormat(waveFormat),
          m_name(name){}
    private:
        IRtpTest *CreateTest(iCore::MsgThread& thread, 
            Utils::SafeRef<iRtp::RtpPcSession> rtp)
        {
            IDataSrcForRtpTest *dataSource;

            if (m_waveFormat) dataSource = new PlayWavFile(m_name);
            else dataSource = new PlayRawFile(m_name);

            return new PlayData(thread, rtp, dataSource);
        }
    private:
        QString m_name;
        bool m_waveFormat;
    };
};

#endif
