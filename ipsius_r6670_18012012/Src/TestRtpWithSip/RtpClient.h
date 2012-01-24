#ifndef RTPCLIENT_H
#define RTPCLIENT_H

#include "Utils/IBasicInterface.h"
#include "Utils/SafeRef.h"
#include "Utils/IVirtualDestroy.h"
#include "iLog/iLogSessionCreator.h"
#include "iRtp/IRtpPCToUser.h"
#include "iRtp/IRtpInfra.h"
#include "Sip/ISipToRtp.h"
#include "Sip/Codec.h"
#include "IRtpTestFactory.h"

namespace iCore{    class MsgThread;    };
namespace iRtp {    
    class RtpPcSession; 
    struct RtpParams;
    class RtpInfraParams;
};
namespace Sip {    class TelephoneSignal;    };


namespace TestRtpWithSip
{
	class IRtpTest;

    class RtpClient : boost::noncopyable,
        public virtual Utils::SafeRefServer,
		public Utils::IVirtualDestroy,
        public Sip::ISipToRtp,
        public iRtp::IRtpPcToUser
    {
	public:       
		RtpClient(iCore::MsgThread &thread,
			Utils::IVirtualDestroyOwner &owner,
            const Sip::Codec &codec,
            iRtp::IRtpInfra &rtpInfra,
            const iRtp::RtpParams &rtpProfile, 
            iLogW::ILogSessionCreator &,
            boost::shared_ptr<IRtpTestFactory> scriptFactory);
        ~RtpClient();
	// impl ISipToRtp 
	private:        
		void SetupReq(Utils::SafeRef<Sip::IRtpToSip> id);
        const Utils::HostInf &LocalRtp() const;
        const Utils::HostInf &LocalRtcp() const;
        const Sip::CodecsList &LocalCodecs() const;
		void Start(const Sip::IRtpToSip *, 
			boost::shared_ptr<Sip::StartRtpRequest> par);
		void Release(const Sip::IRtpToSip *);
	// impl IRtpPÒToUser
	private:
		void RxData(iRtp::SharedBdirBuff buff, iRtp::RtpHeaderForUser header);
		void RxEvent(iRtp::RtpEvent ev, Platform::dword timestamp);
		void RtpErrorInd(iRtp::RtpBaseError error);
        void NewSsrcRegistred(Platform::dword newSsrc) {/*?*/}
    
	// fields:
    private:
		Utils::IVirtualDestroyOwner &m_owner;
        Sip::CodecsList m_codecs;
        iRtp::IRtpInfra &m_rtpInfra;
        boost::scoped_ptr<iRtp::RtpPcSession> m_rtpSession;
		boost::scoped_ptr<iLogW::LogSession>  m_log;
		boost::scoped_ptr<IRtpTest> m_script;
		Utils::SafeRef<Sip::IRtpToSip> m_sip;        
    };
}; // namespace TestRtpWithSip

#endif
