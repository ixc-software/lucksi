#include "stdafx.h"
#include "Platform/Platform.h"
#include "Utils/ErrorsSubsystem.h"
#include "RtpInfra.h"
#include "RtpPcSocket.h"

namespace 
{
	const int CMinDefRtpPort = 8000;
	const int CMaxDefRtpPort = 9000;
	
	class PortAllocator : boost::noncopyable
	{
	public:
		PortAllocator(int minPort = 0, int maxPort = 0) 
		{
			m_minPort = (minPort == 0) ? CMinDefRtpPort : minPort;
			m_maxPort = (maxPort == 0) ? CMaxDefRtpPort : maxPort;
			m_currentPort = m_minPort;
			ESS_ASSERT((m_currentPort & 1) == 0);
		}

		int AllocPort() 
		{
            Platform::MutexLocker lock(m_mutex);
			int res = m_currentPort;
			m_currentPort += 2;  
			if (m_currentPort >= m_maxPort) m_currentPort = m_minPort;
			return res;
		}
	private:
		Platform::Mutex m_mutex;
		int m_minPort; 
		int m_maxPort;
		int m_currentPort;
	};

	PortAllocator GPortAllocator;
};


namespace iRtp
{
    class RtpInfra::Impl : boost::noncopyable
    {
    public:
        Impl(iCore::MsgThread &thread, 
            const RtpInfraParams &params,
            RtpInfra &infra) :
            m_thread(thread), m_checkParams(params), m_infra(infra),
			m_localAddress(params.GetLocalHostAddress()),
			m_thisAllocator(params.MinPort(), params.MaxPort()),
			m_allocator((params.MinPort() == 0) ? GPortAllocator : m_thisAllocator)
        {

		}

        SocketPair CreateSocketPair(ISocketToRtp& socketOwner, bool useRtcp) // can throw // override
        {           
            SocketPair result;

            RtpPcSocket *pRtpSock = new RtpPcSocket(m_thread, socketOwner, m_infra);
            result.RtpSock.reset(pRtpSock);

            RtpPcSocket *pRtcpSock = 0;
            if (useRtcp)
            {
                pRtcpSock = new RtpPcSocket(m_thread, socketOwner, m_infra);
                result.RtcpSock.reset(pRtcpSock);
            }
            
            int firstPort = m_allocator.AllocPort();
			int port = firstPort; 
			do 
			{
				HostInf rtpHost(m_localAddress, port);
				HostInf rtcpHost(m_localAddress, port + 1);
                if(pRtpSock->Bind(rtpHost) && 
                    (pRtcpSock == 0 || pRtcpSock->Bind(rtcpHost))) return result;
				port = m_allocator.AllocPort();
			} while (port != firstPort);

            ESS_THROW(NoFreePortPresent);
            return result;
        }
    private:
        iCore::MsgThread &m_thread;
		RtpInfraParams::Check m_checkParams;
        RtpInfra &m_infra;
		std::string m_localAddress;
		PortAllocator m_thisAllocator;
		PortAllocator &m_allocator;
    };

    // ---------------------------------------------------

    RtpInfra::RtpInfra(iCore::MsgThread &thread,
        const RtpInfraParams &params,
        Utils::IBidirBuffCreator &buffCreator) :
        m_buffCreator(buffCreator),
        m_rnd(Platform::GetSystemTickCount()),
        m_impl(new Impl(thread, params, *this))
    {
        ESS_ASSERT(params.IsValid());
    }

	// ---------------------------------------------------

    RtpInfra::~RtpInfra(){}

    // ---------------------------------------------------

    SocketPair RtpInfra::CreateSocketPair(ISocketToRtp& socketOwner, bool useRtcp)
    {
        return m_impl->CreateSocketPair(socketOwner, useRtcp);
    }

    void RtpInfra::FreeSocketPort( int port )
    {
        // nothing for PC
    }

} //namespace iRtp

