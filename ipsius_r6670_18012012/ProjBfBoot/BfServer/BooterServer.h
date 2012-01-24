#ifndef __BOOTERSERVER__
#define __BOOTERSERVER__


#include "BfUtils/Log.h"
#include "Executer.h"
#include "TimerBasic.h"
#include "ProtocolServer.h"
#include "SessionManager.h"
#include "BfServerConfig.h"

#include "Platform/Platform.h"
using namespace Platform;


namespace BfServer
{
    // »нтерфейс дл€ регистрировани€ транспортов
    class ITransportRegistrator
    {
    public:
        virtual void OnTransportAvailable(boost::shared_ptr<ITransportServer> transport) = 0;
    };


    //-------------------------------------------------------------------------


    //  ласс, представл€ющий сервер загрузчика
    class BooterServer : public ITimerEvents, public IUserSessionEvents, public ITransportRegistrator, public BfUtils::LoggedClass
    {
        typedef std::vector<UserSession*> SessionList;

        // BooterServer configuration
        BfServerConfig &m_config;

        IExecuter &m_executer;
        boost::scoped_ptr<SessionManager> m_pSessionManager;
        TimerBasic m_waitConnectionTimer;


    public:
        BooterServer(BfServerConfig &config, IExecuter &executer, BfUtils::ILogStore &logStore, bool enableTracing);
        ~BooterServer();


    // ITimerEvent implementation
    private:
        void OnTimer(TimerBasic &sender);


    // ITransportRegistrator implementation
    private:
        void OnTransportAvailable(boost::shared_ptr<ITransportServer> transport);


    // IUserSessionEvents implementation
    private:
        void OnClientDisconnected(UserSession &sender);
        void OnRequestReceived(UserSession &sender);
        void OnError(UserSession &sender);
    };

} // namespace BfServer


#endif