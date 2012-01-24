#ifndef _PLAY_VECTOR_H_
#define _PLAY_VECTOR_H_

#include "stdafx.h"
#include "iLog/iLogSessionCreator.h"
#include "IDataSrcForRtpTest.h"
#include "IRtpTestFactory.h"
#include "PlayData.h"

namespace iRtpUtils
{    
	struct CheckGenProfile;

    // Выполняет запись в файл пока не прийдет отбой
    class PlayVector : boost::noncopyable,
        public IDataSrcForRtpTest    
    {   
    public:
		typedef std::vector<Platform::byte> Data;

		PlayVector(boost::shared_ptr<Data>, bool useAlaw);
        static const std::string Name;
    // IDataSrcForRtpTest
    private:
        void Setup(const iMedia::Codec &codec);
        QByteArray Read(int size);
    private:
		boost::shared_ptr<Data> m_data;
        bool m_useAlaw; 
        int m_pos;
    };

	//-----------------------------------------------------------------------

    class PlayVectorFactory : boost::noncopyable,
        public IRtpTestFactory
    {   
    public:
		PlayVectorFactory(const PlayVector::Data &data, bool useAlaw) :
            m_data(new PlayVector::Data(data)), m_useAlaw(useAlaw)
        {}
		PlayVectorFactory(int freq, int power, bool aLaw);
    private:
        IRtpTest *CreateTest(iCore::MsgThread& thread, 
            Utils::SafeRef<iRtp::RtpPcSession> rtp)
        {
            return  
                new PlayData(thread, rtp, new PlayVector(m_data, m_useAlaw));
        }
    private:
		boost::shared_ptr<PlayVector::Data> m_data;
        bool m_useAlaw;
    };
};

#endif
