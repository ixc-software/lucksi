#include "stdafx.h"
#include "TestGoertzelDetect.h"
#include "iDSP/g711gen.h"
#include "GoertzelDetector.h"

namespace 
{
    int EnergyMeter(int filterFreq, iDSP::LineMultiFreqGenerator& dataSource, int dataSize)
    {
        iDSP::GoertzelDetector detector(filterFreq);
        for (int i = 0; i < dataSize; ++i)
        {
            detector.Process( dataSource.Next() );
        }
        return detector.Release();
    }

    void SimpleTest(int freqCount, int f0Index)
    {
        ESS_ASSERT(f0Index < freqCount);        

        const int CMinFreq = 600;
        const int CMaxFreq = 2000;
        const int CFreqStep = (CMaxFreq - CMinFreq) / freqCount;
        const int CGenLevel = 0; // db
        const int CDataSize = 100;

        int maxEnergy = -1;
        int maxEnergyIndex = -1;
        for (int i = 0; i < freqCount; ++i)
        {
            int fCurr = CMinFreq + CFreqStep * i;      
            int f0 = CMinFreq + CFreqStep * f0Index;      

            iDSP::LineMultiFreqGenerator gen;
            gen.Add(fCurr, -2);
            int energy = EnergyMeter(f0, gen, CDataSize);                        

            // find index of max energy
            if (energy <= maxEnergy) continue;
            maxEnergy = energy;
            maxEnergyIndex = i;
        }

        TUT_ASSERT(maxEnergyIndex == f0Index);
    }

        
    void TestEvaluateLevel(int gen_dBm0, int sampleCount, const iDSP::ICodec* codec)
    {
        const int CFreq = 600;        

        iDSP::LineFreqGenerator gen(CFreq, gen_dBm0);
        iDSP::GoertzelDetector detector(CFreq, sampleCount);

        for (int i = 0; i < sampleCount; ++i)
        {            
            int sample = (codec != 0) ? codec->Decode(codec->Encode(gen.Next())) : gen.Next();
            detector.Process(sample);
        }

        int dif = gen_dBm0 - detector.Release_dBm0();        
        TUT_ASSERT( abs(dif) <= iDSP::GoertzelDetector::CdBm0_Precision );
    }

    void TestEvaluateLevel(const iDSP::ICodec* codec = 0)
    {
        for (int level = -40; level <= 0; ++level)
        {            
            TestEvaluateLevel(level, 50, codec);            
            TestEvaluateLevel(level, 200, codec);
        }
    }
} // namespace 

namespace iDSP
{    
    void TestGoertzelDetect(bool silence)
    {           
        if (!silence)
        {
            std::cout << "Start TestGoertzelDetect" << std::flush;
        }
        
        TestEvaluateLevel(0);
        TestEvaluateLevel(&iDSP::CodecAlow());
        TestEvaluateLevel(&iDSP::CodecUlow());        

        const int CFreqCount = 10;
        for (int i = 0; i < CFreqCount; ++i)
        {            
            SimpleTest(CFreqCount, i);

            if (!silence) std::cout << "." << std::flush;
        }

        if (!silence)
        {
            std::cout << "\nTestGoertzelDetect complete." << std::endl;
        }

    }
} // namespace iDSP
