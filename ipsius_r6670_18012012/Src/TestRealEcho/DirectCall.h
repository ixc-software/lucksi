#ifndef DIRECTCALL_H
#define DIRECTCALL_H

#include "stdafx.h"
#include "iCmpExt/ICmpChannel.h"
#include "iRtp/RtpPcSession.h"
#include "iRtpUtils/IRtpTest.h"
#include "iRtpUtils/IRtpTestFactory.h"
#include "Utils/WeakRef.h"

namespace Dss1ToSip	{ class NObjDss1Interface; }

namespace TestRealEcho
{
    class IDirectCallToOwner
    {
    public:
        virtual void ErrorInd(QString errInfo) = 0;
    };

    using Platform::dword;
    using boost::shared_ptr;
    using boost::scoped_ptr;

    // вызов в обход Sip&Dss
	class DirectCall :
		virtual public Utils::SafeRefServer,
        public iRtp::IRtpPcToUser,
        public iCmpExt::ICmpChannelEvent
    {
		Utils::WeakRefHost m_selfRefHost;  
        const iMedia::Codec m_codec;

        scoped_ptr<iRtp::RtpPcSession> m_rtp;        

        // функции чтения-записи
        scoped_ptr<iRtpUtils::IRtpTest> m_dataReceiver;
        scoped_ptr<iRtpUtils::IRtpTest> m_dataSrc;
        IDirectCallToOwner& m_callBack;       
        boost::shared_ptr<iCmpExt::ICmpChannel> m_ctrl;

    // iRtp::IRtpPcToUser
    private:
        void RxData(iRtp::SharedBdirBuff buff, iRtp::RtpHeaderForUser header);
        void RxEvent(iRtp::RtpEvent ev, dword timestamp);
        void RtpErrorInd(iRtp::RtpError er);
        void NewSsrcRegistred(dword newSsrc);

    // iCmpExt::ICmpChannelEvent
    private:
        void ChannelCreated(iCmpExt::ICmpChannel *cmpChannel, iCmpExt::CmpChannelId id);        
        void ChannelDeleted(iCmpExt::CmpChannelId id, const std::string &desc);
        void DtmfEvent(const iCmpExt::ICmpChannel *cmpChannel, const std::string &event);

    public:
        DirectCall(iCore::MsgThread& thread, 
            IDirectCallToOwner& callBack,
            iLogW::LogSession& log,            
            iRtp::RtpPcInfra& infra, 
			int channel,
			Utils::SafeRef<iCmpExt::ICmpChannelCreator>,
            const QString &name,
            iRtpUtils::IRtpTestFactory& srcFactory,
            iRtpUtils::IRtpTestFactory& recFactory,
            const iMedia::Codec &codec
            );

        ~DirectCall();


    };
} // namespace TestRealEcho

#endif
