#ifndef __TRANSPORTTCPIPSERVER__
#define __TRANSPORTTCPIPSERVER__


#include "stdafx.h"
#include "BfUtils/Log.h"
#include "Platform/Platform.h"
#include "ITransportServer.h"
#include "TimerBasic.h"
#include "Executer.h"


using namespace Platform;


namespace BfServer
{

    // Реализация интерфейса транспортного уровня стороны сервера для TCP/IP
    class TransportTCPIPServer : public ITransportServer, public ITimerEvents, public BfUtils::LoggedClass
    {
        ITransportServerEvents *m_pObserver;

        TimerBasic m_timer;
        dword m_transportPollingPeriod;

        // todo: BEGIN DEBUG - delete it as quick as possible
        int time;
        // todo: END DEBUG - delete it as quick as possible

    public:
        TransportTCPIPServer(IExecuter &executer, dword transportPollingPeriod, BfUtils::ILogStore &logStore, bool enableTracing);
        ~TransportTCPIPServer();


    // ITimerEvents implementation
    private:
        void OnTimer(TimerBasic &sender);

        
    // ITransportServer implementation
    private:
        void RegisterObserver(ITransportServerEvents * const observer);
        void UnregisterObserver();

        void Activate();
        void Deactivate();

        void SendData(boost::shared_ptr<Packet> data);
    };

} // namespace BfServer

#endif
