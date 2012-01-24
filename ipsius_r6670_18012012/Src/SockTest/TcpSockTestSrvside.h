#ifndef __TCPSOCKTESTSRVSIDE__
#define __TCPSOCKTESTSRVSIDE__

#include "Utils/IVirtualDestroy.h"
#include "inet/MsgTcpSocket.h"
#include "iNet/SocketError.h"
#include "iCore/MsgObject.h"
#include "iCore/MsgTimer.h"
#include "Utils/TimerTicks.h"
#include "Utils/QtHelpers.h"
#include "ProfileSetup.h"
#include "DataStream.h"
#include "PcUtils.h"
#include "ITestInfra.h"

namespace SockTest
{
    using iCore::MsgObject;
    using iCore::MsgThread;
    using iCore::MsgTimer;
    using Utils::HostInf;
    using namespace iNet;
    using boost::shared_ptr;

    class TcpSockTestSrvside :
        public MsgObject,
        public Utils::IVirtualDestroy,
        public ITcpSocketToUser,
        boost::noncopyable
    {
        ITestInfra &m_infra;
        TcpSockTestSrvsideProfile m_profile;
        iNet::MsgTcpSocket m_sock;
        boost::scoped_ptr<DataStream> m_data;
        iCore::MsgTimer m_pollTimer;
        int m_connectTry;

        void DoConnect()
        {
            QString addr = QString(m_profile.RemoteHost().c_str());
            HostInf host(QHostAddress(addr), m_profile.RemotePort());
            QString debugInfo = host.getAddress().toString();
            m_sock.ConnectToHost(host);

        }

        void OnTimer(MsgTimer *pT)
        {
            if (m_data.get() == 0) return;

            // try send
            {
                std::vector<byte> buff;
                if (m_data->Peek(buff))
                {
                    // dump
                    // Log( CommUtils::DumpVector(buff, 8) );

                    m_sock.SendData(PcUtils::ToQByteArr(buff));
                }
            }

            // check completed
            if (m_data->RunCompleted())
            {
                Log(m_data->StateAsString());

                // destroy data stream
                m_data.reset(0);

                m_sock.DisconnectFromHost();
            }                
        }

        void Log(const std::string &s)
        {
            std::stringstream ss;
            ss << "TcpSockTest" << " #" << m_connectTry << ": " << s;
            m_infra.PrintToLog(ss.str());
        }

    // ITcpSocketToUser impl
    private:

        void Connected(SocketId id)
        {
            ESS_ASSERT(m_data.get() == 0);

            m_connectTry++;
            Log("Connected");
            m_data.reset( m_profile.CreateDataStream() );
        }

        void ReceiveData(boost::shared_ptr<SocketData> data)
        {
            if (m_data.get() == 0) return;

            // put
            m_data->Put( PcUtils::FromQByteArr(data->getData()) );
        }

        void Disconnected(boost::shared_ptr<SocketError> error)
        {
            if (m_data.get() != 0) 
            {
                Log("Disconnected");
                m_data.reset(0);
            }

            DoConnect();                
        }

        void SocketErrorOccur(boost::shared_ptr<SocketError> error)
        {
            //Log(Utils::QStringToString(error->getErrorString()));

            // DoConnect();  // retry - ?

            // ESS_HALT( Utils::QStringToString(error->getErrorString()) );
        }

    public:

        TcpSockTestSrvside(ITestInfra &infra, MsgThread &thread, const TcpSockTestSrvsideProfile &profile) 
            : MsgObject(thread),
            m_infra(infra),
            m_profile(profile),
            m_sock(thread, this),
            m_pollTimer(this, &TcpSockTestSrvside::OnTimer),
            m_connectTry(0)
        {
            m_pollTimer.Start(50, true);

            DoConnect();
        }

    };

}  // namespace SockTest


#endif
