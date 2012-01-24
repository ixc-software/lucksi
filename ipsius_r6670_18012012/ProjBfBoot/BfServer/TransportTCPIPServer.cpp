#include "stdafx.h"
#include "Utils/ErrorsSubsystem.h"
#include "TransportTCPIPServer.h"


using namespace Platform;


namespace BfServer
{

    //-------------------------------------------------------------------------
    // TransportTCPIPServer implementation
    //-------------------------------------------------------------------------


    TransportTCPIPServer::TransportTCPIPServer(IExecuter &executer, dword transportPollingPeriod, BfUtils::ILogStore &logStore, bool enableTracing) :
        LoggedClass(logStore, enableTracing),
        m_timer(executer, *this),
        m_transportPollingPeriod(transportPollingPeriod),
        m_pObserver(0)
    {
        // todo: DEBUG BEGIN - delete it as quick as possible
        time = GetSystemTickCount();
        // todo: DEBUG END - delete it as quick as possible
    }


    //-------------------------------------------------------------------------


    TransportTCPIPServer::~TransportTCPIPServer()
    {
    }


    //-------------------------------------------------------------------------
    // ITransportServer implementation

    void TransportTCPIPServer::RegisterObserver(ITransportServerEvents * const observer)
    {
        ESS_ASSERT(0 == m_pObserver && "TransportTCPIPServer: Observer is already registered!");
        ESS_ASSERT(0 != observer);

        m_pObserver = observer;
    }


    //-------------------------------------------------------------------------


    void TransportTCPIPServer::UnregisterObserver()
    {
        ESS_ASSERT(0 != m_pObserver && "TransportTCPIPServer: Observer isn't registered!");
        ESS_ASSERT(true != m_timer.Enabled() && "TransportTCPIPServer: Timer is enabled!");
        m_pObserver = 0;
    }


    //-------------------------------------------------------------------------


    void TransportTCPIPServer::Activate()
    {
        ESS_ASSERT(0 != m_pObserver && "TransportTCPIPServer: Observer isn't registered!");

        // Start internal Transport timer
        m_timer.Start(m_transportPollingPeriod, true);
    }


    //-------------------------------------------------------------------------


    void TransportTCPIPServer::Deactivate()
    {
        m_timer.Stop();
    }


    //-------------------------------------------------------------------------


    void TransportTCPIPServer::SendData(boost::shared_ptr<Packet> data)
    {
        // todo: 
    }


    //-------------------------------------------------------------------------
    // ITimerEvents implementation

    void TransportTCPIPServer::OnTimer(TimerBasic &sender)
    {
        ESS_ASSERT(0 != m_pObserver);

        // todo:
        // Запрос данных с сокета. Если данные есть - генерирование
        // события OnDataReceived()


        // todo: DEBUG BEGIN - delete it as quick as possible
        if (rand() % 11 == 0)
        {
            LogEvent("TransportTCPIPServer: Generating FAKE OnError()...");
            m_pObserver->OnError(*this);

            return;
        }

        if (rand() % 7 == 0)
        {
            LogEvent("TransportTCPIPServer: Generating FAKE OnClientDisconnected()...");
            m_pObserver->OnClientDisconnected(*this);
        }
        // todo: DEBUG END - delete it as quick as possible
    }


} // namespace BfServer
