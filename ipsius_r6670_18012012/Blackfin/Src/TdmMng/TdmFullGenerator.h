#ifndef __TDMFULLGENERATOR__
#define __TDMFULLGENERATOR__

#include "Platform/Platform.h"

#include "ITdmGenerator.h"
#include "iDSP/FullCicleGen.h"

namespace TdmMng
{
    using Platform::byte;
	
	class TdmFullGenerator : public ITdmGenerator
	{
        boost::scoped_ptr<iDSP::FullCicleGenBasic> m_gen;
        std::vector<byte> m_buff;  // buff for write to ITdmWriteWrapper

        int m_samplesGenerated;
        int m_samplesLimit;
        bool m_completed;

        void Generate(byte *p, int count)
        {
            // fill
            m_gen->Fill(p, count);

            // update m_samplesGenerated
            m_samplesGenerated += count;

            if (m_samplesLimit > 0)
            {
                if (m_samplesGenerated >= m_samplesLimit) m_completed = true;
            }
        }

    // ITdmGenerator impl       
    public:     

        // from TDM
        void WriteTo(Utils::BidirBuffer &buff, int size)
        {
            buff.SetDataOffset(buff.GetDataOffset(), size);

            Generate(buff.Front(), size);
        }

        // to TDM
        void WriteTo(ITdmWriteWrapper &buff, int chNum)
        {
            int count = buff.BlockSize();
            if (m_buff.size() != count) m_buff.resize(count);

            Generate(&m_buff[0], count);

            // m_buff -> buff
            buff.Write(chNum, 0, &m_buff[0], count);
        }

        bool Completed()
        {
            return m_completed;
        }

	public:

        TdmFullGenerator(bool useAlaw, const iDSP::Gen2xProfile &profile, int duration)
        {
            ESS_ASSERT( Validate(profile) );

            // limit
            m_completed = false;
            m_samplesGenerated = 0;
            m_samplesLimit = 0;
            if (duration > 0) 
            {
                m_samplesLimit = iDSP::FullCicleGenBasic::IntervalToSamples(duration);
            }

            // create gen
            iDSP::FullCicleGenBasic *p = 0;
            if (profile.Freq1 <= 0)  // single tone
            {
                p = new iDSP::FullCicleGenOneTone(profile.GenInterval, profile.SilenceInterval, 
                                                  useAlaw, profile.Freq0, profile.Freq0Lev);
            }
            else
            {
                p = new iDSP::FullCicleGenTwoTone(useAlaw, profile);
            }

            m_gen.reset(p);
        }

        static bool Validate(const iDSP::Gen2xProfile &prof)
        {
            typedef iDSP::FullCicleGenOneTone X;

            if (!X::Validate(prof.GenInterval, prof.SilenceInterval, prof.Freq0, prof.Freq0Lev)) 
            {
                return false;
            }

            if (prof.Freq1 > 0)
            {
                if (!X::Validate(prof.GenInterval, prof.SilenceInterval, prof.Freq1, prof.Freq1Lev)) 
                {
                    return false;
                }
            }

            return true;
        }
	};
	
}  // namespace TdmMng

#endif
