#include "stdafx.h"
#include "BroadcastReceiver.h"

namespace 
{
    // for debug
    const bool CNoResetDev = false;

    template<class TCmd>
    void SendCmd(iNet::MsgUdpSocket& socket, const Utils::HostInf& addr)
    {
        std::vector<Platform::byte> cmdData;
        TCmd::getBin(cmdData);

        boost::shared_ptr<iNet::SocketData> cmd(
            new iNet::SocketData(addr, 
            QByteArray(reinterpret_cast<const char*>(&cmdData[0]), cmdData.size())) 
            );

        socket.SendData(cmd);
    }

} // namespace 

namespace BfBootCli
{

    void BroadcastReceiver::ReceiveData( boost::shared_ptr<iNet::SocketData> data )
    {
        const char* pChar = data->getData();
        BfBootCore::BroadcastMsg msg;
        
        if ( !msg.Assign(data->getHostinf(), reinterpret_cast<const Platform::byte*>(pChar), data->getData().size()))
        {
            ++m_trashCounter;
            return;
        }
        
        //msg.SrcAddress.Address();
        m_callback.MsgReceived(msg);        
    }

    void BroadcastReceiver::SocketErrorOccur( boost::shared_ptr<iNet::SocketError> error )
    {        
        // ESS_UNIMPLEMENTED;
    }

    BroadcastReceiver::BroadcastReceiver( iCore::MsgThread& thread, IBroadcastReceiverToOwner& callback, int listenPort ) : m_callback(callback),
        m_socket(thread, this), m_trashCounter(0)
    {
        if (!m_socket.Bind(listenPort))
        {
            ESS_THROW(BindError);
        }
    }

    void BroadcastReceiver::ResetDevice( const BfBootCore::BroadcastMsg& msg )
    {                        
        if (CNoResetDev) return;
        SendCmd<BfBootCore::CmdReload>(m_socket, msg.SrcAddress);
    }

    int BroadcastReceiver::WrongMsgCount() const
    {
        return m_trashCounter;
    }

    void BroadcastReceiver::DirectRequest( const Utils::HostInf& addr )
    {
        SendCmd<BfBootCore::CmdInfoRequest>(m_socket, addr);
    }
} // namespace BfBootCli

