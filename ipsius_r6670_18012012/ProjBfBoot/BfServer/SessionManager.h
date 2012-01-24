#ifndef __SESSIONMANAGER__
#define __SESSIONMANAGER__


#include "stdafx.h"
#include "BfUtils/Log.h"
#include "ITransportServer.h"
#include "Executer.h"
#include "TimerBasic.h"
#include "UserSession.h"
#include "Platform/Platform.h"

using namespace Platform;


namespace BfServer
{

    class SessionManager : public ITimerEvents, public BfUtils::LoggedClass
    {
        typedef std::vector<UserSession*> SessionList;

        IExecuter &m_executer;
        TimerBasic m_timer;
        SessionList m_sessions;

        dword m_protocolPollingPeriod;

    public:
        SessionManager(IExecuter &executer, dword sessionClearingPeriod, dword protocolPollingPeriod, BfUtils::ILogStore &logStore, bool enableTracing);
        ~SessionManager();

        void CreateSession(boost::shared_ptr<ITransportServer> transport, IUserSessionEvents &sessionObserver);
        bool IsEmpty();

        // ITimerEvents implementation
        void OnTimer(TimerBasic &sender);
    };

}


#endif