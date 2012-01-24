#include "stdafx.h"
#include "iRtp/RtpPcSession.h"
#include "PlayData.h"

#include "iRtp/RtpConstants.h"

#include "iDSP/g711codec.h"

namespace 
{
	enum {CPacketSize = 160}; 
	const bool CMarker(false);
};

namespace iRtpUtils
{    
	using namespace Platform;

	PlayData::PlayData(iCore::MsgThread& thread, 
		Utils::SafeRef<iRtp::RtpPcSession> rtp,
		IDataSrcForRtpTest *dataSource) :
		iCore::MsgObject(thread),
		m_rtp(rtp),
		m_dataSource(dataSource),
		m_txTimer(this, &T::Send),
		m_isStarted(false),
		m_prevTickInited(false),
		m_prevTick(0),
		m_rate(0),
        m_timestamp(0)
	{
        m_mustSendBytes = 0;
    }

    PlayData::~PlayData()
    {
        //std::cout << m_statSendSize.getAsString();
    }

	// -----------------------------------------------------------------------
	// IRtpTest

	void PlayData::StartSend(const iMedia::Codec &codec)
	{
		m_rate = codec.getRate();
		int period = 1000 * CPacketSize / getRate();
        m_dataSource->Setup(codec);
		m_txTimer.Start(period, true);
	}

	// -----------------------------------------------------------------------

	void PlayData::StartReceive(const iMedia::Codec &codec)
	{
		// ignore
	}

	// -----------------------------------------------------------------------
	
	void PlayData::ReceiveData(iRtp::SharedBdirBuff buff, iRtp::RtpHeaderForUser header)
	{
		// ignore
	}

	// -----------------------------------------------------------------------

	void PlayData::Send(iCore::MsgTimer*)
	{
		ddword currTime = GetSystemMicrosecTickCount();

		if (!m_prevTickInited)
		{   
			// init tick counter
			m_prevTick = currTime;
			m_prevTickInited = true;
			return;
		}

		if (currTime == m_prevTick) return;
			
		ddword diffTime = currTime - m_prevTick;
		m_prevTick = currTime;                
		
        int size = (getRate()/1000) * diffTime / 1000;  // in bytes
        size += m_mustSendBytes;

        while(size >= CPacketSize)
        {
            SendPack(CPacketSize);
            size -= CPacketSize;
        }

        m_mustSendBytes = size;        
	}

    // -----------------------------------------------------------------------

    void PlayData::SendPack(int size)
    {
        QByteArray packData = m_dataSource->Read(size);        

        ESS_ASSERT(packData.size() == size);

        m_timestamp += size;
        m_rtp->TxData( CMarker, m_timestamp, packData );            
    }


	// -----------------------------------------------------------------------

	int PlayData::getRate() const
	{
		return m_rate;
	}
};
