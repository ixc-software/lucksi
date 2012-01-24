#ifndef _SBP_TCP_SERVER_H_
#define _SBP_TCP_SERVER_H_

#include "iLog/iLogSessionCreator.h"
#include "Utils/HostInf.h"
#include "SbpConnection.h"
#include "ISbpTcpServerEvents.h"


namespace iCore {	class MsgThread;	};

namespace SBProtoExt
{
    class SbpTcpServer : boost::noncopyable
    {
        typedef SbpTcpServer T;
    public:
        struct Profile
        {
            Profile() 
            {
                m_user = 0; 
                m_tcpPort = 0;
				m_logSettings.reset(new SbpLogSettings());
            }
            bool IsValid() const
            {
				// tcp server create transport by oneself
                return m_logSettings != 0 && m_user != 0;
            }
            int m_tcpPort;
            ISbpTcpServerEvents *m_user;
			boost::shared_ptr<SbpLogSettings> m_logSettings;
            SbpConnProfile m_connectionProfile;
        };
        SbpTcpServer(iCore::MsgThread &thread, Profile &profile);
        ~SbpTcpServer();
        void Process();
		SbpLogSettings &LogSettings();
        Utils::HostInf LocalHostInf() const; 
	private:
        struct CheckProfile
        {
            CheckProfile(Profile &profile)
            {
                ESS_ASSERT(profile.IsValid());
            }
        };
        CheckProfile m_checkProfile;
        class Impl;
        boost::scoped_ptr<Impl> m_impl;
    };
} // namespace SBProto

#endif  
