#ifndef __PROXYTEST_SERVER__

#define __PROXYTEST_SERVER__

#include "stdafx.h"

#include "Domain/NamedObject.h"
#include "Utils/ErrorsSubsystem.h"

#include "ProxyTest_Interface.h"

namespace CallModel
{
    using Domain::NamedObject;

    // класс-сервер, реализующий ITestInterface
    class IntfServer 
        : public NamedObject, 
          public ITestInterface 
          // public Domain::IProxyServer
    {
        // DefaultInterfaceHoster m_host;
        bool m_testDone;

    public:

        IntfServer(Domain::IDomain *pDomain, const ObjectName &name) 
            : NamedObject(pDomain, name), m_testDone(false)
        {
            // ...
        }

        void AssertTestDone()
        {
            ESS_ASSERT(m_testDone);
        }

        // implementation ITestInterface
        void Ping()
        {
            m_testDone = true;
        }

        void SetName(boost::shared_ptr<QString> name)
        {
            // ...
        }
        
        void SetNameVal(QString name)
        {
        }

        // implementation Domain::IProxyServer
        /*
        IInterfaceHoster& InterfaceHoster()
        {
            return m_host;
        } */

    };


}  // namespace CallModel


#endif
