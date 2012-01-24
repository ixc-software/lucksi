#ifndef __LOCALTESTSOCKETREGISTER__
#define __LOCALTESTSOCKETREGISTER__

#include "Utils/IBasicInterface.h"
#include "Utils/SafeRef.h"
#include "Utils/HostInf.h"
#include "Utils/MemWriterStream.h"

#include "ITestTcpSocketInterfaces.h"

namespace TestUtils
{
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


} //namespace TestUtils

#endif
