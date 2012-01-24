#include "stdafx.h"

#include "Utils/IntToString.h"

#include "SafeBiProto/LogSbpProtoMonitor.h"

#include "SbpConnection.h"

namespace 
{ 
    int GNumberSbpConnection = 0;
};

namespace SBProtoExt
{    
	using namespace SBProto;

    SbpConnection::SbpConnection(iCore::MsgThread &thread, SbpConnProfile &profile,
		boost::shared_ptr<SbpLogSettings> logSettings) :
        iCore::MsgObject(thread),
        m_checkProfile(profile),
		m_isActivatedByUser(false),
		m_isSbpActivated(false),
        m_user(profile.m_user),
        m_rawDataCallback(profile.m_rawDataCallback),
		m_transport(profile.GetTransport()),
		m_sbpProto(new SafeBiProto(*this, *m_transport, profile.m_sbpSettings, this)),
		m_logSettings(logSettings)
    {
        {
			std::string name("SbpConnection");
			name += Utils::IntToString(GNumberSbpConnection++);
            m_log.reset(profile.m_logCreator->CreateSession(name, true));
        }
        m_tagInfo       = m_log->RegisterRecordKindStr("Info", true);
        m_tagError      = m_log->RegisterRecordKindStr("Error", true);
        m_tagRecvPack   = m_log->RegisterRecordKindStr("SBP.Recv", true);
        m_tagSendPack   = m_log->RegisterRecordKindStr("SBP.Send", true);

        if(m_log->LogActive(m_tagInfo))
		{
            *m_log << m_tagInfo << "Created.";
            std::string info(m_transport->Info());
            
            if(info.size()) *m_log << " Transport info: " << info;
            
            *m_log << iLogW::EndRecord;
        }
        
        m_transport->BindUser(this);
			
		if(profile.m_sbpActivationReq) 
		{
			m_isActivatedByUser = true;	
			if(!m_isSbpActivated) ActivateSbp();
		}
    }

    SbpConnection::~SbpConnection()
    {
        if(m_log->LogActive(m_tagInfo))
        {
            *m_log << m_tagInfo << "Deleted." << iLogW::EndRecord;
        }
    }

    // --------------------------------------------------------
    // SBProto::ISbpTransportToUser

    void SbpConnection::TransportConnected()
    {
		PutMsg(this, &T::onTransportConnected);
    }

    void SbpConnection::TransportDisconnected(const std::string &desc)
    {
        PutMsg(this, &T::onTransportDisconnected, desc);
    }

    void SbpConnection::TransportError(const std::string &err)
    {
		PutMsg(this, &T::onTransportDisconnected, err);
    }

    // --------------------------------------------------------
	// SBProto::ISafeBiProtoForSendPack

	Platform::byte SbpConnection::ProtoVersion() const
	{
		SBProto::ISafeBiProtoForSendPack &p = *m_sbpProto;
		return p.ProtoVersion();
	}

	size_t SbpConnection::PacketStreamCapacity() const
	{
		SBProto::ISafeBiProtoForSendPack &p = *m_sbpProto;
		return p.PacketStreamCapacity();
	}

	void SbpConnection::SendPacketToTransport(const Utils::MemWriterDynStream &header,
		const Utils::MemWriterDynStream &body, 
		SBProto::PacketHeader::SbpPacketType packType)
	{
		if(!m_isSbpActivated) return;
		SBProto::ISafeBiProtoForSendPack &p = *m_sbpProto;

		p.SendPacketToTransport(header,
			body, 
			packType);
	}

    // --------------------------------------------------------
    // SBProto::ISafeBiProtoEvents

    void SbpConnection::CommandReceived(boost::shared_ptr<SbpRecvPack> data)
    {
        PutMsg(this, &T::onCommandReceived, data);
    }

    void SbpConnection::ResponseReceived(boost::shared_ptr<SbpRecvPack> data)
    {
        PutMsg(this, &T::onResponseReceived, data);
    }

    void SbpConnection::InfoReceived(boost::shared_ptr<SbpRecvPack> data)
    {
        PutMsg(this, &T::onInfoReceived, data);
    }

    void SbpConnection::ProtocolError(boost::shared_ptr<SbpError> err)
    {
        PutMsg(this, &T::onProtocolError, err);
    }

    // --------------------------------------------------------
    // msg wrapper SBProto::ISafeBiProtoEvents

    void SbpConnection::onCommandReceived(boost::shared_ptr<SbpRecvPack> data)
    {
        if(!m_user.IsEmpty()) m_user->CommandReceived(this, data);
    }

    void SbpConnection::onResponseReceived(boost::shared_ptr<SbpRecvPack> data)
    {
        if(!m_user.IsEmpty()) m_user->ResponseReceived(this, data);
    }

    void SbpConnection::onInfoReceived(boost::shared_ptr<SbpRecvPack> data)
    {
        if(!m_user.IsEmpty()) m_user->InfoReceived(this, data);
    }

    void SbpConnection::onProtocolError(boost::shared_ptr<SbpError> err)
    {
		if(m_log->LogActive(m_tagError))
		{
			*m_log << m_tagError << "Sbp error.";
			if(err != 0) *m_log << " " << err->ToString();
			*m_log << iLogW::EndRecord;
		}

		m_transport->Disconnect();
		if(m_isSbpActivated) 
		{
			DeactivateSbp();
			if(!m_user.IsEmpty()) m_user->ProtocolError(this, err);
		}
		m_transport->Connect();
    }


	// --------------------------------------------------------
	// msg wrapper SBProto::ISbpTransportToUser

	void SbpConnection::onTransportConnected()
	{
		if(m_log->LogActive(m_tagInfo))
		{
			*m_log << m_tagInfo << "Transport connected. " 
				<< m_transport->Info() << iLogW::EndRecord;
		}
		if(m_isSbpActivated) return;
		
		ActivateSbp();
		if (!m_user.IsEmpty()) m_user->ConnectionActivated(this);
	}

	// --------------------------------------------------------

	void SbpConnection::onTransportDisconnected(const std::string &err)
	{
		// может приходить несколько раз
		if(m_isSbpActivated && m_log->LogActive(m_tagInfo))
		{
			*m_log << m_tagInfo << err << iLogW::EndRecord;
		}

		m_transport->Disconnect();
		if(m_isSbpActivated) 
		{
			DeactivateSbp();
			if (!m_user.IsEmpty()) m_user->ConnectionDeactivated(this, err);
		}
		if(m_isActivatedByUser) m_transport->Connect();
	}

    // --------------------------------------------------------

	void SbpConnection::ActivateSbp()
	{
		ESS_ASSERT(!m_isSbpActivated);

		m_sbpProto->Activate();
		
		m_isSbpActivated = true;
	}

    // --------------------------------------------------------

	void SbpConnection::DeactivateSbp()
	{
		ESS_ASSERT(m_isSbpActivated);

		m_sbpProto->Deactivate();

		m_isSbpActivated = false;
	}

    // --------------------------------------------------------
    // ISbpConnection

	void SbpConnection::Process()
	{
		m_sbpProto->Process();
		m_transport->Process();
	}

	// --------------------------------------------------------

    void SbpConnection::BindUser(Utils::SafeRef<ISbpConnectionEvents> user)
    {
        ESS_ASSERT(m_user.IsEmpty());

        m_user = user;

        if(m_log->LogActive(m_tagInfo))
        {
            *m_log << m_tagInfo << "Bind user" << iLogW::EndRecord;
        }
    }

    // --------------------------------------------------------

    void SbpConnection::ActivateConnection()
    {
		ESS_ASSERT(!m_isActivatedByUser);


        if(m_log->LogActive(m_tagInfo))
        {
			*m_log << m_tagInfo << "Activate transport." << iLogW::EndRecord;
        }

		m_isActivatedByUser = true;

        m_transport->Connect();
    }

    // --------------------------------------------------------

    void SbpConnection::DeactivateConnection()
    {
		ESS_ASSERT(m_isActivatedByUser);

        if(m_log->LogActive(m_tagInfo))
        {
            *m_log << m_tagInfo << "Deactivate transport." << iLogW::EndRecord;
        }
		m_isActivatedByUser = false;
        m_transport->Disconnect();
		DeactivateSbp();
    }

    // --------------------------------------------------------

    void SbpConnection::UnbindUser()
    {
        if(m_log->LogActive(m_tagInfo))
        {
            *m_log << m_tagInfo << "UnbindUser." << iLogW::EndRecord;
        }
        m_user.Clear();
    }

    ISafeBiProtoForSendPack &SbpConnection::Proto()
    {
        return *this;
    }

    std::string SbpConnection::TransportInfo() const
    {
        ESS_ASSERT(m_transport != 0);

        return m_transport->Info();
    }

	std::string SbpConnection::Name() const 
	{
		return m_log->NameStr();
	}

    void SbpConnection::OnRawDataRecv( const void *pData, size_t size ) 
    {
        if (m_rawDataCallback.IsEmpty()) return;

        m_rawDataCallback->OnSbpRawDataRecv(pData, size);
    }

	void SbpConnection::OnSpbPacketOut(const Utils::MemWriterDynStream &header, const Utils::MemWriterDynStream &body)
    {
		if (!m_log->LogActive(m_tagSendPack)) return;

		SBProto::SbpPackInfo data(header, body);

		*m_log << m_tagSendPack;

		LogSbpProtoMonitor(*m_log, data, m_logSettings->CountPar(), m_logSettings->BinaryMaxBytes());

		*m_log << iLogW::EndRecord;

    }

    void SbpConnection::OnSpbPacketIn( const SbpPackInfo &data ) 
    {
		if (!m_log->LogActive(m_tagRecvPack)) return;

		*m_log << m_tagRecvPack;

		LogSbpProtoMonitor(*m_log, data, m_logSettings->CountPar(), m_logSettings->BinaryMaxBytes());

		*m_log << iLogW::EndRecord;

    }

} // namespace SBProto


