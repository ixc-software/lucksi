#ifndef _SBP_CONN_PROFILE_H_
#define _SBP_CONN_PROFILE_H_

#include "Utils/IBasicInterface.h"
#include "iLog/LogWrapper.h"
#include "SafeBiProto/SafeBiProto.h"
#include "SafeBiProto/ISbpTransport.h"
#include "ISbpConnection.h"

namespace SBProtoExt
{   

    class ISpbRawDataCallback : public Utils::IBasicInterface
    {
    public:
        virtual void OnSbpRawDataRecv(const void *pData, size_t size) = 0;
    };

	struct SbpConnProfile
    {
        SbpConnProfile() 
        {
			m_sbpActivationReq = false;
        }
        bool IsValid() const
        {
            return m_transport != 0;
        }
		void SetTransport(boost::shared_ptr<SBProto::ISbpTransport> transport) 
		{
			m_transport = transport;
		}
		boost::shared_ptr<SBProto::ISbpTransport> GetTransport() 
		{
			ESS_ASSERT(m_transport != 0);

			return m_transport;
		}

	public:
        Utils::SafeRef<ISbpConnectionEvents> m_user; 
        Utils::SafeRef<ISpbRawDataCallback> m_rawDataCallback;
        SBProto::SbpSettings m_sbpSettings;
		Utils::SafeRef<iLogW::ILogSessionCreator> m_logCreator;
		bool m_sbpActivationReq;

	private:
		boost::shared_ptr<SBProto::ISbpTransport> m_transport;
    };
} // namespace SBProto

#endif

