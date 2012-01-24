#ifndef __PROXYTEST_CLIENT__

#define __PROXYTEST_CLIENT__

#include "stdafx.h"
#include "Domain/DomainClass.h"
#include "ProxyTest_Interface.h"

namespace CallModel
{

    using Domain::FullObjectName;
    using Domain::ObjectName;
    using Domain::NamedObject;


    // класс-клиент, подлкючащийся к серверу интерфейса ITestInterface
    class IntfClient 
        : public NamedObject // , public Domain::IInterfaceOwner
    {
        // ProxyLink<ITestInterface> m_proxy;

        // ~IntfClient() {}

    public:

        IntfClient(Domain::IDomain *pDomain, const ObjectName &name, NamedObject *pParent) 
            : NamedObject(pDomain, name, pParent)
        {
        }

        void Link(const FullObjectName &name)
        {
            // m_proxy.Connect(this, name);
        }

        // implementation Domain::IInterfaceOwner
        /*
        void OnRemoteInterfaceConnect(ProxyIntfBase &intf)
        {
            //ESS_ASSERT(m_proxy.EqualProxy(intf));
            // m_proxy->Ping();
        }

        void OnRemoteInterfaceDisconnect(ProxyIntfBase &intf)
        {
            // ...
        }

        void OnRemoteInterfaceError(ProxyIntfBase &intf, const ProxyIntfError &error)
        {
            // ...
        } */

        Domain::DomainClass& getDomain()
        {
            return NamedObject::getDomain();
        }

    };


}  // namespace CallModel


#endif
