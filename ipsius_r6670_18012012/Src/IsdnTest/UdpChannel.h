#ifndef UDPCHANNEL_H
#define UDPCHANNEL_H

#include "stdafx.h"
#include "Utils/SafeRef.h"

#include "iCore/ThreadRunner.h"
#include "iNet/SocketError.h"
#include "iNet/SocketData.h"
#include "iNet/IUdpSocketToUser.h"
#include "iNet/MsgUdpSocket.h"
#include "Platform/Platform.h"
#include "Utils/IBasicInterface.h"
#include "YateL2TestProfile.h"

namespace IsdnTest
{
        
    //-----------------------------------------------------------------------------------------------

    class IChannelToDrv : public Utils::IBasicInterface
    {
    public:
        virtual void DataInput(std::vector<Platform::byte> const &data) = 0;
    };

    class IDrvToChannel : public Utils::IBasicInterface
    {
    public:
        virtual void SendData(std::vector<Platform::byte> const &data) = 0;
    };

    //-----------------------------------------------------------------------------------------------

    //Support exchange data between driver(as intf. IChannelToDrv) and Udp-socket
    class UdpChannel :
        public iCore::MsgObject,
        public iNet::IUdpSocketToUser,
        public IDrvToChannel
    {
        typedef UdpChannel T;

        const CfgHost &m_cfg;

    public:

        UdpChannel(iCore::MsgThread &thread, const CfgHost &cfg);

        void SetDrv(IChannelToDrv* pDrv);

        void SendData(std::vector<Platform::byte> const &data);
          
    // интерфейс ITcpSocketToUser
    private:
        void BindConf(iNet::SocketId socketId);

        void ReceiveData(boost::shared_ptr<iNet::SocketData> socketData);

        void SocketErrorOccur(boost::shared_ptr<iNet::SocketError> error);
    private:

        Utils::HostInf m_remouteInf;
        boost::shared_ptr<iNet::MsgUdpSocket> m_socket;

        IChannelToDrv * m_pDrv;
    };

    //--------------------------------------------------------------------------
} // IsdnTest

#endif
