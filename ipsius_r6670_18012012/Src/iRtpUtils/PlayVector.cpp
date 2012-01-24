#include "stdafx.h"
#include "IRtpTest.h"
#include "iDSP/g711gen.h"
#include "iDSP/g711codec.h"
#include "iDSP/get_power.h"
#include "PlayVector.h"

namespace 
{    
    const int CBinaryMaxBytes = 50;

	using namespace iDSP;
	using Platform::byte;
	const int CCount = 512;

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
};

namespace iRtpUtils
{    
    PlayVector::PlayVector(boost::shared_ptr<Data> data, bool useAlaw) :
		m_data(data),
        m_useAlaw(useAlaw),
        m_pos(0)
	{
		if (m_data->size() == 0) ESS_THROW_MSG(IRtpTest::RtpTestError, "Vector is empty." );
	}

	//-----------------------------------------------------------------------    
    
    const std::string PlayVector::Name("Play vector");

	//-----------------------------------------------------------------------
	// IDataSrcForRtpTest

    void PlayVector::Setup(const iMedia::Codec &codec)
    {
        ESS_ASSERT((m_useAlaw && codec == iMedia::Codec::ALaw_8000) ||
            codec == iMedia::Codec::ULaw_8000);
    }

	//-----------------------------------------------------------------------

	QByteArray PlayVector::Read(int size)
	{
        QByteArray packData;
        while(packData.size() !=size)
        {
            if(m_pos == m_data->size()) m_pos = 0;

            int writeSize = m_data->size() - m_pos;
            int restSize = size - packData.size();
            if(writeSize > restSize) writeSize = restSize;
            packData.append(
                QByteArray(reinterpret_cast<const char*>(&(*m_data)[m_pos]), writeSize));
            m_pos += writeSize;
        }
        return packData;
	}

	//-----------------------------------------------------------------------

	PlayVectorFactory::PlayVectorFactory(int freq, int power, bool aLaw) :
		m_data(new PlayVector::Data())
	{
		m_useAlaw = aLaw;
		std::vector<int> gen;
		Generate(freq, power, gen, CCount);
		ConvertToCodec(gen, *m_data, m_useAlaw);
	}
};
