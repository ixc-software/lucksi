#ifndef __USERSESSION__
#define __USERSESSION__


#include "stdafx.h"
#include "Utils/IBasicInterface.h"
#include "Platform/Platform.h"
#include "BfUtils/Log.h"
#include "ProtocolServer.h"
#include "Executer.h"
#include "TimerBasic.h"


using namespace Platform;


namespace BfServer
{

    // —обыти€, генерируемые UserSession
    class UserSession;
    class IUserSessionEvents : public Utils::IBasicInterface
    {
    public:
        virtual void OnClientDisconnected(UserSession &sender) = 0;
        virtual void OnRequestReceived(UserSession &sender /* todo: Params */) = 0;
        virtual void OnError(UserSession &sender /* todo: Params */) = 0;
    };


    //-------------------------------------------------------------------------


    //  ласс, представл€ющий пользовательскую сессию (сеанс св€зи)
    class UserSession : public IProtocolServerToUser, public BfUtils::LoggedClass
    {
        boost::shared_ptr<ITransportServer> m_transport;

        ProtocolServer      m_protocol;
        IUserSessionEvents &m_observer;

        bool                m_sessionIsOutdated;


    public:
        UserSession(IExecuter &executer, boost::shared_ptr<ITransportServer> transport, dword protocolPollingPeriod, IUserSessionEvents &observer, BfUtils::ILogStore &logStore, bool enableTracing);
        ~UserSession();

        bool getSessionOutdated() const;
        void setSessionOutdated();


    // IProtocolServerToUser implementation
    private:
        void OnClientDisconnected(ProtocolServer &sender);
        void OnRequestReceived(ProtocolServer &sender);
        void OnError(ProtocolServer &sender);

    };

} // namespace BfServer


#endif
