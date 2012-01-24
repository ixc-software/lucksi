#include "stdafx.h"
#include "ProtocolServer.h"
#include "Utils/ErrorsSubsystem.h"


using namespace ESS;


namespace BfServer
{
    //-------------------------------------------------------------------------
    // ProtocolServer implementation
    //-------------------------------------------------------------------------


    ProtocolServer::ProtocolServer(IExecuter &executer, dword protocolPollingPeriod, BfUtils::ILogStore &logStore, bool enableTracing) :
        LoggedClass(logStore, enableTracing),
        m_timer(executer, *this),
        m_protocolPollingPeriod(protocolPollingPeriod),
        m_pObserver(0)
    {        
    }


    //-------------------------------------------------------------------------


    ProtocolServer::~ProtocolServer()
    {
    }


    //-------------------------------------------------------------------------


    void ProtocolServer::BindTransport(shared_ptr<ITransportServer> transport)
    {
        ESS_ASSERT(transport.get() != 0);
        ESS_ASSERT(m_transport.get() == 0); // Transport is binded already!

        m_transport = transport;
    }


    //-------------------------------------------------------------------------


    void ProtocolServer::RegisterObserver(IProtocolServerToUser * const observer)
    {
        ESS_ASSERT(0 == m_pObserver);
        ESS_ASSERT(0 != observer);

        m_pObserver = observer;
    }


    //-------------------------------------------------------------------------


    void ProtocolServer::UnregisterObserver()
    {
        ESS_ASSERT( (0 != m_pObserver) && "ProtocolServer: Observer isn's registered!");
        ESS_ASSERT( (true != m_timer.Enabled()) && "ProtocolServer: Timer is enabled!");

        m_pObserver = 0;
    }


    //-------------------------------------------------------------------------


    void ProtocolServer::Activate()
    {
//       ESS_ASSERT(0 != m_pTransport && "ProtocolServer: Transport isn't binded!");
       ESS_ASSERT(0 != m_pObserver && "ProtocolServer: Observer isn's registered!");

       m_timer.Start(m_protocolPollingPeriod, true);
    }


    //-------------------------------------------------------------------------


    void ProtocolServer::Deactivate()
    {
        m_timer.Stop();
    }


    //-------------------------------------------------------------------------


    void ProtocolServer::SendResponse(/* todo: arguments */)
    {
        ESS_ASSERT(0 != m_transport.get());

        // todo: 
        // √енерирование массива данных из команды и отправка его на
        // транспортный уровень
    }


    //-------------------------------------------------------------------------
    // IProtocolServerToUser implementation

    void ProtocolServer::OnClientDisconnected(ITransportServer &sender)
    {
        ESS_ASSERT(0 != m_pObserver);

        m_timer.Stop();
        m_pObserver->OnClientDisconnected(*this);
    }


    //-------------------------------------------------------------------------


    void ProtocolServer::OnDataReceived(ITransportServer &sender, boost::shared_ptr<Packet> data)
    {
        ESS_ASSERT(0 != m_pObserver);

        // todo:
        // ѕрием, накопление и парсинг данных.  огда команда была полностью
        // получена и разобрана, генерируетс€ событие OnRequestReceived().
        // ¬ случае таймаута или ошибки синтаксиса генерируетс€ OnError().

        // m_pObserver->OnRequestReceived(*this, data);
    }


    //-------------------------------------------------------------------------


    void ProtocolServer::OnError(ITransportServer &sender/* todo: Parameters */)
    {
        ESS_ASSERT(0 != m_pObserver);

        m_pObserver->OnError(*this);
    }


    //-------------------------------------------------------------------------
    // ITimerEvents implementation

    void ProtocolServer::OnTimer(TimerBasic &sender)
    {
        ESS_ASSERT(0 != m_pObserver);

        // todo: Protocol activity
    }

} // namespace BfServer
