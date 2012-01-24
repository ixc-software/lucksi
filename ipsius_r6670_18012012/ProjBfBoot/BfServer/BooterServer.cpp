#include "stdafx.h"
#include "Utils/ErrorsSubsystem.h"
#include "BooterServer.h"
#include "TransportTCPIPServer.h"
#include "UserSession.h"
#include "BfUtils/StringConvertions.h"


using namespace BfUtils;
using std::string;


namespace BfServer
{

    //-------------------------------------------------------------------------
    // BooterServer implementation
    //-------------------------------------------------------------------------

    BooterServer::BooterServer(BfServerConfig &config, IExecuter &executer, BfUtils::ILogStore &logStore, bool enableTracing) :
        LoggedClass(logStore, enableTracing),
        m_config(config),
        m_executer(executer),
        m_waitConnectionTimer(executer, *this)
    {
        m_pSessionManager.reset(new SessionManager(
            executer,
            m_config.SessionClearingPeriod.getValue(),
            m_config.ProtocolPollingPeriod.getValue(),
            logStore, true));

        m_waitConnectionTimer.Start( m_config.WaitConnectionTimeout.getValue(), false );
    }


    //-------------------------------------------------------------------------


    BooterServer::~BooterServer()
    {
    }


    //-------------------------------------------------------------------------
    // ITimerEvent implementation

    void BooterServer::OnTimer(TimerBasic &sender)
    {
        LogEvent("BooterServer: Time is out. Stopping WaitConnectionTimer...");

        if (m_pSessionManager->IsEmpty())
        {
            // Nobody was connected before timeout. Stop the Server
            LogEvent("BooterServer: No users connected. Stopping Server...\n");

            // todo:
//            StopServer();
        }
        else
        {
            // Continue work because someone is connected
            LogEvent("BooterServer: User(s) was(were) connected. Continue working...\n");
        }

        // Stop waitingConnectionsTimer
        sender.Stop();
    }


    //-------------------------------------------------------------------------
    // ITransportRegistrator implementation

    void BooterServer::OnTransportAvailable(boost::shared_ptr<ITransportServer> transport)
    {
        LogEvent("BooterServer: OnTransportAvailable()\n");

        // Create User Session with new Transport
        m_pSessionManager->CreateSession(transport, *this);

        // Stop waitConnectionTimer
        if ( true == m_waitConnectionTimer.Enabled() )
        {
            LogEvent("BooterServer: First connection detected! Stopping waitConnectionTimer...\n");
            m_waitConnectionTimer.Stop();
        }
    }


    //-------------------------------------------------------------------------
    // IUserSessionEvents implementation

    void BooterServer::OnClientDisconnected(UserSession &sender)
    {
        LogEvent("BooterServer: OnClientDisconnected()");

        // Mark the disconnected User Session as out-of-date
        sender.setSessionOutdated();
    }


    //-------------------------------------------------------------------------


    void BooterServer::OnRequestReceived(UserSession &sender)
    {
        // todo: Обработать данные
    }


    //-------------------------------------------------------------------------


    void BooterServer::OnError(UserSession &sender)
    {
        LogEvent("BooterServer: OnError()!");

        // Mark the failed User Session as out-of-date
        sender.setSessionOutdated();
    }

} // namespace BfServer
