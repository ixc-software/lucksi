
#include "stdafx.h"
#include "SafeBiProto.h"
#include "Platform/Platform.h"
#include "SbpCRC.h"
#include "SbpPackInfo.h"

namespace SBProto
{
    SafeBiProto::SafeBiProto(ISafeBiProtoEvents &owner, 
		ISbpProtoTransport &transport, 
		const SbpSettings &settings, 
		ISpbProtoMonitor *pMonitor) : 
        m_owner(owner), m_transport(transport), m_settings(settings), 
        m_pMonitor(pMonitor),
        m_isActivated(false), m_isWaitingResponce(false), m_isNeedToSendResponce(false),
        m_timers(m_settings),
        m_lastProcessCall(Utils::TimerTicks::Ticks())
    {
        m_transport.BindProto(this);
    }
    
    // ---------------------------------------------------------------

    SafeBiProto::~SafeBiProto()
    {
        m_transport.UnbindProto();
    }
    
    // ---------------------------------------------------------------

    // returns bytes used
    size_t SafeBiProto::ParseHeader(const void *pData, size_t size, bool &completeInd)
    {
        // if packet wasn't created than header wasn't parsed
        ESS_ASSERT(m_recvPack == 0);
        ESS_ASSERT(m_isActivated);
        completeInd = false;
    
        // InitReceiveEndWaitTimer();
        m_timers.StartRecvEndTimer(); // starts once

        Utils::MemReaderStream stream(pData, size);
        Utils::BinaryReader<Utils::MemReaderStream> reader(stream);

        while (stream.HasToRead())
        {
            if (!m_recvPackHeaderParser.Add(reader.ReadByte())) continue;

            completeInd = true;

            break;
        }

        return (size - stream.HasToRead());
    }
    
    // ---------------------------------------------------------------

    bool SafeBiProto::CheckRecvPackTypeSizeProtoVertion()
    {
        // type
        try
        {
            PacketHeader header = m_recvPackHeaderParser.GetHeader();
        }
        catch(PacketHeader::InvalidPacketType &e)
        {
            SendErrorToOwner(new SbpRecvPackInvalidType);
            return false;
        }

        // size
        dword dataLen = m_recvPackHeaderParser.GetHeader().getDataLength();
        if (dataLen == 0)
        {
            SendErrorToOwner(new SbpRecvPackIsEmpty);
            return false;
        }
        
        if (dataLen > PacketStreamCapacity())
        {
            SendErrorToOwner(new SbpRecvPackSizeIsTooBig(dataLen, PacketStreamCapacity()));
            return false;
        }

        // proto version
        byte recvPackProtoVer = m_recvPackHeaderParser.GetHeader().getProtoVersion();
        if (!m_settings.CheckProtoVersion(recvPackProtoVer)) 
        {
            SendErrorToOwner(new SbpInvalidVersion(recvPackProtoVer));
            return false;
        }

        return true;
    }

    // ---------------------------------------------------------------

    void SafeBiProto::DataReceived(const void *pData, size_t size)
    { 
        ESS_ASSERT(size != 0);

        if (m_pMonitor != 0)
        {
            m_pMonitor->OnRawDataRecv(pData, size);
        }
        
        if (!m_isActivated) 
        {
            SendErrorToOwner(new SbpReceiveDataWhileDeactivated);
            return;
        }
        
		CheckIsProcessing();

        size_t bytesUsed = 0;
        while (bytesUsed < size)
        {
            if (m_recvPack == 0) 
            {
                bool headerComplete = false;
                const byte *pLeftData = static_cast<const byte*>(pData) + bytesUsed;
                bytesUsed += ParseHeader(pLeftData, size - bytesUsed, headerComplete);

                if (headerComplete)
                {
                    if (!CheckRecvPackTypeSizeProtoVertion()) return;

                    m_recvPack = boost::shared_ptr<SbpRecvPack>(
                                    new SbpRecvPack(m_recvPackHeaderParser.GetHeader()));
                }
            }
            
            if (m_recvPack == 0 || bytesUsed >= size) return;

            {
                const byte *pLeftData = static_cast<const byte*>(pData) + bytesUsed;
                size_t addedSize = 0;

				boost::shared_ptr<SbpError> error = AddToRecvPackBody(pLeftData, size - bytesUsed, addedSize);
				
				if(error != 0)
				{
					m_owner.ProtocolError(error);
					return;
				}
                bytesUsed += addedSize;
                if (m_recvPack->IsComplete()) SendPacketToOwner();                
            }

        }
    }

	// ---------------------------------------------------------------

	boost::shared_ptr<SbpError> 
		SafeBiProto::AddToRecvPackBody(const void *pData, size_t size, size_t &addedSize)
	{
		try
		{
			m_recvPack->AddToPacketBody(pData, size, addedSize);
		}
		catch (SbpRecvPack::BadCRC &e)
		{
			return boost::shared_ptr<SbpError>(new SbpRecvPackCRCCheckFailed());
		}
		catch (SbpRecvPack::BadDataType &e)
		{
			return boost::shared_ptr<SbpError>(new SbpRecvPackInvalidDataTypes());
		}
		return boost::shared_ptr<SbpError>();
	}

    // ---------------------------------------------------------------

	void SafeBiProto::SendPacketToTransport(const void *pData, size_t size)
	{
		if (m_transport.MaxSendSize() == 0)
		{
			m_transport.DataSend(pData, size);
			return;
		}

		const byte *p = static_cast<const byte *>(pData);
		size_t maxSendSize = m_transport.MaxSendSize();
		size_t writedSize = 0;
		while (size - writedSize > maxSendSize)
		{
			m_transport.DataSend(p + writedSize, maxSendSize);
			writedSize += maxSendSize;
		}
		if(size - writedSize != 0)
		{
			m_transport.DataSend(p + writedSize, size - writedSize);
		}
		
	}

    // ---------------------------------------------------------------

    void SafeBiProto::SpecificOnPacketSendActions(PacketHeader::SbpPacketType packType)
    {
        ESS_ASSERT(m_isActivated);
        switch (packType)
        {
        case PacketHeader::PTCommand:
            ESS_ASSERT(!m_isWaitingResponce);
            // init wait time
            m_timers.StartResponceTimer();
            m_isWaitingResponce = true;
            break;

        case PacketHeader::PTResponce:
            ESS_ASSERT(m_isNeedToSendResponce);
            m_isNeedToSendResponce = false;
            break;

        case PacketHeader::PTInfo:
            // do nothing
            break;

        default:
            ESS_HALT("Invalid packet type");
        }
    }

    // ---------------------------------------------------------------
    
    void SafeBiProto::SendPacketToTransport(const Utils::MemWriterDynStream &header,
                                            const Utils::MemWriterDynStream &body, 
                                            PacketHeader::SbpPacketType packType)
    {
        SpecificOnPacketSendActions(packType);
        
        // check pack body size        
        if (body.BufferSize() > m_settings.getMaxSendSize())
        {
            SendErrorToOwner(new SbpSendPackSizeIsTooBig(body.BufferSize(), m_settings.getMaxSendSize()) );
            return;
        }

        // send header
		SendPacketToTransport(header.GetBuffer(), header.BufferSize());

        // send body
        // if (body.BufferSize() == 0) return;
        ESS_ASSERT(body.BufferSize() != 0);
        SendPacketToTransport(body.GetBuffer(), body.BufferSize());

        if (m_pMonitor == 0) return;

        // send call-back to user
        m_pMonitor->OnSpbPacketOut(header, body);
    }

	// ---------------------------------------------------------------

    void SafeBiProto::SendPacketToOwner()
    {
        ESS_ASSERT(m_recvPack != 0);
        
        m_timers.StopRecvEndTimer();

        // copy to tmp to prevent segmentation fault 
        // if call-backs to owner won't be wrapped in messages
        byte packType = m_recvPackHeaderParser.GetHeader().getPackType();
        boost::shared_ptr<SbpRecvPack> pack = m_recvPack;
        
        m_recvPackHeaderParser.Reset();
        m_recvPack.reset();

        switch (packType)
        {
        case PacketHeader::PTCommand:
            m_isNeedToSendResponce = true;
            m_owner.CommandReceived(pack);
            break;

        case PacketHeader::PTResponce:
            // reset wait time
            m_timers.StopResponceTimer();
            m_isWaitingResponce = false;
            m_owner.ResponseReceived(pack);
            break;

        case PacketHeader::PTInfo:
            m_owner.InfoReceived(pack);
            break;

        default:
            ESS_HALT("Unknown received packet type");
        }

        if (m_pMonitor == 0) return;

        // send call-back to user
        m_pMonitor->OnSpbPacketIn(pack->PackInfo());
    }
    
    // ---------------------------------------------------------------

    // Reset states to Default, set all variables to 0 
    void SafeBiProto::Deactivate()
    {
		m_isActivated = false;
		m_isWaitingResponce = false;
		m_isNeedToSendResponce = false;

		m_timers.ResetAll();

		m_lastProcessCall = Utils::TimerTicks::Ticks();

		m_recvPackHeaderParser.Reset();
		m_recvPack.reset();
    }
        
    // ---------------------------------------------------------------

    void SafeBiProto::Process()
    {
        m_lastProcessCall = Utils::TimerTicks::Ticks();

        if (!IsActive()) return;

        // check timeouts
        if (m_timers.RecvEndTimeout())
        {
            SendErrorToOwner(new SbpWaitingEndOfReceiveTimeout);
			return;
        }

        if (m_isWaitingResponce && m_timers.ResponceTimeout())
        {
            SendErrorToOwner(new SbpWaitingResponceTimeout);
        }
    }

    // ---------------------------------------------------------------

    void SafeBiProto::CheckIsProcessing()
    {
        if (m_settings.getTimeoutCheckIntervalMs() == 0) return;

        dword nextProcessCall = m_lastProcessCall + m_settings.getTimeoutCheckIntervalMs();

        if (nextProcessCall >= Utils::TimerTicks::Ticks()) return;

		SendErrorToOwner(new SbpProcessTimeout);
//        ESS_HALT("Protocol timeouts are not processed or check interval is too short");
    }
    
} // namespace SBProto
