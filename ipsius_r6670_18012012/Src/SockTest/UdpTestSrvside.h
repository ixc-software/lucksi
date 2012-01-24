#ifndef __UDPTESTSRVSIDE__
#define __UDPTESTSRVSIDE__

#include "Utils/IVirtualDestroy.h"
#include "iNet/MsgUdpSocket.h"
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
    using iCore::MsgThread;
    using iCore::MsgObject;
    using iNet::SocketData;
    using iNet::SocketError;
    using Platform::byte;

    // -----------------------------------------------------------

    // UDP test, server (PC) side
    class UdpTestSrvside : 
        public MsgObject, 
        public Utils::IVirtualDestroy,
        public iNet::IUdpSocketToUser,
        boost::noncopyable
	{
        ITestInfra &m_infra;
        UdpTestSrvsideProfile m_profile;
        iNet::MsgUdpSocket m_sock;
        boost::scoped_ptr<DataStream> m_stream;
        iCore::MsgTimer m_pollTimer;
        Utils::HostInf m_dst;

        enum
        {
            CPollTime = 10,
        };

        void Log(const std::string &s)
        {
            m_infra.PrintToLog("UdpSrvTest: " + s);
        }

        void OnTimer(iCore::MsgTimer *pT)
        {
            if (m_stream.get() == 0) return;  // test not active

            // try send
            {
                std::vector<byte> buff;
                if (m_stream->Peek(buff))
                {
                    // dump
                    // Log( CommUtils::DumpVector(buff, 8) );

                    boost::shared_ptr<SocketData> data(
                        new SocketData(m_dst, PcUtils::ToQByteArr(buff))
                        );
                    
                    m_sock.SendData(data);
                }
            }

            // check completed
            if (m_stream->RunCompleted())
            {
                Log(m_stream->StateAsString());

                // destroy data stream
                m_stream.reset(0);                
            }
        }

    // iNet::IUdpSocketToUser impl
    private:

        void ReceiveData(boost::shared_ptr<SocketData> data)
        {
            if (m_stream.get() == 0)  // test begin
            {
                m_stream.reset( m_profile.CreateDataStream() );
                m_dst = data->getHostinf();
            }

            // dump
            // Log( PcUtils::DumpQByteArray(data->getData()) );

            // put
            m_stream->Put( PcUtils::FromQByteArr(data->getData()) );
        }

        
        void SocketErrorOccur(boost::shared_ptr<SocketError> error)
        {
            ESS_HALT( Utils::QStringToString(error->getErrorString()) );
        }

    public:

        UdpTestSrvside(ITestInfra &infra, MsgThread &thread, const UdpTestSrvsideProfile &profile) 
            : MsgObject(thread), 
            m_infra(infra),
            m_profile(profile), 
            m_sock(thread, this),
            m_pollTimer(this, &UdpTestSrvside::OnTimer)
        {
            // Utils::HostInf host(QHostAddress("0.0.0.0"), m_profile.OwnPort());
            Utils::HostInf host(m_profile.OwnPort());
            ESS_ASSERT( m_sock.Bind(host) );

            m_pollTimer.Start(CPollTime, true);

            {
                std::stringstream ss;
                ss << "Listen on " << profile.OwnPort() << " port" ;
                Log(ss.str());
            }
            
        }
	};	
	
}  // namespace SockTest


#endif
