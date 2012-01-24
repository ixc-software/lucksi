#include "stdafx.h"

#include "Dss1ToSipCall.h"
#include "ISDN/IeConstants.h"
#include "IGateDss1.h"
#include "Dss1ToSipCause.h"
#include "NObjDss1ToSip.h"

#include "iSip/MediaToCmpAdapter.h"
#include "iSip/MediaLineList.h"
#include "iSip/SipUtils.h"
#include "iSip/Sdp/Sdp.h"

#include "Utils/AtomicTypes.h"

namespace Dss1ToSip 
{
    Dss1ToSipCall::Dss1ToSipCall(
		Registrator registrator,
		Calls::StatisticCalls &stat,
		iLogW::ILogSessionCreator &logCreator,
		Utils::SafeRef<iSip::IRemoteParticipant> callSip,
		IGateDss1 &gateDss1,
		const ISDN::DssPhoneNumber& calledNum, 
		const ISDN::DssPhoneNumber& callingNum) :
		m_registrator(registrator),
		m_deleteProcess(false),
		m_stat(stat.IncomingCall()),
        m_callSip(callSip),
		m_callRecord(m_callSip->CallRecord()),
		m_regRecord(m_callRecord->Record().Dst())

    {
        // Sip->Dss1 call

		m_registrator(this, true);

		InitLog(logCreator);

		m_media.reset(new iSip::MediaLineList(
			*m_log,
			boost::bind(&T::InitRemoteSdp, this, _1)));

		m_callDss1 = gateDss1.RunOutCall(this, calledNum, callingNum);

		if(m_log->LogActive(m_infoTag))
		{
			*m_log << m_infoTag << "Created.\n" 
				<< " Source address: " << callingNum.Digits().getAsString()
				<< ".\n Destination: " << m_callDss1->CallName() << ", address: " << calledNum.Digits().getAsString() 
				<< iLogW::EndRecord;
		}

        m_callSip->LinkUser(this);

		m_regRecord.SetName(m_log->NameStr().c_str());	
		m_regRecord.SetCalledInfo (calledNum.Digits().getAsString().c_str());
		m_regRecord.SetCallingInfo(callingNum.Digits().getAsString().c_str());
    }
    
	// -------------------------------------------------------------------------------------------------------

    Dss1ToSipCall::Dss1ToSipCall(
		Registrator registrator,
		Calls::StatisticCalls &stat,
		iLogW::ILogSessionCreator &logCreator,
        Utils::SafeRef<IDss1Call> callDss1,
		boost::shared_ptr<iReg::CallRecordWrapper> callRecord,
		Utils::SafeRef<iSip::ISipAccessPoint> sipPoint,
        boost::shared_ptr<iSip::IRemoteParticipantCreator::Parameters> parameters) :
		m_registrator(registrator),
		m_deleteProcess(false),
		m_stat(stat.OutgoingCall()),
		m_callDss1(callDss1),
		m_callRecord(callRecord),
		m_regRecord(m_callRecord->Record().Src())
    {
		m_registrator(this, true);

        // Dss1->Sip call
		InitLog(logCreator);
      
		m_media.reset(new iSip::MediaLineList(
			*m_log,
			boost::bind(&T::InitRemoteSdp, this, _1)));

		m_callDss1->SetUserCall(this);

		CreateMediaLine();

        m_callSip = sipPoint->CreateOutgoingParticipant(this, parameters);

		if(m_log->LogActive(m_infoTag))
		{
			*m_log << m_infoTag << "Created.\n" 
				<< " Source: " << m_callDss1->CallName() << ", address: " 
				<< iSip::SipUtils::ToString(parameters->SourceUserName)  
				<< ";\n Destination: address: " 
				<< iSip::SipUtils::ToString(parameters->GetDestination().uri().user())
				<< iLogW::EndRecord;
		}

		m_regRecord.SetName(m_log->NameStr().c_str());
		m_regRecord.SetCalledInfo(
			iSip::SipUtils::ToQString(parameters->SourceUserName));
		m_regRecord.SetCallingInfo(
			iSip::SipUtils::ToQString(parameters->GetDestination().uri().user()));
    }

	// -------------------------------------------------------------------------------------------------------

    Dss1ToSipCall::~Dss1ToSipCall()
    {
        if(m_log->LogActive(m_infoTag))
        {
            *m_log << m_infoTag << "Deleted. " << iLogW::EndRecord;
        }

        if(!m_callSip.IsEmpty()) m_callSip.Clear()->DestroyParticipant();

        if(!m_callDss1.IsEmpty()) m_callDss1.Clear()->Release(this);
    }

	// -------------------------------------------------------------------------------------------------------
	//impl iSip::IRemoteParticipantUser

	std::string Dss1ToSipCall::CallUserName() const
	{
		return m_log->NameStr();
	}

	// -------------------------------------------------------------------------------------

	iSip::ISipMedia* Dss1ToSipCall::Media()
	{
		return m_media.get();
	}

	// -------------------------------------------------------------------------------------


	const iSip::ISipMedia* Dss1ToSipCall::Media() const
	{
		return m_media.get();
	}

	// -------------------------------------------------------------------------------------

	boost::shared_ptr<iReg::CallRecordWrapper> Dss1ToSipCall::CallRecordByUser()
	{
		return m_callRecord;
	}

	// -------------------------------------------------------------------------------------
	// iSip::IRemoteParticipantUser

	void Dss1ToSipCall::OnParticipantTerminated(const iSip::IRemoteParticipant *src, 
		unsigned int statusCode)
	{
		ESS_ASSERT(m_callSip.IsEqualIntf(src));

		m_callSip.Clear();

		if(m_log->LogActive(m_infoTag))
		{
			*m_log << m_infoTag << "Sip call terminated.";
			if(statusCode) *m_log << " Code: " << statusCode;
			*m_log << iLogW::EndRecord;
		}
			
		m_regRecord.SetEndCode(0, "Ended by Sip");
		DestroyCall(statusCode);
	}

	// -------------------------------------------------------------------------------------------------------

	void Dss1ToSipCall::OnRefer(const iSip::IRemoteParticipant *src, const resip::SipMessage &msg)
	{
		ESS_ASSERT (m_callSip.IsEqualIntf(src));

		if (m_log->LogActive(m_infoTag))
		{
			*m_log << m_infoTag << "Call redirected. " 
				<< iSip::SipUtils::ToString(iSip::SipUtils::Short, msg) 
				<< iLogW::EndRecord;
		}

	}

	// -------------------------------------------------------------------------------------------------------

	void Dss1ToSipCall::OnParticipantAlerting(const iSip::IRemoteParticipant *src, 
		const resip::SipMessage& msg)
	{
		ESS_ASSERT(m_callSip.IsEqualIntf(src));

		if(m_log->LogActive(m_infoTag))
		{
			*m_log << m_infoTag << "Sip call alerted." << iLogW::EndRecord;
		}

		m_callDss1->Alert(this);
	}

	// -------------------------------------------------------------------------------------------------------

	void Dss1ToSipCall::OnParticipantConnected(const iSip::IRemoteParticipant *src,
		const resip::SipMessage &msg)
	{
		ESS_ASSERT(m_callSip.IsEqualIntf(src));

		if(m_log->LogActive(m_infoTag))
		{
			*m_log << m_infoTag << "Sip call connected." << iLogW::EndRecord;
		}

		m_stat.Talk();
		m_callDss1->Connect(this);
	}

	// -------------------------------------------------------------------------------------------------------

	void Dss1ToSipCall::OnParticipantRedirectSuccess(const iSip::IRemoteParticipant *src)
	{
		ESS_UNIMPLEMENTED;
	}

	// -------------------------------------------------------------------------------------------------------

	void Dss1ToSipCall::OnParticipantRedirectFailure(const iSip::IRemoteParticipant *src, 
		unsigned int statusCode)
	{
		ESS_UNIMPLEMENTED;
	}

	// -------------------------------------------------------------------------------------

	void Dss1ToSipCall::OnForkingOccured(const iSip::IRemoteParticipant *src, 
		Utils::SafeRef<iSip::IRemoteParticipant> newCall,
		const resip::SipMessage &msg)
	{
		newCall->DestroyParticipant();
	}
    
    // -------------------------------------------------------------------------------------------------------
    //impl ISDN::IDss1CallEvents

	void Dss1ToSipCall::Created(const IDss1Call *dss1Call)
	{
		ESS_ASSERT(m_callDss1.IsEqualIntf(dss1Call));

		CreateMediaLine();
	}

    // -------------------------------------------------------------------------------------------------------

	void Dss1ToSipCall::Alerted(const IDss1Call *dss1Call)
	{
		ESS_ASSERT(m_callDss1.IsEqualIntf(dss1Call));

		if(m_log->LogActive(m_infoTag))
		{
			*m_log << m_infoTag << "Dss1 call alerted." << iLogW::EndRecord;
		}

		m_callSip->AlertParticipant(false);
	}

    // -------------------------------------------------------------------------------------------------------	

	void Dss1ToSipCall::Connected(const IDss1Call *dss1Call)
	{
		ESS_ASSERT(m_callDss1.IsEqualIntf(dss1Call));

		if(m_log->LogActive(m_infoTag))
		{
			*m_log << m_infoTag << "Dss1 call connected." << iLogW::EndRecord;
		}
		m_stat.Talk();
		m_callSip->AnswerParticipant();
	}

	// -------------------------------------------------------------------------------------------------------	

	void Dss1ToSipCall::Disconnented(const IDss1Call *dss1Call,
		boost::shared_ptr<const ISDN::DssCause> cause)
	{
		ESS_ASSERT(m_callDss1.IsEqualIntf(dss1Call));

		if(m_log->LogActive(m_infoTag))
		{
			*m_log << m_infoTag << "Dss1 call disconnected.";

			if(cause != 0) *m_log << " Cause: " << cause->ToString() << iLogW::EndRecord;
		}
		m_callDss1.Clear();

		// 480 - Temporarily Unavailable
		int statusCode = (cause == 0) ? 480 :
			Dss1ToSipCauseConvertor::Dss1CauseToSipResponse(cause->GetCauseNum());			
		
		
		m_regRecord.SetEndCode((cause == 0) ? 0 : cause->GetCauseNum(), "Ended by Dss1");
		
		DestroyCall(statusCode);			
	}

	// -------------------------------------------------------------------------------------------------------

	iReg::CallSideRecord& Dss1ToSipCall::CallRecord()
	{
		return m_regRecord;
	}

	// -------------------------------------------------------------------------------------------------------

	void Dss1ToSipCall::InitLog(iLogW::ILogSessionCreator &logCreator)
	{
		static Utils::AtomicInt GCountCall;

		m_log.reset(logCreator.
			CreateSession(QString("Dss1ToSipCall_%1").arg(GCountCall.FetchAndAdd(1)).toStdString(), true));

		m_infoTag = m_log->RegisterRecordKind(L"Info", true);
	}

	// -------------------------------------------------------------------------------------------------------

	void Dss1ToSipCall::DestroyCall(int statusCode)
	{
		if(m_deleteProcess) return;

		m_deleteProcess = true;

		if(!m_callDss1.IsEmpty()) 
		{
			m_callDss1.Clear()->Release(this, 
				Dss1ToSipCauseConvertor::SipResponseToDss1Cause(statusCode));
		}
		if(!m_callSip.IsEmpty()) m_callSip.Clear()->DestroyParticipant(statusCode);

		m_registrator(this, false);
	}

	// -------------------------------------------------------------------------------------

	void Dss1ToSipCall::InitRemoteSdp(boost::shared_ptr<resip::SdpContents> offer)
	{
		if (m_log->LogActive(m_infoTag)) 
		{
			*m_log << m_infoTag << "Remote sdp: "
				<< iSip::SipUtils::ToString(*offer)
				<< iLogW::EndRecord;
		}
	}

	// -------------------------------------------------------------------------------------

	void Dss1ToSipCall::CreateMediaLine()
	{
		ESS_ASSERT (m_mediaLine == 0);

		m_mediaLine.reset(new iSip::MediaToCmpAdapter(*m_media, 
			boost::bind(&T::CmpChResolver, this)));

		if (!m_callSip.IsEmpty()) m_callSip->MediaObjectIsReady();
	}

	// -------------------------------------------------------------------------------------

	iCmpExt::ICmpChannel* Dss1ToSipCall::CmpChResolver()
	{
		return (m_callDss1.IsEmpty()) ? 0 : &m_callDss1->Rtp(this);
	}

};

