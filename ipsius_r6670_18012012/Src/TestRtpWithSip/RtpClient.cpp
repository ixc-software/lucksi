#include "stdafx.h"
#include "Utils/ErrorsSubsystem.h"
#include "Sip/ISipToRtp.h"
#include "iRtp/RtpPcSession.h"
#include "IRtpTest.h"
#include "RtpClient.h"

namespace TestRtpWithSip
{
    RtpClient::RtpClient(iCore::MsgThread &thread,
        Utils::IVirtualDestroyOwner &owner,
        const Sip::Codec &codec,
        iRtp::IRtpInfra &rtpInfra,
        const iRtp::RtpParams &rtpProfile, 
        iLogW::ILogSessionCreator &logCreator,
        boost::shared_ptr<IRtpTestFactory> scriptFactory) :
        m_owner(owner),
		m_rtpInfra(rtpInfra),
        m_rtpSession(new iRtp::RtpPcSession(thread, "", rtpProfile, m_rtpInfra, *this)),
        m_script(scriptFactory->CreateTest(thread, m_rtpSession.get())),
        m_log(logCreator.CreateLogSesion("RtpClient", rtpProfile.PcTraceInd))
	{
        m_codecs.push_back(codec);
        m_rtpSession->setPayload(iRtp::RtpPayload(codec.getPayloadType()));
    }

	//----------------------------------------------------------------------------------------

    RtpClient::~RtpClient()
    {
        if(!m_sip.IsEmpty()) m_sip.Clear()->Release(this);
    }

	//----------------------------------------------------------------------------------------
	// impl ISipToRtp     

    void RtpClient::SetupReq(Utils::SafeRef<Sip::IRtpToSip> id)
	{
		ESS_ASSERT(m_sip.IsEmpty());

		m_sip = id;
	}

    //----------------------------------------------------------------------------------------

    const Utils::HostInf &RtpClient::LocalRtp() const
    {
        return m_rtpSession->getLocalRtpAddr();
    }

    //----------------------------------------------------------------------------------------

    const Utils::HostInf &RtpClient::LocalRtcp() const
    {
        return m_rtpSession->getLocalRtpAddr();
    }

    //----------------------------------------------------------------------------------------

    const Sip::CodecsList &RtpClient::LocalCodecs() const
    {
        return m_codecs;
    }

	//----------------------------------------------------------------------------------------

	void RtpClient::Start(const Sip::IRtpToSip *id, 
		boost::shared_ptr<Sip::StartRtpRequest> startReq)
	{
		ESS_ASSERT(m_sip.IsEqualIntf(id));

        m_log->Add(L"StartReq", iLogW::LogRecordTag() );
		ESS_ASSERT(m_rtpSession.get() != 0);

		m_rtpSession->setDestAddr(startReq->remoteRtp());

		if(startReq->isSendAllowed())
			m_script->StartSend(startReq->sendCodecs().front());
		if(startReq->isRecAllowed())
			m_script->StartReceive(startReq->recCodecs().front());
	}

    //----------------------------------------------------------------------------------------

	void RtpClient::Release(const Sip::IRtpToSip *id)
	{
		ESS_ASSERT(m_sip.IsEqualIntf(id));

		m_sip.Clear();
		m_owner.Delete(this);
	}

	//----------------------------------------------------------------------------------------
	// IRtpPñToUser

    void RtpClient::RxData( iRtp::SharedBdirBuff buff, iRtp::RtpHeaderForUser header)
    {
		if(m_sip.IsEmpty())
			return;

        if (m_codecs.front().getPayloadType()==header.Payload)
        {
			m_script->ReceiveData(buff, header);
        }
		else
        {
			m_sip.Clear()->Release(this, 415);
        }
    }

	//----------------------------------------------------------------------------------------

	void RtpClient::RxEvent(iRtp::RtpEvent ev, Platform::dword timestamp)
	{
		ESS_HALT("NotImplemented");
	}

	//----------------------------------------------------------------------------------------

	void RtpClient::RtpErrorInd(iRtp::RtpBaseError error)
	{
		ESS_HALT("NotImplemented");
	}
} // namespace TestRtpWithSip
