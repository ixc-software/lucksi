#ifndef __LOCALSOCKETREGISTER__
#define __LOCALSOCKETREGISTER__

#include "Utils/IBasicInterface.h"
#include "Utils/SafeRef.h"
#include "iNet/ITcpSocket.h"
#include "Utils/HostInf.h"

namespace TelnetTests
{
    using boost::shared_ptr;


    // test-socket interface
    class ITcpSocketEmul : public iNet::ITcpSocket
    {
    public:
        virtual void setOther(Utils::SafeRef<ITcpSocketEmul> pOther) = 0;
        virtual void Connected() = 0;
        virtual void OnReceiveData(boost::shared_ptr<QByteArray> pData) = 0;
        virtual void resetOther() = 0;
        virtual bool IsConnected() = 0;
    };

    // -----------------------------------------
    // test-server socket interface
    class ITcpServerSocketEmul : public Utils::IBasicInterface
    {
    public:
        virtual void NewConnection(Utils::SafeRef<ITcpSocketEmul> pSender) = 0;
        virtual const Utils::HostInf &LocalInf() const = 0;
    };
    
    // -----------------------------------------
    // class register all test-server sockets in the system
    class LocalSocketRegisterClass
    {
        std::vector<Utils::SafeRef<ITcpServerSocketEmul> > m_list;

        typedef std::vector<Utils::SafeRef<ITcpServerSocketEmul> > List;

        Utils::SafeRef<ITcpServerSocketEmul> Find(const Utils::HostInf &hostInf)
        {
            for (size_t i = 0; i < m_list.size(); ++i)
            {
                if (m_list.at(i)->LocalInf() == hostInf)
                {
                    return m_list.at(i);
                }
            }
            return Utils::SafeRef<ITcpServerSocketEmul>();
        }

    public:
        LocalSocketRegisterClass(){}

        void Register(Utils::SafeRef<ITcpServerSocketEmul> pSocket)
        {
            m_list.push_back(pSocket);
        }

        void Link(Utils::SafeRef<ITcpSocketEmul> pSender, 
                  const Utils::HostInf &receiverHost)
        {
            Utils::SafeRef<ITcpServerSocketEmul> res = Find(receiverHost);

            // thue on server side or if client create before server ... 
            if (res.IsEmpty()) return;

            res->NewConnection(pSender);
        }

        void UnRegister(Utils::SafeRef<ITcpServerSocketEmul> pSocket)
        {
            List::iterator i = std::find(m_list.begin(), m_list.end(), pSocket);
            ESS_ASSERT(i != m_list.end());
            m_list.erase(i);
        }

        ~LocalSocketRegisterClass()
        {
            ESS_ASSERT(m_list.empty());
        }
        
    };

    // --------------------------------------
    
    LocalSocketRegisterClass& LocalSocketRegister();


} //namespace TelnetTest

#endif
