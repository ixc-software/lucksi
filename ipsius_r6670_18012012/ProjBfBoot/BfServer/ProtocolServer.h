#ifndef __PROTOCOLSERVER__
#define __PROTOCOLSERVER__


#include "stdafx.h"
#include "Utils/IBasicInterface.h"
#include "Platform/Platform.h"
#include "BfUtils/Log.h"
#include "ITransportServer.h"
#include "TimerBasic.h"
#include "Executer.h"


using namespace Platform;


namespace BfServer
{
    using boost::shared_ptr;

    class ProtocolServer;

    class IProtocolServerToUser : public Utils::IBasicInterface
    {
    public:
        virtual void OnClientDisconnected(ProtocolServer &sender) = 0;

        virtual void OnRequestReceived(ProtocolServer &sender/* todo: Параметры */) = 0;
        virtual void OnError(ProtocolServer &sender/* todo: Параметры */) = 0;
    };


    //-----------------------------------------------------------------------------


    // Реализация протокольного уровня стороны сервера
    class ProtocolServer : public ITransportServerEvents, public ITimerEvents, public BfUtils::LoggedClass
    {
    private:
        boost::shared_ptr<ITransportServer> m_transport;
        IProtocolServerToUser *m_pObserver;

        TimerBasic m_timer;
        dword m_protocolPollingPeriod;


    public:
        ProtocolServer(IExecuter &executer, dword protocolPollingPeriod, BfUtils::ILogStore &logStore, bool enableTracing);
        ~ProtocolServer();


        void BindTransport(boost::shared_ptr<ITransportServer> transport);

        void RegisterObserver(IProtocolServerToUser * const observer);
        void UnregisterObserver();

        void Activate();
        void Deactivate();

        void RequestData();
        void SendResponse(/* todo: arguments */);


    // ITransportServerEvents implementation
    private:
        void OnClientDisconnected(ITransportServer &sender);

        void OnDataReceived(ITransportServer &sender, boost::shared_ptr<Packet> data);

        void OnError(ITransportServer &sender/* todo: Parameters */);

    // ITimerEvents implementation
    private:
        void OnTimer(TimerBasic &sender);
    };

} // namespace BfServer

#endif
