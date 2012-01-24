#include "stdafx.h"
#include "IRtpTest.h"
#include "PlayFile.h"
#include "iDSP/g711codec.h"

namespace 
{
    Platform::byte Compress(int val, const iMedia::Codec &codec)
    {
        if (codec.getName() != "PCMU" && codec.getName() != "PCMA") ESS_UNIMPLEMENTED;
        return (codec.getName() == "PCMA") ? iDSP::linear2alaw(val) : iDSP::linear2ulaw(val);
    }
} // namespace 

namespace iRtpUtils
{    
	PlayRawFile::PlayRawFile(const QString &name) :
		m_file(name)
	{
		if (!m_file.open(QIODevice::ReadOnly) )     
		{
			std::string msg("Can`t open file");
            ESS_THROW_MSG(IRtpTest::RtpTestError, msg + m_file.fileName().toStdString());
		}
		if (m_file.size() == 0)
			ESS_THROW_MSG(IRtpTest::RtpTestError, m_file.fileName().toStdString() + " is empty." );
	}

	//-----------------------------------------------------------------------    
    
    const std::string PlayRawFile::Name("Play raw file");

	//-----------------------------------------------------------------------
	// IDataSrcForRtpTest

    void PlayRawFile::Setup(const iMedia::Codec &codec)
    {
        // ignore
    }

	//-----------------------------------------------------------------------

	QByteArray PlayRawFile::Read(int size)
	{
        QByteArray packData = m_file.read(size);
        
        if (!packData.size())  
        {
            m_file.seek(0); // reset position
            packData = m_file.read(size);
        }
        if (packData.size() == size) return packData;

        QByteArray result(packData);        
        while(result.size() + packData.size() <= size)
        {
            result += packData;
        }
        if (result.size() == size) return result;
        
        int residue = size - result.size();
        for (int i = 0; i < residue; ++i)
        {
            result += packData[i];
        }

        return result;
	}

    // ------------------------------------------------------------------------------------

    const std::string PlayWavFile::Name("Play wave file");

    // ------------------------------------------------------------------------------------


    PlayWavFile::PlayWavFile( const QString &name )
        : m_wave(name), m_pos(m_wave, 0), m_codec(iMedia::Codec::ALaw_8000)
    {
    }

    // ------------------------------------------------------------------------------------

    void PlayWavFile::Setup( const iMedia::Codec &codec )
    {
        m_codec = codec;
    }

    // ------------------------------------------------------------------------------------

    QByteArray PlayWavFile::Read( int size )
    {
        QByteArray result;       

        for (int i = 0; i < size; ++i)
        {
            if (m_pos.Eof()) m_pos.First();
            result.push_back( Compress(m_pos.Get(), m_codec) );
            m_pos.Next();
        }

        return result;
    }
};
