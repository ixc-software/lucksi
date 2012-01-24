#ifndef __PROTOCOLCLIENT__
#define __PROTOCOLCLIENT__


#include "stdafx.h"
#include "ITransportClient.h"
#include "Utils/IBasicInterface.h"


namespace BfBoot
{

/*
	class Data
    {
        std::string m_data;
        bool        m_isBinary;

    public:
        Argument(const std::string &data, bool isBinary);

        const std::string &getArgument();
        bool               IsBinary();
    };

    typedef std::vector<Argument> ArgumentList;  // stored by value ?!!!
*/

    //-------------------------------------------------------------------------
		

    class ProtocolClient;

    class IProtocolClientToUser : public Utils::IBasicInterface
    {
    public:
        virtual void OnConnected(ProtocolClient &sender) = 0;
        virtual void OnDisconnected(ProtocolClient &sender) = 0;

        virtual void OnResponseReceived(ProtocolClient &sender/* todo: Параметры */) = 0;
        virtual void OnError(ProtocolClient &sender/* todo: Параметры */) = 0;
    };


    //-----------------------------------------------------------------------------


    // Реализация протокольного уровня стороны клиента
    class ProtocolClient : public ITransportClientToProtocol
    {
    private:
        ITransportClient *m_pTransport;
        IProtocolClientToUser &m_observer;


    public:
        ProtocolClient(IProtocolClientToUser &observer);
        ~ProtocolClient();

        void BindTransport(ITransportClient *pTransport); // todo: boost::shared_ptr<>

        void Connect();
        void Disconnect();

        void SendRequest(/* todo: arguments */);

        // ITransportClientToProtocol implementation
        void OnConnected(ITransportClient &sender);
        void OnDisconnected(ITransportClient &sender);

        void OnDataReceived(ITransportClient &sender, boost::shared_ptr<Packet> data);

        void OnError(ITransportClient &sender/* todo: Parameters */);
    };

} // namespace BfBoot

#endif