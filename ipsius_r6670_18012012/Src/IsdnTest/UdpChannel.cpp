#include "stdafx.h"
#include "UdpChannel.h"

namespace IsdnTest
{
    UdpChannel::UdpChannel(iCore::MsgThread &thread, const CfgHost &cfg) :
        iCore::MsgObject(thread), m_remouteInf(cfg.hostInf()),
        m_socket(new iNet::MsgUdpSocket(thread, this, cfg.localPotr())),
        m_pDrv(0),
        m_cfg(cfg)
    {
       //m_socket->Bind(cfg.localPotr());
    }

    //--------------------------------------------------------------------------

    void UdpChannel::SetDrv(IChannelToDrv* pDrv)
    {
        m_pDrv = pDrv;
    }

    //--------------------------------------------------------------------------

    void UdpChannel::SendData(std::vector<Platform::byte> const &data)
    {
        QByteArray dataArray;
        for (int i = 0; i < data.size(); ++i)
        {
            dataArray.push_back(data[i]);
        }

        boost::shared_ptr<iNet::SocketData> socketData(
          new iNet::SocketData(m_remouteInf.getAddress(), m_cfg.localPotr(), dataArray));

        m_socket->SendData(socketData);
    }

    //--------------------------------------------------------------------------          
    // интерфейс ITcpSocketToUser
    void UdpChannel::BindConf(iNet::SocketId socketId)
    {
        ESS_ASSERT(m_socket->IsEqual(socketId));  
    }

    //--------------------------------------------------------------------------

    void UdpChannel::ReceiveData(boost::shared_ptr<iNet::SocketData> socketData)
    {
        ESS_ASSERT(m_socket->IsEqual(socketData->ID()));            

        std::vector<Platform::byte> vbData;
        QByteArray& baData = socketData->getData();

        for (int i = 0; i < baData.size(); ++i)
        {
            vbData.push_back(baData[i]);
        }

        m_pDrv->DataInput(vbData);
    }

    //-----------------------------------------------------------        

    void UdpChannel::SocketErrorOccur(boost::shared_ptr<iNet::SocketError> error)
    {
        ESS_ASSERT(m_socket->IsEqual(error->ID()));
    }

    //--------------------------------------------------------------------------
};
