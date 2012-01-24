#include "stdafx.h"

#include "Platform/Platform.h"
#include "Utils/ErrorsSubsystem.h"
#include "Utils/IntToString.h"

#include "RtpInfra.h"
#include "RtpCoreSocket.h"

namespace iRtp
{
    class RtpInfra::Impl : boost::noncopyable
    {
        RtpInfraParams m_params;
        RtpInfra &m_infra;

        // stats 
        int m_totalAllocated, m_totalFreed;        

        std::vector<bool> m_allocMap;  // map of pair 

        int FindFreePair()
        {
            for(int i = 0; i < m_allocMap.size(); ++i)
            {
                if ( !m_allocMap.at(i) ) return i;
            }

            return -1;
        }

        std::string Info() const
        {
            std::ostringstream oss;

            oss << "total alloc " << m_totalAllocated 
                << "; total freed " << m_totalFreed
                << "; pool " << m_params.PortCount();

            return oss.str();
        }

    public:

        ESS_TYPEDEF(Error);

        Impl(const RtpInfraParams &params, RtpInfra &infra) :
            m_params(params), m_infra(infra)
        {
        	ESS_ASSERT(m_params.IsValid());
        	
		    m_allocMap.resize(m_params.PortCount() / 2, false);
        }

        SocketPair CreateSocketPair(ISocketToRtp& socketOwner, bool useRtcp) 
        {
            ESS_ASSERT( !useRtcp && "Not supported!" );                

            int indx = FindFreePair();

            if (indx < 0)
            {
                ESS_THROW_MSG(Error, "No free socks; " + Info());
            }

            int startPort = m_params.MinPort() + (indx * 2);
            ESS_ASSERT( (startPort >= m_params.MinPort()) && 
                        (startPort + 1 <= m_params.MaxPort()) );

            const std::string &localAddr = m_params.GetLocalHostAddress();

            SocketPair result;

            // create Rtp socket
            result.RtpSock.reset(
                new RtpCoreSocket( socketOwner, m_infra, HostInf(localAddr, startPort) )
                );

            if (useRtcp)
            {
                result.RtcpSock.reset(
                    new RtpCoreSocket( socketOwner, m_infra, HostInf(localAddr, startPort + 1) )
                    );
            }

            m_allocMap.at(indx) = true;  // capture

            return result;
        }

        void FreeSocketPort(int port)
        {
            int indx = (port - m_params.MinPort()) / 2;

            if ( (indx < 0) || (indx >= m_allocMap.size()) )
            {
                std::ostringstream oss;
                oss << "Bad free indx " << indx << "; " << Info();
                ESS_THROW_MSG(Error, oss.str());
            }

            // it's don't work for RTCP - !!
            if ( !m_allocMap.at(indx) )
            {
                std::ostringstream oss;
                oss << "Indx already free " << indx << "; " << Info();
                ESS_THROW_MSG(Error, oss.str());
            }

            m_allocMap.at(indx) = false;  // free
        }

    };

    // ---------------------------------------------------

    RtpInfra::RtpInfra(iCore::MsgThread &thread,
        const RtpInfraParams &params,
        Utils::IBidirBuffCreator &buffCreator) :
        m_buffCreator(buffCreator),
        m_rnd(Platform::GetSystemTickCount()),
        m_impl(new Impl(params, *this))
    {
        ESS_ASSERT(params.IsValid());    
    }

	// ---------------------------------------------------

    RtpInfra::~RtpInfra()
    {
        // nothing 
    }

    // ---------------------------------------------------

    SocketPair RtpInfra::CreateSocketPair(ISocketToRtp& socketOwner, bool useRtcp)
    {
        return m_impl->CreateSocketPair(socketOwner, useRtcp);
    }

    // ---------------------------------------------------

    void RtpInfra::FreeSocketPort(int port)
    {
        m_impl->FreeSocketPort(port);
    }

} //namespace iRtp

