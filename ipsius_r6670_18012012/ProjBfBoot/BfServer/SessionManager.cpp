#include "stdafx.h"
#include "Utils/ErrorsSubsystem.h"
#include "SessionManager.h"


namespace BfServer
{

    SessionManager::SessionManager(IExecuter &executer, dword sessionClearingPeriod, dword protocolPollingPeriod, BfUtils::ILogStore &logStore, bool enableTracing) :
        LoggedClass(logStore, enableTracing),
        m_executer(executer),
        m_timer(executer, *this),
        m_protocolPollingPeriod(protocolPollingPeriod)
    {
        m_timer.Start(sessionClearingPeriod, true);
    }


    //-------------------------------------------------------------------------


    SessionManager::~SessionManager()
    {
        SessionList::iterator it = m_sessions.begin();
        for (; it != m_sessions.end(); )
        {
            delete (*it);
            it = m_sessions.erase(it);
        }

        ESS_ASSERT(m_sessions.empty());
    }


    //-------------------------------------------------------------------------


    void SessionManager::CreateSession(boost::shared_ptr<ITransportServer> transport, IUserSessionEvents &sessionObserver)
    {
        UserSession *session = new UserSession(m_executer, transport, m_protocolPollingPeriod, sessionObserver, m_logStore, true);
        m_sessions.push_back(session);
    }


    //-------------------------------------------------------------------------


    bool SessionManager::IsEmpty()
    {
        return m_sessions.empty();
    }


    //-------------------------------------------------------------------------
    // ITimerEvents implementation

    void SessionManager::OnTimer(TimerBasic &sender)
    {
        if (m_sessions.empty()) return;

        SessionList::iterator it = m_sessions.begin();
        for (; it != m_sessions.end(); )
        {
            if ((*it)->getSessionOutdated())
            {
                // Remove an outdated User Session from session list and delete it
                delete (*it);
                it = m_sessions.erase(it);

                LogEvent("SessionManager: Outdated Session deleted\n");
            }
            else
            {
                ++it;
            }
        }
    }

}
