#include "stdafx.h"
#include "iLog/LogWrapperLibrary.h"
#include "RtpPcSession.h"

namespace iRtp
{

    //обертка для передачи TxData через сообщение
    struct RtpPcSession::TxDataParam 
    {
        TxDataParam(bool marker, dword timestamp,  QByteArray data)
            : m_marker(marker),
            m_timestamp(timestamp),
            m_data(data)
        {}

        bool m_marker;
        dword m_timestamp;
        QByteArray m_data;
    };

    //---------------------------------------------------------------------

    //обертка для передачи TxEvent через сообщение
    struct RtpPcSession::TxEventParam 
    {
        TxEventParam(dword timestamp, RtpEvent ev)
            : m_timestamp(timestamp),
            m_ev(ev)

        {}

        dword m_timestamp;
        RtpEvent m_ev;            
    };

    //---------------------------------------------------------------------

    //обертка для передачи RxData через сообщение
    struct RtpPcSession::RxDataParam
    {
        RxDataParam(SharedBdirBuff buff, const RtpHeaderForUser& header)
            : Buff(buff),
            Header(header)            
        {}
        
        SharedBdirBuff Buff;
        RtpHeaderForUser Header;
    };

    //---------------------------------------------------------------------

    //обертка для передачи RxEvent через сообщение
    struct RtpPcSession::RxEventParam
    {
        RxEventParam(RtpEvent ev, dword timestamp)
            : m_ev(ev),
            m_timestamp(timestamp)
        {}

        RtpEvent m_ev; 
        dword m_timestamp;
    };

    //---------------------------------------------------------------------

    void RtpPcSession::TxData( bool marker, dword timestamp, QByteArray data )
    {
        ESS_ASSERT(m_dstAddrIsSet);
        //todo Здесь должен быть виден размер буфера отдачи. Ассертить здесь data.size()!!!

        if (m_log->LogActive(m_infoTag) && m_prof.TraceFull)
		{
			*m_log << m_infoTag << "TxData with timestamp = " << timestamp << iLogW::EndRecord;
		}

        TxDataParam param(marker, timestamp, data);
        
        //OnTxData(param);
        PutMsg(this, &TMy::OnTxData, param );
    }

    //---------------------------------------------------------------------

    void RtpPcSession::TxEvent( dword timestamp, RtpEvent ev )
    {
        ESS_ASSERT(m_dstAddrIsSet);

        if (m_log->LogActive(m_infoTag))
		{
            *m_log << m_infoTag << "TxEvent" << iLogW::EndRecord;;
		}

        TxEventParam param(timestamp, ev);
        PutMsg(this, &TMy::OnTxEvent, param );
    }

    //---------------------------------------------------------------------

    void RtpPcSession::setPayload( RtpPayload payloadType )
    {
        ESS_ASSERT (payloadType != iRtp::CEmptyPayloadType);
        if (m_log->LogActive(m_infoTag))
        {
            *m_log << m_infoTag << "Set payload = " << payloadType  << iLogW::EndRecord; // todo payload as string
        }

        m_coreSession.setPayload( payloadType );
    }

    //---------------------------------------------------------------------

    void RtpPcSession::OnTxData( TxDataParam& param )
    {
        BidirBuffer* pBuff = m_infra.CreateBuff(0);
        pBuff->SetDataOffset(pBuff->getBlockSize());
       
        pBuff->PushFront(param.m_data.data(), param.m_data.size());

        m_coreSession.TxData( param.m_marker, param.m_timestamp, pBuff);
    }

    //---------------------------------------------------------------------

    void RtpPcSession::OnTxEvent( TxEventParam& param )
    {
        m_coreSession.TxEvent(param.m_timestamp, param.m_ev);
    }
    //---------------------------------------------------------------------

    void RtpPcSession::RxData( BidirBuffer* pBuff, RtpHeaderForUser header)
    {        
		if (m_log->LogActive(m_infoTag) && m_prof.TraceFull)
		{
			*m_log << m_infoTag << "RxData with timestamp =" << header.Timestamp  << iLogW::EndRecord;
        }

        SharedBdirBuff buff(pBuff);   
        RxDataParam param(buff, header);
        PutMsg(this, &TMy::OnRxData, param);
    }

    //---------------------------------------------------------------------

    void RtpPcSession::RxEvent( RtpEvent ev, dword timestamp )
    {
		if (m_log->LogActive(m_infoTag) && m_prof.TraceFull)
		{
			*m_log << m_infoTag << "RxEvent with timestamp = " << timestamp << iLogW::EndRecord;
		}

        RxEventParam param(ev, timestamp);
        PutMsg(this, &TMy::OnRxEvent, param);
    }

    //---------------------------------------------------------------------
    
    void RtpPcSession::RtpErrorInd( RtpError err )
    {
        // todo use trace options
        //if (m_log.getTraceOn() )
        //    LogWriter(m_log, m_tags.m_msgFromCore) << "Error indicaton "/*<< err.toString()*/;

        PutMsg(this, &TMy::OnRtpErrorInd, err);
    }

    //---------------------------------------------------------------------

    void RtpPcSession::OnRxData( RxDataParam& param )
    {
		if(getLocalRtpAddr().Port() == 8060)
			getLocalRtpAddr().Port();

        m_user.RxData(param.Buff, param.Header);
    }

    //---------------------------------------------------------------------
   
    void RtpPcSession::OnRtpErrorInd( RtpError err )
    {
        m_user.RtpErrorInd(err);
    }

    //---------------------------------------------------------------------

    RtpPcSession::RtpPcSession(iCore::MsgThread& thread,
		iLogW::ILogSessionCreator &logCreator,
        const RtpParams& prof, 
        IRtpInfra &infra, 
        IRtpPcToUser& user, 
        const HostInf &dstAddr) : 
        MsgObject(thread),
        m_prof(prof),
		m_log(logCreator.CreateSession("RtpPcSession", prof.PcTraceInd)),
		m_infoTag(m_log->RegisterRecordKind(L"Info") ),
        m_coreSession(m_prof, *m_log, infra, *this, dstAddr),
        m_user(user),
        m_infra(infra)
    {

        m_dstAddrIsSet = !dstAddr.Empty();
		if (m_log->LogActive(m_infoTag))
		{
			*m_log << m_infoTag << "Created." << iLogW::EndRecord;
		}

    }

	RtpPcSession::~RtpPcSession()
	{
		if (m_log->LogActive(m_infoTag))
		{
			*m_log << m_infoTag << "Deleted." << iLogW::EndRecord;
		}
	}

    void RtpPcSession::setDestAddr(const HostInf &dstAddr)
    {
        ESS_ASSERT(!m_dstAddrIsSet && "Try reset dstAddress. Not Supported.");
        m_coreSession.setDestAddr(dstAddr);
        m_dstAddrIsSet = true;
    }

    //---------------------------------------------------------------------


    void RtpPcSession::OnRxEvent( RxEventParam& param )
    {
        m_user.RxEvent(param.m_ev, param.m_timestamp);
    }

    //-------------------------------------------------------------------------------------

    void RtpPcSession::OnNewSsrcRegistred( dword newSsrc )
    {
        m_user.NewSsrcRegistred(newSsrc);
    }

    //---------------------------------------------------------------------

    void RtpPcSession::NewSsrcRegistred( dword newSsrc )
    {
        PutMsg(this, &TMy::OnNewSsrcRegistred, newSsrc);
    }

} // namespace iRtp

