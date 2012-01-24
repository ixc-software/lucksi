#include "stdafx.h"

#include "iNet/MsgTcpServer.h"
#include "iNet/SocketError.h"
#include "SafeBiProto/SbpTcpTransportFactory.h"
#include "SpbTcpServer.h"

namespace SBProtoExt
{
    class SbpTcpServer::Impl : boost::noncopyable,
        public virtual Utils::SafeRefServer,
        public iNet::ITcpServerToUser
    {
    public:
        Impl(iCore::MsgThread &thread,
            const SbpTcpServer &wrapper,
            SbpTcpServer::Profile &profile) : 
            m_wrapper(wrapper),
            m_user(*profile.m_user),
            m_profile(profile.m_connectionProfile),
            m_logSettings(profile.m_logSettings),
            m_server(thread, this)
        {	
			ESS_ASSERT(m_logSettings != 0);
			m_profile.m_sbpActivationReq = true;
			boost::shared_ptr<iNet::SocketError> serverError = 
				m_server.Listen(Utils::HostInf(profile.m_tcpPort));
			if(serverError != 0)
			{
				m_user.Error(&m_wrapper, serverError->getErrorString().toStdString());
			}
        }
		SbpLogSettings &ConnectionLogSettings()
		{
			return *m_logSettings;
		}
		Utils::HostInf LocalHostInf() const
		{
			return m_server.LocalHostInf();
		}

    // ITcpServerToUser
    private:
        void NewConnection(iNet::SocketId id, boost::shared_ptr<iNet::ITcpSocket> socket)
        {
            SbpConnProfile profile(m_profile);
			profile.SetTransport(SBProto::SbpTcpTransportFactory::CreateTransport(socket));
            boost::shared_ptr<SbpConnection> 
                connection(new SbpConnection(m_server.getMsgThread(), profile, m_logSettings));
            m_user.NewConnection(&m_wrapper, connection);
        }
	private:
        const SbpTcpServer &m_wrapper;
        ISbpTcpServerEvents &m_user;
		SbpConnProfile m_profile;
		boost::shared_ptr<SbpLogSettings> m_logSettings;
        iNet::MsgTcpServer m_server;
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
        // nothing        
    }

	//---------------------------------------------------------------------------

	SbpLogSettings &SbpTcpServer::LogSettings()
	{
		return m_impl->ConnectionLogSettings();
	}

	//---------------------------------------------------------------------------

	Utils::HostInf SbpTcpServer::LocalHostInf() const
	{
		return m_impl->LocalHostInf();
	}

} // namespace SBProto


