#ifndef __TCPSRVTESTSRVSIDE__
#define __TCPSRVTESTSRVSIDE__

#include "iCore/MsgObject.h"
#include "iCore/MsgThread.h"
#include "iNet/MsgTcpServer.h"
#include "iNet/ITcpSocketToUser.h"
#include "iNet/ITcpSocket.h"
#include "Utils/HostInf.h"
#include "Utils/QtHelpers.h"
#include "Utils/ManagedList.h"
#include "Utils/SafeRef.h"
#include "ITestInfra.h"
#include "DataStream.h"

namespace SockTest
{
    using iCore::MsgObject;
    using iCore::MsgThread;
    using iCore::MsgTimer;
    using Utils::HostInf;
    using namespace iNet;
    using boost::shared_ptr;

	// TcpSocket test, PC side
    class TcpSrvTestSrvside :
        public MsgObject,
        public Utils::IVirtualDestroy,
        public iNet::ITcpServerToUser,
        boost::noncopyable
	{

        static const char* CTestName() { return "TcpSrvTest"; }

        class TcpSocketTest : 
            boost::noncopyable,
            public ITcpSocketToUser,
            public virtual SafeRefServer
        {
            shared_ptr<ITcpSocket> m_socket;
            boost::scoped_ptr<DataStream> m_data;
            ITestInfra &m_infra;
            int m_indx;

            void Log(const std::string &s)
            {
                std::stringstream ss;
                ss << TcpSrvTestSrvside::CTestName() << " #" << m_indx << ": " << s;
                m_infra.PrintToLog(ss.str());
            }

        // ITcpSocketToUser impl
        private:

            void Connected(SocketId id)
            {
                ESS_HALT("Already connected");
            }

            void ReceiveData(boost::shared_ptr<SocketData> data)
            {
                if (m_data.get() == 0) return;

                // put
                m_data->Put( PcUtils::FromQByteArr(data->getData()) );
            }

            void Disconnected(boost::shared_ptr<SocketError> error)
            {
                Log("Disconnected");
            }

            void SocketErrorOccur(boost::shared_ptr<SocketError> error)
            {
                ESS_HALT( Utils::QStringToString(error->getErrorString()) );
            }

        public:

            TcpSocketTest(int indx, shared_ptr<ITcpSocket> socket, 
                TcpSrvTestSrvsideProfile &profile, ITestInfra &infra) 
                : m_socket(socket), m_infra(infra), m_indx(indx)
            {
                m_data.reset( profile.CreateDataStream() );

                m_socket->LinkUserToSocket(this);
            }

            void Process()
            {
                if (m_data.get() == 0) return;

                // try send
                {
                    std::vector<byte> buff;
                    if (m_data->Peek(buff))
                    {
                        // dump
                        // Log( CommUtils::DumpVector(buff, 8) );

                        m_socket->SendData(PcUtils::ToQByteArr(buff));
                    }
                }

                // check completed
                if (m_data->RunCompleted())
                {
                    Log(m_data->StateAsString());

                    // destroy data stream
                    m_data.reset(0);                
                }                
            }
        };

        TcpSrvTestSrvsideProfile m_profile;
        ITestInfra &m_infra;
        iNet::MsgTcpServer m_srvSock;
        Utils::ManagedList<TcpSocketTest> m_socks;
        MsgTimer m_timer;


        void Log(const std::string &s)
        {
            m_infra.PrintToLog(CTestName() + std::string(": ") + s);
        }

        void OnTimer(MsgTimer *pT)
        {
            for(int i = 0; i < m_socks.Size(); ++i)
            {
                m_socks[i]->Process();
            }
        }

    // ITcpServerToUser impl
    private:

        void NewConnection(SocketId id, shared_ptr<ITcpSocket> socket)
        {
            Log("New connection!");

            m_socks.Add( new TcpSocketTest(m_socks.Size(), socket, m_profile, m_infra) );
        }
    public:

        TcpSrvTestSrvside(ITestInfra &infra, MsgThread &thread, const TcpSrvTestSrvsideProfile &profile) 
            : MsgObject(thread), 
            m_infra(infra),
            m_profile(profile), 
            m_srvSock(thread, this),
            m_timer(this, &TcpSrvTestSrvside::OnTimer)
        {
			boost::shared_ptr<iNet::SocketError> serverError = m_srvSock.Listen( HostInf(m_profile.ListenPort()) );
			
			ESS_ASSERT(serverError != 0 && serverError->getErrorString().toStdString())

            m_timer.Start(25, true);

            {
                std::stringstream ss;
                ss << "Listen on " << m_profile.ListenPort() << " port" ;
                Log(ss.str());
            }
        }

	};	
	
}  // namespace SockTest


#endif
