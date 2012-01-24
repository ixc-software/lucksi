#include "stdafx.h"
#include "TransportTCPIPServer.h"
#include "TCPSocketListener.h"


using namespace Platform;


namespace BfServer
{

    TCPSocketListener::TCPSocketListener(IExecuter &executer, dword socketListenerPollingPeriod, dword transportPollingPeriod, ITransportRegistrator &observer, BfUtils::ILogStore &logStore, bool enableTracing) :
        LoggedClass(logStore, enableTracing),
        m_executer(executer),
        m_timer(executer, *this),
        m_transportPollingPeriod(transportPollingPeriod),
        m_observer(observer)
    {
        m_timer.Start(socketListenerPollingPeriod, true);
    }


    //-------------------------------------------------------------------------


    TCPSocketListener::~TCPSocketListener()
    {
    }


    //-------------------------------------------------------------------------


    void TCPSocketListener::OnTimer(TimerBasic &sender)
    {
        // todo: DEBUG BEGIN - delete it as quick as possible
        {
            static int time = GetSystemTickCount();
            static int num = 0;

            if ((num < 10) && (GetSystemTickCount() - time > 150))
            {
                num++;
                time = GetSystemTickCount();

                boost::shared_ptr<ITransportServer> transport(new TransportTCPIPServer(m_executer, m_transportPollingPeriod, m_logStore, true));
                m_observer.OnTransportAvailable(transport);
            }
        }
        // todo: DEBUG END - delete it as quick as possible
    }

} // namespace BfServer
