#ifndef _GENERATE_SINUS_H_
#define _GENERATE_SINUS_H_

#include "stdafx.h"
#include "iLog/iLogSessionCreator.h"
#include "IDataSrcForRtpTest.h"
#include "IRtpTestFactory.h"
#include "PlayData.h"

#include "iDSP/g711gen.h"

namespace iRtpUtils
{    

    
    class GenerateSinus : boost::noncopyable,
        public IDataSrcForRtpTest    
    {           
    public:
        GenerateSinus(int amplitude, 
            int frec);
        static const std::string Name;
    // IDataSrcForRtpTest
    private:
        void Setup(const iMedia::Codec &codec);
        QByteArray Read(int size);
    // SinusArray
    private:
        class SinusArray 
        {
        public:
            SinusArray(int amplitude, int frec, const iMedia::Codec& codec);
            QByteArray Read(int size);
        private:
            QByteArray m_array;
            int m_pos;
        };
    private:
        int m_amplitude;
        int m_frec;
        boost::scoped_ptr<SinusArray> m_sinus;
    };

    class GenerateSinus_db: boost::noncopyable,
        public IDataSrcForRtpTest     
    {
        int m_freq;
        int m_dBm0;
        boost::scoped_ptr<iDSP::FreqGenerator> m_gen;
        QByteArray m_cash;

        // IDataSrcForRtpTest
    private:
        void Setup(const iMedia::Codec &codec);
        QByteArray Read(int size);

    public:
        GenerateSinus_db(int freq, int dBm0): m_freq(freq), m_dBm0(dBm0)
        {            
        }
    };

    class GenerateSinusFactory : boost::noncopyable,
        public IRtpTestFactory
    {   
    public:
        GenerateSinusFactory(/*iLogW::ILogSessionCreator &, */int amplitude, int frec, bool ampIndBm0 = false) : 
            m_amplitude(amplitude), 
            m_ampIndBm0(ampIndBm0), // amplitude in percent or in dBm0
            m_frec(frec){}
    public:
        IRtpTest *CreateTest(iCore::MsgThread& thread, 
            Utils::SafeRef<iRtp::RtpPcSession> rtp)
        {
            IDataSrcForRtpTest* pSrc = 0;
            if (m_ampIndBm0)
                pSrc = new GenerateSinus_db(m_frec,  m_amplitude);
            else
                pSrc = new GenerateSinus(m_amplitude, m_frec);

            return new PlayData(thread, rtp, pSrc);
        }

    private:
        int m_amplitude;
        bool m_ampIndBm0;
        int m_frec;
    };

    // ------------------------------------------------------------------------------------

    

    



    
};

#endif
