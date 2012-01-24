#include "stdafx.h"
#include "DirectCall.h"
#include "iRtpUtils/WriteToFile.h"
#include "Dss1ToSip/NObjDss1Interface.h"

namespace TestRealEcho
{

    DirectCall::DirectCall(
        iCore::MsgThread& thread, 
        IDirectCallToOwner& callBack, 
        iLogW::LogSession& log,             
        iRtp::RtpPcInfra& infra, 
		int channel,
        Utils::SafeRef<iCmpExt::ICmpChannelCreator> chCreator,
        const QString &name, 
		iRtpUtils::IRtpTestFactory& srcFactory,
        iRtpUtils::IRtpTestFactory& recFactory, 
		const iMedia::Codec &codec) :         
        m_codec(codec),
		m_callBack(callBack)
        
		//m_ctrl(dss1Interface->BChannel(channel, name.toStdString(), *this))
    {        
        iRtp::RtpParams prof; // use default?

		m_rtp.reset(new iRtp::RtpPcSession(thread, 
			log, 
			prof, 
			infra.Infra(), 
			*this, 
			m_ctrl->ChannelRtpAddr()));

        ESS_ASSERT(codec.getPayloadType() < iRtp::CMaxPayloadType_Unused );
        
        m_rtp->setPayload( iRtp::RtpPayload(codec.getPayloadType()) );

        m_dataReceiver.reset( recFactory.CreateTest(thread, m_rtp.get()) ); // can throw
        m_dataSrc.reset( srcFactory.CreateTest(thread, m_rtp.get()) );

        chCreator->CreateCmpChannel(
            &m_ctrl, channel,
            m_selfRefHost.Create<iCmpExt::ICmpChannelEvent&>(*this), 
            log.LogCreatorWeakRef()
            );

        // Start when cmp channel be created.
    }


	// ------------------------------------------------------------------------------------

	DirectCall::~DirectCall()
	{
	}

    // ------------------------------------------------------------------------------------

    void DirectCall::RxData( iRtp::SharedBdirBuff buff, iRtp::RtpHeaderForUser header )
    {
        m_dataReceiver->ReceiveData(buff, header);
    }

    // ------------------------------------------------------------------------------------

    void DirectCall::RxEvent( iRtp::RtpEvent ev, dword timestamp )
    {
        // ignore
    }

    // ------------------------------------------------------------------------------------

    void DirectCall::RtpErrorInd( iRtp::RtpError er )
    {
        m_callBack.ErrorInd(er.Desc().c_str());
    }

    // ------------------------------------------------------------------------------------

    void DirectCall::NewSsrcRegistred( dword newSsrc )
    {

    }

    // ------------------------------------------------------------------------------------

    void DirectCall::ChannelCreated(iCmpExt::ICmpChannel *cmpChannel, iCmpExt::CmpChannelId id)
    {
        ESS_ASSERT(&m_ctrl == id);
        ESS_ASSERT(m_ctrl == 0);

        m_ctrl.reset(cmpChannel);

        // Start()
        m_ctrl->StartRecv();        
        m_dataSrc->StartSend(m_codec);
        m_dataReceiver->StartReceive(m_codec);
        m_ctrl->StartSend( m_rtp->getLocalRtpAddr() );
    }

    // ------------------------------------------------------------------------------------

    // ICmpChannel *channel can be null
    void DirectCall::ChannelDeleted(iCmpExt::CmpChannelId id, const std::string &desc)
    {
        ESS_ASSERT(&m_ctrl == id);
        m_ctrl.reset();

        m_callBack.ErrorInd(desc.c_str());
    }

    // ------------------------------------------------------------------------------------

    void DirectCall::DtmfEvent(const iCmpExt::ICmpChannel *cmpChannel, const std::string &event)
    {
        ESS_ASSERT(m_ctrl.get() == cmpChannel);
    }
	
	
} // namespace TestRealEcho
