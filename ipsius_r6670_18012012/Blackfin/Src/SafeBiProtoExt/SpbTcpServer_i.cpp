#include "stdafx.h"
#include "Lw/TcpSocket.h"
#include "Lw/TcpServerSocket.h"
#include "SafeBiProto/SbpTcpTransportFactory.h"
#include "SpbTcpServer.h"

namespace SBProtoExt
{
   
    class SbpTcpServer::Impl : boost::noncopyable
    {
    public:
        Impl(iCore::MsgThread &thread,
            const SbpTcpServer &wrapper,
            SbpTcpServer::Profile &profile) : 
            m_thread(thread),
            m_wrapper(wrapper),
            m_user(*profile.m_user),
            m_profile(profile.m_connectionProfile),
            m_logSettings(profile.m_logSettings)
        {	
			if (m_logSettings == 0) m_logSettings.reset(new SbpLogSettings());
			m_profile.m_sbpActivationReq = true;
            m_server.Listen(profile.m_tcpPort);
        }
        void Process()
        {
            Lw::TcpSocket *socket = m_server.Accept();
            if (socket == 0) return;
	
            SbpConnProfile profile(m_profile);
			profile.SetTransport(SBProto::SbpTcpTransportFactory::CreateTransport(socket));
            
            boost::shared_ptr<SbpConnection> 
                connection(new SbpConnection(m_thread, profile, m_logSettings));
            m_user.NewConnection(&m_wrapper, connection);
        }
        
		SbpLogSettings &LogSettings()
		{
			return *m_logSettings;
		}
		Utils::HostInf LocalHostInf() const
		{
			return m_server.LocalHostInf();
		}
    private:
        iCore::MsgThread &m_thread;
        const SbpTcpServer &m_wrapper;
        ISbpTcpServerEvents &m_user;
        SbpConnProfile m_profile;
		boost::shared_ptr<SbpLogSettings> m_logSettings;        
        Lw::TcpServerSocket m_server;
    };

    //---------------------------------------------------------------------------

    SbpTcpServer::SbpTcpServer(iCore::MsgThread &thread, Profile &profile) :
        m_checkProfile(profile),
        m_impl(new Impl(thread, *this, profile))
    {}

    //---------------------------------------------------------------------------

    SbpTcpServer::~SbpTcpServer(){}

    //---------------------------------------------------------------------------

    void SbpTcpServer::Process()
    {
        m_impl->Process();

    }

    //---------------------------------------------------------------------------

	SbpLogSettings &SbpTcpServer::LogSettings()
	{
		return m_impl->LogSettings();
	}
	

	//---------------------------------------------------------------------------

	Utils::HostInf SbpTcpServer::LocalHostInf() const
	{
		return m_impl->LocalHostInf();
	}

	
    
} // namespace SBProto

