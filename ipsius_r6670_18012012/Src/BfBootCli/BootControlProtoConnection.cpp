#include "stdafx.h"
#include "SafeBiProtoExt/SbpConnection.h"
#include "BootControlProtoConnection.h"
#include "BfBootCore/NamedScript.h"
#include "BfBootCore/GeneralBooterConst.h"



namespace 
{
    const int CMaxBinaryTrace = 5;
    const Platform::dword CPrcInterval = 50;
} // namespace 

namespace BfBootCli
{
    //using SBProtoExt::ISbpTransportFactory;
    using SBProtoExt::SbpConnection;
    using SBProtoExt::ISbpConnection;

    BootControlProtoConnection::BootControlProtoConnection(
        iCore::MsgThread& thread,
        IProtoConnectionToOwner& owner,
        boost::shared_ptr<SBProto::ISbpTransport> transport,
		Utils::SafeRef<iLogW::ILogSessionCreator> logCreator
        ) 
        : iCore::MsgObject(thread),
        m_owner(owner),
        m_logSession( logCreator->CreateSession("BootControlProtoConnection", true) ),
        m_prcTimer(this, &BootControlProtoConnection::OnProcess)
        
    {
        SBProtoExt::SbpConnProfile prof;

        // Максимальные таймауты + таймаут в реальных тасках?
        prof.m_sbpSettings.setResponceTimeoutMs(40 * 1000);
        prof.m_sbpSettings.setReceiveEndTimeoutMs(60 * 1000);
        prof.m_sbpSettings.setTimeoutCheckIntervalMs(30 * 1000);                       
        prof.m_sbpSettings.setMaxSendSize(BfBootCore::CMaxChankSize + BfBootCore::CAditionalReservedSize);

        prof.m_logCreator = m_logSession->LogCreator();
        prof.SetTransport(transport);
        prof.m_user = this;
        boost::shared_ptr<SBProtoExt::SbpLogSettings> logSettings(new SBProtoExt::SbpLogSettings(0, CMaxBinaryTrace));
        
        m_connection.reset(new SbpConnection(thread, prof, logSettings));              
        m_prcTimer.Start(CPrcInterval, true);
    }

    void BootControlProtoConnection::ProcessProtocolErr( const std::string& errInfo )
    {
		if(m_connection == 0) return;

		*m_logSession  << m_tagErr 
			<< "Booter protocol error: " << errInfo 
			<< ". Src: " << m_connection->Name() << EndRecord;
        
		m_connection.reset();

        m_owner.DiscWithProtoError(errInfo);
    }

    void BootControlProtoConnection::CommandReceived( SafeRef<ISbpConnection> src, shared_ptr<SBProto::SbpRecvPack> data )
    {
        ESS_ASSERT( src.IsEqualIntf(m_connection.get()) );
        ProcessProtocolErr("Unsupported SbpConnectionEvents \"ComandReceived\"");
    }

    void BootControlProtoConnection::ResponseReceived( SafeRef<ISbpConnection> src, shared_ptr<SBProto::SbpRecvPack> data )
    {
        *m_logSession << m_tagSbpEv << "ResponseReceived from: " << src->Name() << EndRecord;
        ESS_ASSERT( src.IsEqualIntf(m_connection.get()) );

        PackReceived(data);
    }

    void BootControlProtoConnection::InfoReceived( SafeRef<ISbpConnection> src, shared_ptr<SBProto::SbpRecvPack> data )
    {
        ESS_ASSERT( src.IsEqualIntf(m_connection.get()) );
        *m_logSession << m_tagSbpEv << "InfoReceived" << EndRecord;

        PackReceived(data);
        
        //ProcessProtocolErr("Unsupported SbpConnectionEvents \"InfoReceived\"");
    }

    void BootControlProtoConnection::PackReceived( shared_ptr<SBProto::SbpRecvPack> data )
    {
        try
        {
            if ( !m_protoConverter->Process(*data) )  *m_logSession << "Unknown packet." << EndRecord;
        }
        catch(const BfBootCore::StringToTypeConverter::CantConvert& e)
        {
            // ProtoError ?
            *m_logSession << "Packet " << (*data)[0].AsString() 
                << "received, but data conversion failed." 
                << "Location: " << e.getLocation() 
                << EndRecord;
        }
    }

    void BootControlProtoConnection::ProtocolError( SafeRef<ISbpConnection> src, boost::shared_ptr<SBProto::SbpError> err )
    {
        ESS_ASSERT( src.IsEqualIntf(m_connection.get()) );
		*m_logSession << m_tagSbpEv << "Protocol error. Src: " << src->Name() << EndRecord;
        PutMsg(this, &BootControlProtoConnection::ProcessProtocolErr, err->ToString());
    }

    void BootControlProtoConnection::ConnectionActivated( SafeRef<ISbpConnection> src )
    {
        ESS_ASSERT( src.IsEqualIntf(m_connection.get()) );
		*m_logSession << m_tagSbpEv << "Connection activated. Src: " << src->Name() << EndRecord;
        m_owner.Connected();
    }

    void BootControlProtoConnection::ConnectionDeactivated( SafeRef<ISbpConnection> src, const std::string & errInfo )
    {
        ESS_ASSERT( src.IsEqualIntf(m_connection.get()) );
		*m_logSession << m_tagSbpEv << "Connection deactivated. Src: " << src->Name() << EndRecord;        
        PutMsg(this, &BootControlProtoConnection::ProcessProtocolErr, errInfo);
//        ProcessProtocolErr(errInfo);
    }

    void BootControlProtoConnection::SetTrace( bool on )
    {
        BfBootCore::BooterCmdSetTrace::Send(m_connection->Proto(), on);
    }

    void BootControlProtoConnection::SetUserParam( const std::string& name, const std::string& value )
    {        
        BfBootCore::BooterCmdSetUserParam::Send(m_connection->Proto(), name, value);
    }

    void BootControlProtoConnection::SetFactory(Platform::dword hwNumber, Platform::dword hwType, const std::string& mac)
    {        
        BfBootCore::BooterCmdSetBoardId::Send(m_connection->Proto(), hwNumber, hwType, mac);
    }

    void BootControlProtoConnection::GetParamList() const
    {        
        BfBootCore::BooterCmdGetParamList::Send(m_connection->Proto());
    }

    void BootControlProtoConnection::GetAppList() const
    {        
        BfBootCore::BooterCmdGetAppList::Send(m_connection->Proto());
    }

    void BootControlProtoConnection::OpenWriteImg( dword size )
    {        
        BfBootCore::BooterCmdOpenWriteImg::Send(m_connection->Proto(), size);
    }    
    void BootControlProtoConnection::WriteImgChunk(const void* pData, dword size )
    {
        BfBootCore::BooterCmdWriteImgChunk::Send(m_connection->Proto(), pData, size);
    }

    void BootControlProtoConnection::CloseNewImg( const std::string& name, dword CRC )
    {        
        BfBootCore::BooterCmdCloseNewImg::Send(m_connection->Proto(), name, CRC);
    }

    void BootControlProtoConnection::DeleteLastImg()
    {        
        BfBootCore::BooterCmdDeleteLastImg::Send(m_connection->Proto());
    }

    void BootControlProtoConnection::DeleteAllImg()
    {        
        BfBootCore::BooterCmdDeleteAllImg::Send(m_connection->Proto());
    }

    void BootControlProtoConnection::SaveChanges()
    {        
        BfBootCore::BooterCmdSaveChanges::Send(m_connection->Proto());
    }

    void BootControlProtoConnection::OpenWriteBootImg( dword size )
    {        
        BfBootCore::BooterCmdOpenWriteBootImg::Send(m_connection->Proto(), size);
    }

    void BootControlProtoConnection::WriteBooterChunk( const void* pData, dword length )
    {        
        BfBootCore::BooterCmdWriteBooterChunk::Send(m_connection->Proto(), pData, length);
    }

    void BootControlProtoConnection::CloseNewBootImg( dword CRC )
    {        
        BfBootCore::BooterCmdCloseNewBootImg::Send(m_connection->Proto(), CRC);
    }

    void BootControlProtoConnection::EraseDefaultCfg()
    {        
        BfBootCore::BooterCmdEraseDefaultCfg::Send(m_connection->Proto());
    }

    void BootControlProtoConnection::DoLogout( bool withSave )
    {        
        BfBootCore::BooterCmdDoLogout::Send(m_connection->Proto(), withSave);
    }

    void BootControlProtoConnection::DoLogin( const std::string& pwd, dword protoVersion )
    {        
        BfBootCore::BooterCmdLogin::Send(m_connection->Proto(), pwd, protoVersion);
    }

    void BootControlProtoConnection::CloseUpdateTransaction( dword softRelNum )
    {
        BfBootCore::BooterCmdCloseUpdateTransaction::Send(m_connection->Proto(), softRelNum);
    }

    void BootControlProtoConnection::GetScriptList() const
    {
        BfBootCore::BooterCmdGetScriptList::Send(m_connection->Proto());
    }

    void BootControlProtoConnection::RunAditionalScript( const std::string& scriptName, bool withSave)
    {
        BfBootCore::BooterCmdRunAditionalScript::Send(m_connection->Proto(), scriptName, withSave);
    }

    void BootControlProtoConnection::OnProcess( iCore::MsgTimer* )
    {
        if (m_connection) m_connection->Process();
    }

    void BootControlProtoConnection::OnActivateTransport()
    {
        m_connection->ActivateConnection();
    }

    void BootControlProtoConnection::DestroyConnection()
    {        
        m_prcTimer.Stop();
        m_connection.reset();
    }



} // namespace BfBootCli

