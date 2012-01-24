#include "stdafx.h"
#include "EchoChipCtrl.h"

#include "SafeBiProtoExt/SbpUartTransportFactory.h"
#include "SafeBiProtoExt/SbpConnProfile.h"
#include "SafeBiProtoExt/SbpConnection.h"


namespace TestRealEcho
{

    void EchoChipCtrl::CommandReceived( SafeRef<SBProtoExt::ISbpConnection> src, shared_ptr<SBProto::SbpRecvPack> data )
    {
        ErrorOccured("CommandReceived");
    }

    void EchoChipCtrl::ResponseReceived( SafeRef<SBProtoExt::ISbpConnection> src, shared_ptr<SBProto::SbpRecvPack> data )
    {
        ErrorOccured("ResponseReceived");
    }

    void EchoChipCtrl::InfoReceived( SafeRef<SBProtoExt::ISbpConnection> src, shared_ptr<SBProto::SbpRecvPack> data )
    {
        if (EchoApp::EchoResponse::ProcessPacket(*data, m_owner.MainIntf())) return;
        if (EchoApp::EchoStateResp::ProcessPacket(*data, m_owner.MainIntf())) return;

        ESS_HALT("Unknown packet");
    }

    void EchoChipCtrl::ProtocolError( SafeRef<SBProtoExt::ISbpConnection> src, boost::shared_ptr<SBProto::SbpError> err )
    {
        std::string msg("Protocol error: ");
        msg += err->ToString();
        ErrorOccured(msg);
    }

    void EchoChipCtrl::ConnectionActivated( SafeRef<SBProtoExt::ISbpConnection> src )
    {
        m_owner.Activated();
    }

    void EchoChipCtrl::ConnectionDeactivated( SafeRef<SBProtoExt::ISbpConnection> src, const std::string & errInfo )
    {
        ErrorOccured("ConnectionDeactivated");
    }

    void EchoChipCtrl::OnCmdInit( const EchoApp::CmdInitData &data )
    {
        EchoApp::MainCmdInit::Send(m_connection->Proto(), data);
    }

    void EchoChipCtrl::OnCmdEcho( int chNum, int taps )
    {
        EchoApp::MainCmdEcho::Send(m_connection->Proto(), chNum, taps);
    }

    void EchoChipCtrl::OnCmdRequestState()
    {        
        if (!m_connection) return;
        EchoApp::MainCmdRequestState::Send(m_connection->Proto());
    }

    void EchoChipCtrl::OnCmdShutdown()
    {        
        EchoApp::MainCmdShutdown::Send(m_connection->Proto());
    }

	void EchoChipCtrl::OnCmdRequestHalt()
	{ESS_UNIMPLEMENTED;}

	void EchoChipCtrl::OnCmdSetFreqRecvMode(int ch, const std::string& name, const std::string& param, bool enable)
	{
        EchoApp::MainCmdSetFreqRecvMode::Send(m_connection->Proto(), ch, name, param, enable);
    }

    void EchoChipCtrl::SendUnknownCmd()
    {
        //EchoApp::SendPack p(m_connection->Proto());
        SBProto::SbpSendPackCmd p(m_connection->Proto());
        p.WriteString( "UnknownCmd" );
    }

    shared_ptr<SBProto::ISbpTransport> EchoChipCtrl::CreateUart( int com, int boudRate )
    {
        SBProtoExt::SbpUartProfile uartProf;
        uartProf.BoudRate = boudRate;
        std::string err;
        return SBProtoExt::SbpUartTransportFactory::CreateTransport(com, uartProf, err, false);
    }

    void EchoChipCtrl::ErrorOccured( const std::string& err )
    {        
        m_process.Stop();
        PutMsg(this, &EchoChipCtrl::OnErrorOccured, err);        
    }

    void EchoChipCtrl::OnErrorOccured(const std::string& err)
    {
        m_connection.reset();
        m_owner.Error(err);
    }

    void EchoChipCtrl::OnProcess( iCore::MsgTimer* pTimer )
    {
        if (m_connection) m_connection->Process();
    }

    EchoChipCtrl::EchoChipCtrl( iCore::MsgThread& thread, IEchoChipCtrlOwner& owner, SafeRef<iLogW::ILogSessionCreator> log, int com, int boudRate ) : iCore::MsgObject(thread),
        m_owner(owner),
        m_process(this, &EchoChipCtrl::OnProcess)
    {
        SBProtoExt::SbpConnProfile prof;

        // prof.m_sbpSettings... defaults.

        prof.m_logCreator = log;                       

        prof.SetTransport( CreateUart(com, boudRate) );
        ESS_ASSERT(prof.GetTransport());
        prof.m_user = this; 
        m_connection.reset(new SBProtoExt::SbpConnection(thread, prof/*, logSettings*/));   
        m_connection->ActivateConnection();
        m_process.Start(50, true);
    }

    
   
} // namespace TestRealEcho
