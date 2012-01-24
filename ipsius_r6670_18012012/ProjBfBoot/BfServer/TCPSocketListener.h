#ifndef __TCPSOCKETLISTENER___
#define __TCPSOCKETLISTENER___


#include "Utils/IBasicInterface.h"
#include "Platform/Platform.h"
#include "BfUtils/Log.h"
#include "Executer.h"
#include "TimerBasic.h"
#include "ITransportServer.h"
#include "BooterServer.h"


using namespace Platform;


namespace BfServer
{

    // Класс-wrapper для TCP-сокета
    class TCPSocketListener : public ITimerEvents, public BfUtils::LoggedClass
    {
        ITransportRegistrator &m_observer;
        TimerBasic m_timer;
        IExecuter &m_executer;

        dword m_transportPollingPeriod;


    public:
        TCPSocketListener(IExecuter &executer, dword socketListenerPollingPeriod, dword transportPollingPeriod, ITransportRegistrator &observer, BfUtils::ILogStore &logStore, bool enableTracing);
        ~TCPSocketListener();

    // ITimerEvents implementation
    private:
        void OnTimer(TimerBasic &sender);
    };

} // namespace BfServer


#endif
