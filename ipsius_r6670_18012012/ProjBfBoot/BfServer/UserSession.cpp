#include "stdafx.h"
#include "Utils/ErrorsSubsystem.h"
#include "UserSession.h"
#include "Executer.h"
#include "ProtocolServer.h"
#include "TransportTCPIPServer.h"


namespace BfServer
{

    //-------------------------------------------------------------------------
    // UserSession implementation
    //-------------------------------------------------------------------------


    UserSession::UserSession(IExecuter &executer, boost::shared_ptr<ITransportServer> transport, dword protocolPollingPeriod, IUserSessionEvents &observer, BfUtils::ILogStore &logStore, bool enableTracing) :
        LoggedClass(logStore, enableTracing),
        m_observer(observer),
        m_transport(transport),
        m_protocol(executer, protocolPollingPeriod, logStore, true),
        m_sessionIsOutdated(false)
    {
        // Bind Transport and Protocol to each other
        m_protocol.BindTransport(m_transport);
        m_protocol.RegisterObserver(this);
        m_transport->RegisterObserver(&m_protocol);

        m_transport->Activate();
        m_protocol.Activate();
    }


    //-------------------------------------------------------------------------


    UserSession::~UserSession()
    {
        m_transport.reset();
        ESS_ASSERT(0 == m_transport.get() && "Transport is owned by someone else!");
    }


    //-------------------------------------------------------------------------


    bool UserSession::getSessionOutdated() const
    {
        return m_sessionIsOutdated;
    }


    //-------------------------------------------------------------------------


    void UserSession::setSessionOutdated()
    {
        ESS_ASSERT(true != m_sessionIsOutdated && "User Session is already outdated!");

        m_transport->Deactivate();
        m_protocol.Deactivate();

        m_sessionIsOutdated = true;
    }


    //-------------------------------------------------------------------------
    // IUserSessionEvents implementation

    void UserSession::OnClientDisconnected(ProtocolServer &sender)
    {
        m_observer.OnClientDisconnected(*this);
    }


    //-------------------------------------------------------------------------


    void UserSession::OnRequestReceived(ProtocolServer &sender)
    {
        m_observer.OnRequestReceived(*this);
    }


    //-------------------------------------------------------------------------


    void UserSession::OnError(ProtocolServer &sender)
    {
        m_observer.OnError(*this);
    }

} // namespace BfServer
