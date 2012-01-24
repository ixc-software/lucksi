#include "stdafx.h"
#include "WriteToFile.h"

#include "Utils/MsWave.h"
#include "iDSP/g711codec.h"

namespace 
{
    void ConvertToLin(iRtp::SharedBdirBuff inBuff, const iMedia::Codec &codec, std::vector<int>& result)
    {
        if (codec.getName() != "PCMU" && codec.getName() != "PCMA") ESS_UNIMPLEMENTED;
        //result.clear();
        int lin;
        for (int i = 0; i < inBuff->Size(); ++i)
        {            
            lin = (codec.getName() == "PCMU") ? iDSP::ulaw2linear(inBuff->At(i)) : iDSP::alaw2linear(inBuff->At(i));
            result.push_back(lin);
        }
    }
} // namespace 

namespace iRtpUtils
{    
	WriteToFile::WriteToFile(const QString &name, bool convertToWave) :
		m_file(name),
		m_isStarted(false),
        m_receiveCodec(iMedia::Codec::ALaw_8000),
        m_convertToWave(convertToWave)
	{
		if (!m_file.open(QIODevice::WriteOnly))
		{
			std::string msg("Can`t open file ");
			ESS_THROW_MSG(RtpTestError, msg + m_file.fileName().toStdString() );
		}
	}

    const std::string WriteToFile::Name("Write to file");

	// ------------------------------------------------
    // IRtpTest

	void WriteToFile::StartSend(const iMedia::Codec &sendCodec)
	{
		/*ignore*/
	}

	// ------------------------------------------------

	void WriteToFile::StartReceive(const iMedia::Codec &receiveCodec)
	{
		m_isStarted = true;
        m_receiveCodec = receiveCodec;
	}

	// ------------------------------------------------

	void WriteToFile::ReceiveData(iRtp::SharedBdirBuff buff, iRtp::RtpHeaderForUser header)
	{
        if (!m_isStarted) return;

        // raw mode
		if (!m_convertToWave)
        {
            void* pRawData = buff->Front();
			m_file.write( static_cast<const char*>(pRawData), buff->Size() );
            return;
        }

        // wave mode                                        
        ConvertToLin(buff, m_receiveCodec, m_linCash);
        
	}

    WriteToFile::~WriteToFile()
    {
        if (!m_convertToWave) return;

        QByteArray wavOut = Utils::MsWaveWriter::MakeWavStreamOneCh(Utils::MsWaveBase::Pcm, m_receiveCodec.getRate(), m_linCash);
        m_file.write(wavOut);        
    }
   

    
};

