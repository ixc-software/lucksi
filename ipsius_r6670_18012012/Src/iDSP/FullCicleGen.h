#ifndef __FULLCICLEGEN__
#define __FULLCICLEGEN__

#include "Platform/PlatformTypes.h"
#include "Utils/ErrorsSubsystem.h"
#include "Utils/IBasicInterface.h"
#include "Utils/TimerTicks.h"

#include "g711gen.h"
#include "GenUtils.h"

namespace iDSP
{
    using Platform::byte;
    using Platform::int16;

    enum
    {
        CSilenceSample = 0x55,
    };

    /*

        внешние параметры -- toLine, полная длительность 

        int freq0, int freq0Lev,  // 4,5
        int freq1, int freq1Lev,  // 6,7
        int genInterval, int silenceInterval,  // 8,9


        Однотональный генератор использует FreqGenerator, получает от него
        сразу же сэмплы в нужно законе.

        Двухтональный -- использует 2 x LineFreqGenerator, выполняет
        суммирование, а потом кодирует сэмпл. 
        Альтернативный алгоритм -- испльзовать FreqGenerator и двумерную 
        таблицу 256x256 для O(1) операции суммирования и кодирования. 
        Недостатки -- точность, размер таблицы

    */

    // -----------------------------------------------------

    // абстрактный базовый класс для генератора полного цикла
    template<class TSample, int CSilenceVal>
    class FullCicleGenBasicT
    {
        int m_genSamples; 
        int m_silenceSamples;

        bool m_genStage;
        int  m_stageSamples;

        void EnterGenStage(bool withReset = true)
        {
            ESS_ASSERT(m_genSamples > 0);

            m_genStage = true;
            m_stageSamples = m_genSamples;

            if (withReset) GeneratorReset();
        }

        void EnterSilenceStage()
        {
            ESS_ASSERT(m_silenceSamples > 0);

            m_genStage = false;
            m_stageSamples = m_silenceSamples;
        }

        virtual void GeneratorReset() = 0;
        virtual TSample NextGeneratorSample() = 0;
        virtual void FillGenerator(TSample *pBuff, int count) = 0;

        void DecCurrStageSamples(int count)
        {
            m_stageSamples -= count;
            if (m_stageSamples > 0) return;

            if (m_genStage)  // switch to silence
            {
                if (m_silenceSamples > 0) EnterSilenceStage();
                                     else EnterGenStage();
            }
            else
            {
                if (m_genSamples > 0) EnterGenStage();
                                 else EnterSilenceStage();
            }
        }

    public:

        // Если silenceInterval == 0, генерация идет непрерывно
        // Если genInterval == 0, транслируется тишина
        FullCicleGenBasicT(int genInterval, int silenceInterval)           
        {
            ESS_ASSERT( Validate(genInterval, silenceInterval) );

            m_genSamples     = IntervalToSamples(genInterval);
            m_silenceSamples = IntervalToSamples(silenceInterval);

            if (m_genSamples > 0) EnterGenStage(false);
                              else EnterSilenceStage();
        }

        virtual ~FullCicleGenBasicT()
        {
        }

        static bool Validate(int genInterval, int silenceInterval)
        {
            if (genInterval < 0)     return false;
            if (silenceInterval < 0) return false;

            if ((genInterval + silenceInterval) == 0) return false;

            return true;
        }

        TSample NextSample()
        {
            TSample result = m_genStage ? NextGeneratorSample() : CSilenceVal;

            DecCurrStageSamples(1);

            return result;
        }

        // не используется корректировка: count может быть больше чем остаток m_stageSamples
        void Fill(TSample *pBuff, int count)
        {
            if (m_genStage) FillGenerator(pBuff, count);
                       else std::memset(pBuff, CSilenceVal, count);

            DecCurrStageSamples(count);
        }

        static int IntervalToSamples(int intervalMs)
        {
            return (intervalMs * CChFreq) / 1000;
        }

    };

    // -----------------------------------------------------

    typedef FullCicleGenBasicT<byte, CSilenceSample> FullCicleGenBasic;

    // однотональный генератор полного цикла
    class FullCicleGenOneTone : public FullCicleGenBasic
    {
        FreqGenerator m_gen;

        void GeneratorReset()  // override
        {
            m_gen.Reset();
        }

        byte NextGeneratorSample() // override
        {
            return m_gen.Next();
        }

        void FillGenerator(byte *pBuff, int count)  // override
        {
            m_gen.Fill(pBuff, count);
        }

    public:

        FullCicleGenOneTone(int genInterval, int silenceInterval, bool useAlaw, int freq, int freqLev)  
            : FullCicleGenBasic(genInterval, silenceInterval), 
            m_gen(freq, freqLev, useAlaw)
        {
        }

        static bool Validate(int genInterval, int silenceInterval, int freq, int freqLev)
        {
            if (!FreqGenerator::ValidateParams(freq, freqLev)) return false;

            return FullCicleGenBasic::Validate(genInterval, silenceInterval);
        }

    };


    // -----------------------------------------------------

    // двухтональный генератор полного цикла
    class FullCicleGenTwoTone : public FullCicleGenBasic
    {
        LineFreqGenerator m_gen0;
		boost::scoped_ptr<LineFreqGenerator> m_gen1;

        bool m_useAlaw;

        void GeneratorReset()  // override
        {
            m_gen0.Reset();
            if(m_gen1 != 0) m_gen1->Reset();
        }

        byte NextGeneratorSample() // override
        {
            int val = m_gen0.Next();
			if(m_gen1 != 0) val += m_gen1->Next();

            return m_useAlaw ? linear2alaw(val) : linear2ulaw(val);
        }

        void FillGenerator(byte *pBuff, int count)  // override
        {
            for(int i = 0; i < count; ++i)
            {
                pBuff[i] = NextGeneratorSample();
            }
        }

    public:

        FullCicleGenTwoTone(bool useAlaw, const Gen2xProfile &p) : 
            FullCicleGenBasic(p.GenInterval, p.SilenceInterval), 
            m_gen0(p.Freq0, p.Freq0Lev), 
			m_useAlaw(useAlaw)
        {
            if(p.Freq1 > 0) m_gen1.reset(new LineFreqGenerator(p.Freq1, p.Freq1Lev));
        }

        static bool Validate(const Gen2xProfile &p)
        {
            if (!LineFreqGenerator::ValidateParams(p.Freq0, p.Freq0Lev)) return false;
            if (p.Freq1 >0 &&
				!LineFreqGenerator::ValidateParams(p.Freq1, p.Freq1Lev)) return false;

            return FullCicleGenBasic::Validate(p.GenInterval, p.SilenceInterval);
        }

    };

    // -----------------------------------------------------

    typedef FullCicleGenBasicT<int16, 0> FullCicleGenBasicLine;
    
    class FullCicleGenTwoToneLine : public FullCicleGenBasicLine
    {
        LineFreqGenerator m_gen0;
		boost::scoped_ptr<LineFreqGenerator> m_gen1;

        void GeneratorReset()  // override
        {
            m_gen0.Reset();
            if(m_gen1 != 0) m_gen1->Reset();
        }

        int16 NextGeneratorSample() // override
        {
            int val = m_gen0.Next();
			 if(m_gen1 != 0) val += m_gen1->Next();
            return val;
        }

        void FillGenerator(int16 *pBuff, int count)  // override
        {
            for(int i = 0; i < count; ++i)
            {
                pBuff[i] = NextGeneratorSample();
            }
        }

    public:

        FullCicleGenTwoToneLine(const Gen2xProfile &p) : 
            FullCicleGenBasicLine(p.GenInterval, p.SilenceInterval), 
            m_gen0(p.Freq0, p.Freq0Lev)
        {
			if(p.Freq1 > 0) m_gen1.reset(new LineFreqGenerator(p.Freq1, p.Freq1Lev));
        }

		static bool Validate(const Gen2xProfile &p)
		{
			if (!LineFreqGenerator::ValidateParams(p.Freq0, p.Freq0Lev)) return false;
			if (p.Freq1 >0 &&
				!LineFreqGenerator::ValidateParams(p.Freq1, p.Freq1Lev)) return false;

			return FullCicleGenBasicLine::Validate(p.GenInterval, p.SilenceInterval);
		}

    };


}  // namespace iMedia

#endif
