#ifndef __ITRANSPORTCLIENT__
#define __ITRANSPORTCLIENT__


#include "stdafx.h"
#include "Platform/Platform.h"
#include "Utils/IBasicInterface.h"


using namespace Platform;


namespace BfBoot
{
    typedef std::vector<byte> Packet;


    // Интерфейс транспортного уровня стороны клиента
    class ITransportClient : public Utils::IBasicInterface
    {
    public:
        virtual void Connect() = 0;
        virtual void Disconnect() = 0;

        virtual void SendData(boost::shared_ptr<Packet> data) = 0;
    };


    //-------------------------------------------------------------------------


    // 
    class ITransportClientToProtocol : public Utils::IBasicInterface
    {
    public:
        virtual void OnConnected(ITransportClient &sender) = 0;
        virtual void OnDisconnected(ITransportClient &sender) = 0;

        virtual void OnDataReceived(ITransportClient &sender, boost::shared_ptr<Packet> data) = 0;

        virtual void OnError(ITransportClient &sender/* todo: Parameters */) = 0;
    };


} // namespace BfBoot

#endif