#ifndef __G711GEN__
#define __G711GEN__

#include "Platform/PlatformTypes.h"
#include "Utils/ErrorsSubsystem.h"
#include "Utils/IntFrac.h"
#include "Utils/ComparableT.h"
#include "Utils/IBasicInterface.h"
#include "Utils/ManagedList.h"
#include "Utils/RawCRC32.h"

#include "g711codec.h"

namespace iDSP
{
    using Platform::byte;

    enum
    {
        CChFreq       = 8000,    // samples / sec in channel
    };

    float PowerToAmpMax(int power);
    float RadianSplit(int freq);  // (2 * pi) / freq

    void GeneratorsTest();

    // -----------------------------------------------------------

    class ISampleConvert : public Utils::IBasicInterface
    {
    public:
        virtual int SampleConvert(int val) const = 0;
    };

    // -----------------------------------------------------------

    // базовый класс генератора частоты с заданными параметрами
    // m_sinBuff одной разрядности и закона можно было бы шарить между классами 
    template<class TSampleType>
    class BasicFreqGenerator
    {
        typedef Utils::IntFrac<16> IntFrac;

        const int m_sinTableSize;

        std::vector<TSampleType> m_sinBuff;

        IntFrac m_step;
        IntFrac m_currPos;

        std::string TableToString(int pos, int count) const
        {
            std::ostringstream ss;

            for(int i = pos; i < pos + count; ++i)
                ss << m_sinBuff.at(i) << " ";

            return ss.str();
        }

    protected:

        BasicFreqGenerator(int freq, int power, const ISampleConvert &conv, int sinTableSize)
            : m_sinTableSize(sinTableSize)
        {
            ESS_ASSERT( ValidateParams(freq, power) );

            // fill sin table: 2*pi -> CSinTableSize
            {
                m_sinBuff.reserve(m_sinTableSize);

                float Amax = PowerToAmpMax(power);
                float Kfreq = RadianSplit(m_sinTableSize); 

                for(int i = 0; i < m_sinTableSize; ++i)
                {
                    int val = (int)(Amax * std::sin(i * Kfreq));
                    TSampleType sample =  conv.SampleConvert(val);

                    // m_sinBuff[i] = sample;
                    m_sinBuff.push_back(sample);
                }
            }

            // step count
            double step = ((double)m_sinTableSize) / ((double)CChFreq / freq);
            ESS_ASSERT((step > 1) && (step < m_sinTableSize));
            m_step = IntFrac(step);

            m_currPos = 0;
        }

    public:

        virtual ~BasicFreqGenerator() {}

        void Fill(TSampleType *pBuff, int count)
        {
            ESS_ASSERT(pBuff != 0);

            for(int i = 0; i < count; ++i)
            {
                pBuff[i] = Next();
            }
        }

        TSampleType Next()
        {
            int indx = m_currPos.IntPartRounded();
            if (indx == m_sinTableSize) indx = 0;

            // inc
            m_currPos += m_step;
            if (m_currPos.IntPart() >= m_sinTableSize) 
            {
                IntFrac maxVal(m_sinTableSize);
                m_currPos -= maxVal;
            }

            // ret
            return m_sinBuff.at(indx);
        }

        void Reset()
        {
            m_currPos = 0;
        }

        std::string ToString(int halfTableSlice = -1) const
        {
            using namespace std;
            std::ostringstream ss;

            ss << "Table size " << m_sinTableSize << ", step " << m_step.ToString();

            if (halfTableSlice != 0)
            {
                Platform::dword crc = Utils::UpdateCRC32(m_sinBuff);
                ss << "; CRC " << hex << crc << dec << "; ";

                int count = (halfTableSlice < 0) ? m_sinBuff.size() : halfTableSlice;
                ss << TableToString(0, count);
                
                if (halfTableSlice > 0)
                {
                    ss << " (...) " << TableToString(m_sinBuff.size() - halfTableSlice, halfTableSlice);
                }
            }

            std::string s = ss.str();
            return s;
        }

        static bool ValidateParams(int freq, int power)
        {
            bool freqOK = (freq >= 300) && (freq <= 2000);
            bool powOK  = (power >= -40) && (power <= 10);

            return freqOK && powOK;
        }
    };

    // -----------------------------------------------------------

    // генератор в a/u закон
    class FreqGenerator : public BasicFreqGenerator<Platform::byte>
    {

        class Conv : public ISampleConvert
        {
            bool m_UseAlaw;

        public:
            Conv(bool UseAlaw) : m_UseAlaw(UseAlaw) {}

            int SampleConvert(int val) const
            {
                return m_UseAlaw ? linear2alaw(val) : linear2ulaw(val);
            }

        };

    public:

        FreqGenerator(int freq, int power, bool UseAlaw, int sinTableSize = 512)
            : BasicFreqGenerator<Platform::byte>(freq, power, Conv(UseAlaw), sinTableSize)
        {
            // ...
        }

    };

    // -----------------------------------------------------------

    // генератор в линейном коде
    class LineFreqGenerator : public BasicFreqGenerator<int>
    {

        class Conv : public ISampleConvert
        {
        public:

            int SampleConvert(int val) const
            {
                return val;
            }

        };

    public:

        LineFreqGenerator(int freq, int power, int sinTableSize = 512)
            : BasicFreqGenerator<int>(freq, power, Conv(), sinTableSize)
        {
            // ...
        }

    };

    // -----------------------------------------------------------

    // многочастотный генератор в линейном коде
    class LineMultiFreqGenerator
    {
        Utils::ManagedList<LineFreqGenerator> m_gens;

    public:

        LineMultiFreqGenerator()
        {
            // ...
        }

        void Add(int freq, int power, int sinTableSize = 512)
        {
            m_gens.Add( new LineFreqGenerator(freq, power, sinTableSize) );
        }

        int Next()
        {
            int result = 0;

            for(size_t i = 0; i < m_gens.Size(); ++i)
            {
                result += m_gens[i]->Next();
            }

            // limit check!
            const int CMax = 0x7E00;

            if (result > CMax) result = CMax;
            if (result < -CMax) result = -CMax;

            return result;
        }

        template<class TCont>
        void Fill(TCont &c, int count)
        {
            while(count--)
            {
                c.push_back( Next() );
            }
        }

    };
		
}  // namespace iMedia


#endif
