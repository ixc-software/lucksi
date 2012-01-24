#include "stdafx.h"
#include "Domain/DomainTestRunner.h"
#include "iCore/MsgTimer.h"

#include "ObjectLink.h"
#include "ObjectLinkTest2.h"
#include "ObjectLinkTestDetail.h"
#include "ObjectLinkBinder.h"

// ------------------------------------------------------

namespace
{
    using namespace Domain;
    using namespace ObjLink;

    class DialServer : 
        public iCore::MsgObject,
        public IObjectLinksHost,
        public IDialInterface
    {
        ObjectLinksServer m_server;
        DomainClass &m_domain;

    // IObjectLinksHost impl
    private:

        SafeRef<IObjectLinksServer> getObjectLinksServer()
        {
            return m_server.getInterface();
        }

    // IDialInterface impl
    private:

        void Dial(shared_ptr<QString> number) {}
        void SetRegionCode(int code) {}

        void EndCall() 
        {
            m_domain.Stop(Domain::DomainExitOk);
        }

    public:

        DialServer(IDomain &domain) : 
          iCore::MsgObject(domain.getDomain().getMsgThread()), 
          m_server(domain, this), m_domain(domain.getDomain())
       {
       }

    };

    // -------------------------------------------------

    class DialClient : 
        public iCore::MsgObject,
        public IObjectLinkOwner
    {
        DomainClass &m_domain;
        ObjectLink<IDialInterface> m_dialer;

    // IObjectLinkOwner impl
    private:

        void OnObjectLinkConnect(ILinkKeyID &linkID) 
        {
            // проверка "досылки" сообщений после отключения линка клиентом
            m_dialer->EndCall();
            m_dialer.Disconnect();

            // assertion test
            bool wasException = false;

            {
                // translate assert to exception ESS::HookRethrow
                ESS::ExceptionHookRethrow<> globalHook;  

                try
                {
                    m_dialer->EndCall();  
                }
                catch (ESS::HookRethrow &e)
                {
                    wasException = true;
                }
            }

            ESS_ASSERT(wasException);
        }

        void OnObjectLinkDisconnect(ILinkKeyID &linkID) 
        {
        }

        void OnObjectLinkError(boost::shared_ptr<ObjectLinkError> error) {}

    // IDomain impl
    private:

        DomainClass& getDomain()
        {
            return m_domain;
        }

    public:

        DialClient(IDomain &domain) :
          iCore::MsgObject(domain.getDomain().getMsgThread()),
          m_domain(domain.getDomain()), m_dialer(*this)
       {
       }

       void ConnectToServer(DialServer &srv)
       {
          m_dialer.Connect(&srv);
       }
    };



}  // namespace


// ------------------------------------------------------

namespace
{
    using namespace Domain;
    using namespace ObjLink;

    enum { CTestTimeout = 2 * 1000 };

    class ObjLinkTestParams
    {

    public:
        ObjLinkTestParams() {}
    };


    class ObjLinkTest
    {
        DialServer m_server;
        DialClient m_client;

    public:

        ObjLinkTest(DomainClass &domain, ObjLinkTestParams &params)
            : m_server(domain), m_client(domain)
        {
            m_client.ConnectToServer(m_server);
        }

    };


}  // namespace


// ------------------------------------------------------

namespace ObjLink
{

    void RunObjLinkTests2()
    {
        Domain::DomainTestRunner runner(CTestTimeout);

        ObjLinkTestParams params;

        bool res = runner.Run<ObjLinkTest>(params);
        TUT_ASSERT(res);

    }

}  // namespace ObjLink

