#ifndef __TRANSPORTSERVER__
#define __TRANSPORTSERVER__


#include "stdafx.h"
#include "Platform/Platform.h"
#include "Utils/IBasicInterface.h"
#include "Executer.h"


using namespace Platform;


namespace BfServer
{
    typedef std::vector<byte> Packet;


    // События, генерируемые транспортом (наследниками ITransportServer)
    class ITransportServer;
    class ITransportServerEvents : public Utils::IBasicInterface
    {
    public:
        virtual void OnClientDisconnected(ITransportServer &sender) = 0;
        virtual void OnDataReceived(ITransportServer &sender, boost::shared_ptr<Packet> data) = 0;
        virtual void OnError(ITransportServer &sender/* todo: Parameters */) = 0;
    };


    //-------------------------------------------------------------------------


    // Интерфейс транспортного уровня стороны сервера
    class ITransportServer : public Utils::IBasicInterface
    {
    public:
        virtual void RegisterObserver(ITransportServerEvents * const observer) = 0;
        virtual void UnregisterObserver() = 0;

        virtual void Activate() = 0;
        virtual void Deactivate() = 0;

        virtual void SendData(boost::shared_ptr<Packet> data) = 0;
    };


    //-------------------------------------------------------------------------


    class ITransportError : public Utils::IBasicInterface
    {
    public:
        virtual void getString(const std::string &Error) = 0;
    };

} // namespace BfServer


#endif