
#include "stdafx.h"
#include "ModelRun.h"
#include "Domain/DomainClass.h"
#include "AnalogDeviceDriver.h"
#include "Domain/RemoteCallIO.h"
#include "Domain/DomainRPC.h"
#include "Domain/ObjectRemoteHandle.h"
#include "Domain/RemoteCall.h"

#include "ProxyTest_Interface.h"
#include "ProxyTest_Server.h"
#include "ProxyTest_Client.h"


// ---------------------------------------

namespace CallModel
{
    const char *GServerName = "IntfServer";
    const char *GClientName = "IntfClient";

    class ModelRun
    {

        /*
        void InitDomain() 
        {
            using Domain::DomainClass;

            DomainClass::Create("TestDomain", DomainClass::RemotePointMoD(), true);
        } */

    public:

        static void Run(Domain::DomainClass &domain)
        {
            using std::string;

            // server
            new IntfServer(&domain, GServerName);
            NamedObject *pSrv = domain.FindFromRoot(GServerName);
            ESS_ASSERT(pSrv);

            // client
            NamedObject *pClient = new IntfClient(&domain, GClientName, pSrv);

            QString name = QString(GServerName) + "." + QString(GClientName);
            pClient = domain.FindFromRoot(name);
            ESS_ASSERT(pClient);

            pClient = pSrv->FindObject( ObjectName(GClientName) );
            ESS_ASSERT(pClient);

            // cast client
            IntfClient *pClientT = dynamic_cast<IntfClient*>(pClient);
            ESS_ASSERT(pClientT);

            // do test
            pClientT->Link(pSrv->GetFullObjectName());

            // verify
            dynamic_cast<IntfServer&>(*pSrv).AssertTestDone();

            // delete
            pSrv->Delete();

            // работает, а не должно работать, у обоих классов деструкторы потомков protected
            // delete pClientT;  

            // OK
            std::cout << "Ok!" << std::endl;

            // device
            // AnalogDeviceDriver drv(ObjectName("Dev"), 1, Domain::RemotePointMoD());

        }
    };

    // ---------------------------------------

    void RunCallModel()
    {
        using Domain::DomainClass;

        //DomainClass domain("TestDomain", DomainClass::RemotePointMoD(), true);

        // ModelRun::Run(domain);
    }


} // namespace CallModel


