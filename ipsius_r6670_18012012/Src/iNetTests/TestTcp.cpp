#include "stdafx.h"
#include <QDataStream>

#include "Utils/SafeRef.h"
#include "iCore/MsgTimer.h"
#include "iCore/ThreadRunner.h"

#include "iNet/MsgTcpServer.h"

#include "ProfileTcpTest.h"
#include "IFinish.h"
#include "TestUserTcpSocket.h"	

namespace
{
    using namespace iNetTests;

    class TcpSocketOwner :
        public iCore::MsgObject,		
        public IFinish::Item,
        public IFinish
    {
        typedef TcpSocketOwner T; 
    public:
        TcpSocketOwner(iCore::MsgThread &thread, 
            IFinish &owner, 
            ProfileTcpTest &profile) :
            iCore::MsgObject(thread), 
            m_owner(owner),
            m_profile(profile), 
            m_createdUser(0),
            m_finishedUser(0)
        {}

        void Add(TestUserTcpSocket *userOfSocket)
        {
            m_createdUser++;
            //TUT_ASSERT(m_createdUser <= m_profile.getCountClient());
            m_sockets.push_back(userOfSocket);
        }
    
    // IFinish
    private:
       
        void Finish(IFinish::Item *userOfSocket)
        {
            TestUserTcpSocketList::iterator i = m_sockets.begin();
            for (; i != m_sockets.end() && &(*i) != userOfSocket; ++i);
            ESS_ASSERT(i != m_sockets.end() && "Unknown ptr");

            PutMsg(this, &T::OnDeleteUser, m_sockets.release(i).release());
            
            m_finishedUser++;
            m_profile.LogFinishedSocket(m_finishedUser);
            
            if(m_finishedUser == m_profile.getCountClient())
            {
                //TUT_ASSERT(!m_sockets.size() && "Not all client deleted");
                PutMsg(this, &T::OnFinish);
            }
        }
        
        //-----------------------------------------------------        
        
        void OnDeleteUser(TestUserTcpSocket *userOfSocket)
        {

        }        
        
        //-----------------------------------------------------        
        
        void OnFinish()
        {
            TestUserTcpSocketList::iterator i = m_sockets.begin();
            while(i != m_sockets.end())
            {
                i = m_sockets.erase(i);
            }
            m_owner.Finish(this);
        }
            
    // members
    private:
        IFinish &m_owner;
        typedef boost::ptr_vector< boost::nullable<TestUserTcpSocket> > TestUserTcpSocketList;
        TestUserTcpSocketList m_sockets;
        ProfileTcpTest &m_profile;	
        int m_createdUser;
        int m_finishedUser;
        
    };
    
    //--------------------------------------------------------------------------------------
    // Серверная часть теста

    class TestTcpServerSide : 
        virtual public Utils::SafeRefServer,
        public iNet::ITcpServerToUser,
        public IFinish::Item,
        public IFinish
    {
    public:
        TestTcpServerSide(iCore::MsgThread &thread, 
            IFinish &owner, 
            ProfileTcpTest &profile) :
            m_owner(owner),
            m_profile(profile),
			m_sockets(thread, *this, profile),
            m_server(thread, this),
            m_isFinished(false){}
        
        void Run(const Utils::HostInf &host)
        {
			boost::shared_ptr<iNet::SocketError> error = m_server.Listen(host);
        }

        bool isFinished() { return m_isFinished; }

    // ITcpServerToUser
    private:
        void NewConnection(iNet::SocketId id, boost::shared_ptr<iNet::ITcpSocket> serverSocket)
        {
            TestUserTcpSocket *testSocketUser = new TestUserTcpSocket(
                m_sockets.getMsgThread(), 
                m_profile, 
                m_sockets, 
                serverSocket);
            m_sockets.Add(testSocketUser);
        }
        
    // IFinish
    private:
        void Finish(IFinish::Item *item)
        {
            ESS_ASSERT(m_sockets.IsSame(item) && "Incorrect ptr");
            m_isFinished = true;
            m_owner.Finish(this);
        }
        
    // members
    private:
        IFinish &m_owner;        
        ProfileTcpTest &m_profile;
        TcpSocketOwner m_sockets;
        iNet::MsgTcpServer m_server;
        bool m_isFinished;
    };
   
    //--------------------------------------------------------------------------------------
    // Тестирование клиентской части Tcp

    class TestTcpClientSide : 
        public IFinish::Item,
        public IFinish
    {
        typedef TestTcpClientSide T; 
	public:
        TestTcpClientSide(iCore::MsgThread &thread, IFinish &owner, ProfileTcpTest &profile) :
            m_owner(owner),
            m_profile(profile),
			m_sockets(thread, *this, profile),
            m_isFinished(false)
        {}
        
        //----------------------------------------------------------        
        
        void Run(const Utils::HostInf &host)
        {
            for(int i = 0; i < m_profile.getCountClient(); ++i)
            {
                TestUserTcpSocket *testSocketUser = 
                    new TestUserTcpSocket(m_sockets.getMsgThread(), m_profile, m_sockets, i);
                m_sockets.Add(testSocketUser);
                testSocketUser->Start(host);
            }
        }

        bool isFinished() { return m_isFinished; }

    // IFinish
    private:
        void Finish(IFinish::Item *item)
        {
            ESS_ASSERT(m_sockets.IsSame(item) && "Incorrect ptr");
            m_isFinished = true;
            m_owner.Finish(this);
        }
	private:
        IFinish &m_owner;
        ProfileTcpTest &m_profile;
        TcpSocketOwner m_sockets;		
        bool m_isFinished;
    };

    //----------------------------------------------------------
    
    Utils::AtomicInt GCountTestTcp;

    class TestTcp : 
        public boost::noncopyable,
        public iCore::MsgObject, 
        public IFinish
    {
        typedef TestTcp T;

        TestTcpClientSide m_client;
        TestTcpServerSide m_server;
        Utils::AtomicBool &m_done;
    public:
        TestTcp(iCore::IThreadRunner &runner, ProfileTcpTest &profile) :
          iCore::MsgObject(runner.getThread()),
              m_client(runner.getThread(), *this, profile),
              m_server(runner.getThread(), *this, profile),
              m_done(runner.getCompletedFlag())
        {
            ESS_ASSERT(runner.getThread().InCurrentThreadContext());
            GCountTestTcp.Inc();
            m_server.Run(profile.PeerHostInf());
            m_client.Run(profile.PeerHostInf());
        }

        //----------------------------------------------------------            

        ~TestTcp()
        {
            GCountTestTcp.Dec();
        }

        //----------------------------------------------------------            

        void Finish(IFinish::Item *item)
        {
            TUT_ASSERT((m_client.IsSame(item) ||  m_server.IsSame(item)) && "Incorrect ptr");

            if(m_client.isFinished() && m_server.isFinished())
                AsyncSetReadyForDeleteFlag(m_done);
        }
    };
};

//--------------------------------------------------------------------------------------

struct Test{};

namespace iNetTests
{
    void TcpTest(bool silenceMode)
    {
        iCoreTests::Logger log(silenceMode);

        ProfileTcpTest profile(log, Utils::HostInf("127.0.0.1", 3040));
        iCore::ThreadRunner test(Platform::Thread::LowPriority, profile.getMaxTestDuration());

        if (test.Run<TestTcp>("TestTcp", profile))
        {
            TUT_ASSERT(!GCountTestTcp.Get()); 
            log.Log(profile.Result());
            TUT_ASSERT(profile.TestIsOk() && "Error in data flow"); 
            log.Log("Tcp test: Ok!\n");
        }
        else
        {
            TUT_ASSERT(0 && "Timeout");
        }
    }
};


	



