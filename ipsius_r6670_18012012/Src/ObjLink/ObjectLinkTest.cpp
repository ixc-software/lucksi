
#include "stdafx.h"
#include "Domain/DomainTestRunner.h"
#include "iCore/MsgTimer.h"

#include "ObjectLink.h"
#include "ObjectLinkTest.h"
#include "ObjectLinkTestDetail.h"
#include "ObjectLinkBinder.h"

// -------------------------------------------------------

namespace
{
    using namespace ObjLink;
    using namespace Domain;
    using iCore::MsgTimer;

    enum TestMode
    {
        ModeJustMsgSend,     // client destroyed first
        ModeServerDestroyed,
    };

    // -------------------------------------------------------

    class DialServer : 
        public iCore::MsgObject,
        public IObjectLinksHost,
        public IDialInterface
    {
        ObjectLinksServer m_server;
        DomainClass &m_domain;
        TestMode m_mode;
        MsgTimer m_timer;
        ObjLinkStoreBinder m_binder;

        // см. DialClient::StopByTimer
        void StopByTimer(MsgTimer *pTimer)
        {
            MsgObject::AsyncDeleteSelf();
            m_domain.Stop(Domain::DomainExitOk);
        }

    // IObjectLinksHost impl
    private:

        SafeRef<IObjectLinksServer> getObjectLinksServer()
        {
            return m_server.getInterface();
        }

    // IDialInterface impl
    private:

        void Dial(shared_ptr<QString> number) 
        {
            TUT_ASSERT(number->compare("666") == 0);

            if (m_mode == ModeJustMsgSend)
            {
                m_timer.BindEvent(this, &DialServer::StopByTimer);
                m_timer.Start(100);
            }

            if (m_mode == ModeServerDestroyed) 
            {
                MsgObject::AsyncDeleteSelf();
            }

        }

        void SetRegionCode(int code) {}
        void EndCall() {}

    public:

        DialServer(IDomain &domain, TestMode mode) : 
            iCore::MsgObject(domain.getDomain().getMsgThread()), 
            m_server(domain, this), m_domain(domain.getDomain()),
            m_mode(mode), m_timer(this, &DialServer::StopByTimer)
        {
            m_binder.getBinder<IDialInterface>(this);
        }

    };

    // -------------------------------------------------------

    class DialClient : 
        public iCore::MsgObject,
        public IObjectLinkOwner
    {
        DomainClass &m_domain;
        ObjectLink<IDialInterface> m_dialer;
        TestMode m_mode;
        MsgTimer m_timer;

        void StopByTimer(MsgTimer *pTimer)
        {
            // обе операции асинхронны на одной очереди, поэтому тут важен пор€док - !!
            MsgObject::AsyncDeleteSelf();
            m_domain.Stop(Domain::DomainExitOk);
        }

    // IObjectLinkOwner impl
    private:

        void OnObjectLinkConnect(ILinkKeyID &linkID) 
        {
            shared_ptr<QString> number( new QString("666") );
            m_dialer->Dial(number);

            if (m_mode == ModeJustMsgSend) MsgObject::AsyncDeleteSelf();

            // nothing, wait till disconnected
            if (m_mode == ModeServerDestroyed); // m_timer.Start(100);
        }

        void OnObjectLinkDisconnect(ILinkKeyID &linkID) 
        {
            m_dialer->EndCall();  // test call on disconnected
            m_timer.Start(100);
        }

        void OnObjectLinkError(boost::shared_ptr<ObjectLinkError> error) {}

    // IDomain impl
    private:

        DomainClass& getDomain()
        {
            return m_domain;
        }

    public:

        DialClient(IDomain &domain, TestMode mode) :
            iCore::MsgObject(domain.getDomain().getMsgThread()),
            m_domain(domain.getDomain()),
            m_dialer(*this), m_mode(mode), m_timer(this, &DialClient::StopByTimer)
        {
        }

          void ConnectToServer(DialServer &srv)
          {
              m_dialer.Connect(&srv);
              m_dialer->SetRegionCode(0);  // does nothing or must be throw ?!
          }
    };

} // namespace

// -----------------------------------------------------
// -----------------------------------------------------

namespace
{
    using namespace Domain;
    using namespace ObjLink;

    enum { CTestTimeout = 2 * 1000 };

    class ObjLinkTestParams : public IDomainTestRunnerSupport
    {
        TestMode m_mode;

    // IDomainTestRunnerSupport impl
    private:

        void SetDomainTestRunnerControl(IDomainTestRunnerControl &control)
        {
            // ...
        }

    public:
        ObjLinkTestParams(TestMode mode) : m_mode(mode) {}

        TestMode getTestMode() const { return m_mode; }
    };

    // --------------------------------------------------------

    class ObjLinkTest
    {
        // обычные указатели, т.к. объекты сами себ€ удал€ют
        DialServer *m_pServer;
        DialClient *m_pClient;

    public:

        ObjLinkTest(DomainClass &domain, ObjLinkTestParams &params)
        {
            TestMode mode = params.getTestMode();

            m_pServer = new DialServer(domain, mode); 
            m_pClient = new DialClient(domain, mode);

            m_pClient->ConnectToServer(*m_pServer);
        }
    };

    // --------------------------------------------------------

    void RunTest(TestMode mode)
    {
        Domain::DomainTestRunner runner(CTestTimeout);

        ObjLinkTestParams params(mode);

        bool res = runner.Run<ObjLinkTest>(params);
        TUT_ASSERT(res);
    }
}

// -----------------------------------------------------

namespace ObjLink
{

    void RunObjLinkTests()
    {
        RunTest(ModeJustMsgSend);
        RunTest(ModeServerDestroyed); 
    }
}

