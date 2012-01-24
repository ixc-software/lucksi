#include "stdafx.h"

#include "iDSP/g711gen.h"
#include "iDSP/g711codec.h"

#include "MsWave.h"

// ------------------------------------------------

namespace
{
    using Utils::MsWaveReader;
    using Utils::MsWaveBase;
    using Utils::MsWaveWriter;

    void RunTest1(QString fileName)
    {
        const int CSampleRate = 8000;
        const int CSamplesCount = CSampleRate * 10;

        // make wav 
        iDSP::LineFreqGenerator gen0(850, 0);
        iDSP::LineFreqGenerator gen1(331, -10);

        std::vector<int> ch0;
        std::vector<int> ch1;

        for(int i = 0; i < CSamplesCount; ++i)
        {
            ch0.push_back( gen0.Next() );
            ch1.push_back( gen1.Next() );
        }

        QByteArray wavOut = MsWaveWriter::MakeWavStreamTwoCh(MsWaveBase::Pcm, CSampleRate, 2, ch0, ch1);

        // save to disk
        if (fileName.length() > 0)
        {
            QFile f(fileName);
            ESS_ASSERT( f.open(QIODevice::WriteOnly | QIODevice::Truncate) );
            f.write(wavOut);
        }

        // read and verify
        {
            MsWaveReader wave(wavOut);

            MsWaveReader::Iterator i0(wave, 0);
            MsWaveReader::Iterator i1(wave, 1);

            TUT_ASSERT( i0.Size() == CSamplesCount );
            TUT_ASSERT( i1.Size() == CSamplesCount );

            int pos = 0;

            while(!i0.Eof())
            {
                if (i0.Get() != ch0.at(pos)) ESS_HALT("");
                if (i1.Get() != ch1.at(pos)) ESS_HALT("");

                // next
                i0.Next();
                i1.Next();

                ++pos;
            }

        }

    }

    void RunTest2(QString path)
    {
        const int CSampleRate = 8000;
        const int CSamplesCount = CSampleRate * 10;

        // make 1x PCM
        {
            iDSP::LineFreqGenerator gen0(330, 0);

            std::vector<int> ch0;
            ch0.reserve(CSamplesCount);

            for(int i = 0; i < CSamplesCount; ++i)
            {
                ch0.push_back( gen0.Next() );
            }

            MsWaveWriter w(path + "pcm_8k_16bit_1x_wr.wav", MsWaveBase::Pcm, 1, CSampleRate);
            w.Add(ch0);
        }

        // make 2x ALow        
        {
            iDSP::LineFreqGenerator gen0(330, 0);
            iDSP::LineFreqGenerator gen1(800, -10);

            std::vector<int> ch0, ch1;
            ch0.reserve(CSamplesCount);
            ch1.reserve(CSamplesCount);

            for(int i = 0; i < CSamplesCount; ++i)
            {
                ch0.push_back( iDSP::linear2alaw(gen0.Next()) );
                ch1.push_back( iDSP::linear2alaw(gen1.Next()) );
            }

            MsWaveWriter w(path + "pcm_8k_alow_2x_wr.wav", MsWaveBase::G711a, 2, CSampleRate);
            w.Add(ch0, ch1);
        } 

        // make 2x ALow
        {
            iDSP::LineFreqGenerator gen0(330, 0);
            iDSP::LineFreqGenerator gen1(800, -10);

            std::vector<int> ch0, ch1;
            ch0.reserve(CSamplesCount);
            ch1.reserve(CSamplesCount);

            for(int i = 0; i < CSamplesCount; ++i)
            {
                ch0.push_back( gen0.Next() );
                ch1.push_back( gen1.Next() );
            }

            MsWaveWriter w(path + "pcm_8k_pcm_2x_wr.wav", MsWaveBase::Pcm, 2, CSampleRate);
            w.Add(ch0, ch1);
        }

    }


}  // namespace


// ------------------------------------------------

namespace Utils
{
    
    void MsWaveTest()
    {
        RunTest1("c:\\temp2\\pcm_8k_16bit_2xgen_stereo.wav");
        RunTest2("c:\\temp2\\");
    }

}  // namespace Utils
