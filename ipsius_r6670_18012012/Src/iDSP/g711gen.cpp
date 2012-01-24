#include "stdafx.h"

#include "Utils/ErrorsSubsystem.h"
#include "Platform/Platform.h"

#include "g711gen.h"
#include "g711codec.h"
#include "get_power.h"

using namespace iDSP;
using Platform::byte;

namespace
{
    const int CDivider      = 16128;  // max value

    const float CPi         = (float)3.141592;
    const float CSqrt2      = (float)1.41421356;  // sqrt(2)

    // ----------------------------------------------------------

    void Generate(int freq, int power, std::vector<int> &res, int count)
    {
        res.clear();
        res.reserve(count);

        double Amax = PowerToAmpMax(power);       
        double Kfreq = freq * RadianSplit(CChFreq);

        for(int i = 0; i < count; ++i)
        {
            double val = Amax * std::sin(i * Kfreq);
            res.push_back( (int)val );
        }
    }

    // ----------------------------------------------------------

    void ConvertToCodec(const std::vector<int> &src, std::vector<byte> &res, bool toAlow)
    {
        res.clear();
        res.reserve( src.size() );

        for(int i = 0; i < src.size(); ++i)
        {
            int val = src.at(i);

            byte sample = toAlow ? linear2alaw(val) : linear2ulaw(val);

            res.push_back(sample);
        }
    }

    // ----------------------------------------------------------

    bool RunTest(int freq, int power, bool UseAlaw)
    {
        const int CCount = 512;

        std::vector<int> gen;
        Generate(freq, power, gen, CCount);

        std::vector<byte> compressed;
        ConvertToCodec(gen, compressed, UseAlaw);

        {
            int freqRes;
            float powerRes;
            bool res = GP_ProcessBuff(&compressed.at(0), compressed.size(), UseAlaw, &freqRes, &powerRes); 

            if (!res) return false;

            if (std::abs(freq - freqRes) > 5) return false;
            if (std::abs(powerRes - power) > 0.1) return false;
        }

        return true;
    }

    // ----------------------------------------------------------

    bool RunGenTest(int freq, int power, bool UseAlaw)
    {
        FreqGenerator gen(freq, power, UseAlaw);

        const int CBuffSize = 512;
        byte buff[CBuffSize];

        gen.Fill(buff, CBuffSize);

        {
            int freqRes;
            float powerRes;
            bool res = GP_ProcessBuff(buff, CBuffSize, UseAlaw, &freqRes, &powerRes); 

            if (!res) return false;

            if (std::abs(freq - freqRes) > 10) return false;
            if (std::abs(powerRes - power) > 0.1) return false;
        }

        return true;
    }

}


// -----------------------------------------------

namespace iDSP
{
	
	void GeneratorsTest()
	{
        TUT_ASSERT( RunTest(1000, 0, true) );
        TUT_ASSERT( RunTest(300, -15, false) );

        TUT_ASSERT( RunGenTest(1000, 0, true) );
        TUT_ASSERT( RunGenTest(300, -15, false) );        
        TUT_ASSERT( RunGenTest(430, 1, false) );  
        TUT_ASSERT( RunGenTest(1999, -7, false) );  
	}
	
    // -----------------------------------------------

    float PowerToAmpMax(int power)
    {
        float raise = power * 0.05;  // power/20
        float Amax = std::pow(10.0f, raise) * CDivider * CSqrt2;

        return Amax;
    }

    float RadianSplit(int freq)
    {
        return (2 * CPi) / freq;
    }



}  // namespace iDSP
