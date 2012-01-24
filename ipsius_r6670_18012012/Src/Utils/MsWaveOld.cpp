#include "stdafx.h"

#include "iDSP/g711gen.h"

#include "MsWaveOld.h"

// ------------------------------------------------

namespace
{
    using Utils::MsWaveOld;
    using Utils::WaveChIteratorOld;

    void RunTest(QString fileName)
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

        QByteArray wavOut = MsWaveOld::MakeWavStreamTwoCh(MsWaveOld::Pcm, CSampleRate, 2, ch0, ch1);

        // save to disk
        if (fileName.length() > 0)
        {
            QFile f(fileName);
            ESS_ASSERT( f.open(QIODevice::WriteOnly | QIODevice::Truncate) );
            f.write(wavOut);
        }

        // read and verify
        {
            MsWaveOld wave(wavOut);

            WaveChIteratorOld i0(wave, 0);
            WaveChIteratorOld i1(wave, 1);

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

}  // namespace


// ------------------------------------------------

namespace Utils
{
    
    void MsWaveOldTest()
    {
        RunTest("c:\\temp2\\pcm_8k_16bit_2xgen_stereo.wav");
    }

}  // namespace Utils
