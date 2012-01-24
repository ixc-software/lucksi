#include "stdafx.h"
#include "BroadcastAutoSender.h"
#include "BfBootCore/GeneralBooterConst.h"
#include "E1App/MngLwip.h"
#include "BfDev/BfWatchdog.h"
#include "BfBootCore/UdpCommandsOfAutoSender.h"

namespace BfBootSrv
{

    void BroadcastAutoSender::Process( iCore::MsgTimer* )
    {
        Listen();
        Send();              
    }

    // ------------------------------------------------------------------------------------

    void BroadcastAutoSender::SetBroadcastDest()
    {
        //todo bcAddr узнавать у глобального стека 
        Utils::HostInf broadcastAddress("255.255.255.255", BfBootCore::CBroadcastClientPort);
        m_socket.SetDestination( broadcastAddress );   
    }

    // ------------------------------------------------------------------------------------

    template<class TCmd>
    bool BroadcastAutoSender::IsCmd( Lw::Packet& pack )
    {
        TCmd::getBin(m_recData);
        return m_recData == pack;
    }   

    // ------------------------------------------------------------------------------------

    void BroadcastAutoSender::Listen()
    {        
        Utils::HostInf addr;
        if (!m_socket.Recv(m_pack, addr)) return;        
        
        if ( IsCmd<BfBootCore::CmdInfoRequest>(m_pack) )
        {            
            m_socket.SetDestination(addr);
            Send(true);             
            SetBroadcastDest(); // restore destination
        }        

        if ( IsCmd<BfBootCore::CmdReload>(m_pack) ) 
            m_pReboot->AsyncReboot();                    
    }

    // ------------------------------------------------------------------------------------

    void BroadcastAutoSender::Send(bool direct/* = false*/)
    {
        if (direct) m_socket.Send(m_msgPacketDirectRequest);
        else        m_socket.Send(m_msgPacketBroadcast);
    }

    // ------------------------------------------------------------------------------------

    BroadcastAutoSender::BroadcastAutoSender( 
        iCore::MsgThread& thread, 
        BfBootCore::BroadcastMsg& msg,
        int bindPort,
        ICloseApp* pReboot/* = 0*/
        ) 
        : MsgObject(thread),                    
        m_pReboot(pReboot),
        m_msg(msg),
        m_tSend(this, &BroadcastAutoSender::Process),
        m_sendPeriod(BfBootCore::CBroadcastSendPeriod)
    {
        ESS_ASSERT(E1App::Stack::IsInited());                   
        ESS_ASSERT(m_socket.Bind(bindPort));        
        
        SetBroadcastDest();        
    }

    // ------------------------------------------------------------------------------------
    
    void BroadcastAutoSender::Start(int cmpPort) // in booter cmpPort must be 0
    {
        if (m_msgPacketBroadcast.empty() || m_msg.CmpPort != cmpPort)
        {
            m_msg.CmpPort = cmpPort;
            UpdateAllPacket();
        }             

        Send();
        m_tSend.Start(m_sendPeriod, true);
    }

    // ------------------------------------------------------------------------------------

    void BroadcastAutoSender::Stop()
    {
        m_tSend.Stop();
    }

    void BroadcastAutoSender::UpdateAllPacket()
    {   
        m_msg.MarkAsDirect(false);
        UpdatePacket(m_msg, m_msgPacketBroadcast);        
        m_msg.MarkAsDirect(true);
        UpdatePacket(m_msg, m_msgPacketDirectRequest);
    }

    void BroadcastAutoSender::UpdatePacket( const BfBootCore::BroadcastMsg& msg, Lw::Packet& pack )
    {
        msg.ToBin(pack);
        ESS_ASSERT(m_socket.MaxUpdPackSize() >= pack.size()); 
    }
    void BroadcastAutoSender::UpdateOwnerInfo( const std::string& info )
    {
        if (m_msg.OwnerAddress == info) return;
        m_msg.OwnerAddress = info;
        UpdateAllPacket();
    }

    
    
} // namespace BfBootSrv

