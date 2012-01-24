#include "stdafx.h"
#include "Dss1CallForGate.h"
#include "Domain/DomainClass.h"
#include "InfraGateDss1.h"
#include "TdmMng/TdmException.h"
#include "GateCallInfo.h"
#include "iReg/CallRecord.h"



namespace Dss1ToSip
{
    // ------------------------------------------------------------------------------------------------------
    // Incoming call. user <- Dss1

    Dss1CallForGate::Dss1CallForGate(Dss1CallForGate::Profile &profile,
        ISDN::ICallbackDss::IncommingCallParametrs par) : 
        MsgObject(profile.m_infra->Domain().getMsgThread()),
		m_checkProfile(profile),
		m_stat(profile.m_stat->IncomingCall()),
        m_infra(*profile.m_infra),
        m_gate(*profile.m_owner),
		m_waitAnswerTimer(this, &T::WaitAnswerTimeout),
        m_isIncomingCall(true),
		m_isDeleted(false),
        m_isAddressComplete(false),
		m_isAllerted(false),
		m_isConnected(false),
		m_addressCompleteSize(profile.m_addressComplete),
		m_sendAlertingTone(profile.m_sendAlertingTone),
        m_calledNumber(par.m_callParams->GetCalled()),
        m_callingNumber(par.m_callParams->GetCalling()),
	    m_server(m_infra.Domain(), this),
        m_dss1Call(*this),
		m_interfaceNumber(-1)
    {
		ESS_ASSERT(par.m_callParams->InterfaceNumber().IsInited());
        ESS_ASSERT( par.m_bindToDssCall->Connect(m_dss1Call) );
	
		InitCallInfo();

		InitLog(*profile.m_logCreator.Clear());

		if(m_log->LogActive(m_infoTag))
		{
			*m_log << m_infoTag << "Created. Dss1->Sip. Dss1 call: " 
				<< par.m_unicCallName << iLogW::EndRecord;
		}
		LinkToBChannel(par.m_callParams->InterfaceNumber(), par.m_callParams->GetAckBchans());
    }

    // ------------------------------------------------------------------------------------------------------
    // Outgoing call. user -> Dss1

	Dss1CallForGate::Dss1CallForGate(Utils::SafeRef<IDss1CallEvents> user,
		Dss1CallForGate::Profile &profile,
        ISDN::ILayerDss &dss1,
        boost::shared_ptr<ISDN::DssCallParams> dss1CallParams) : 
		MsgObject(profile.m_infra->Domain().getMsgThread()),
		m_checkProfile(profile),
		m_stat(profile.m_stat->OutgoingCall()),
		m_infra(*profile.m_infra),
		m_gate(*profile.m_owner),
		m_waitAnswerTimer(this, &T::WaitAnswerTimeout),
		m_isIncomingCall(false),
		m_isDeleted(false),
		m_isAddressComplete(true),
		m_isAllerted(false),
		m_addressCompleteSize(0),
		m_sendAlertingTone(profile.m_sendAlertingTone),
        m_calledNumber(dss1CallParams->GetCalled()),
        m_callingNumber(dss1CallParams->GetCalling()),
        m_server(m_infra.Domain(), this),
		m_dss1Call(*this),
		m_userCall(user),
		m_interfaceNumber(-1)
    {
		InitCallInfo();

		InitLog(*profile.m_logCreator.Clear());
        if(m_log->LogActive(m_infoTag))
        {
            *m_log << m_infoTag << "Created. Sip->Dss1." << iLogW::EndRecord;
        }
        dss1.CreateOutCall(ISDN::ILayerDss::CreateOutCallParametrs(getBinder(), dss1CallParams, m_log->NameStr()));

		if(profile.m_waitAnswerTimeout != 0) m_waitAnswerTimer.Start(profile.m_waitAnswerTimeout);
    }

    // ------------------------------------------------------------------------------------------------------

    Dss1CallForGate::~Dss1CallForGate()
    {
		ClearUserCallRef();

		if(m_log->LogActive(m_infoTag))
		{
			*m_log << m_infoTag << "Deleted. " << iLogW::EndRecord;
		}
    }

    // ------------------------------------------------------------------------------------------------------
    // ISDN::IDssCallEvents Impl

    void Dss1CallForGate::DssCallCreated(DssCallCreatedParametrs params)
    {
		if(m_log->LogActive(m_infoTag))
		{
			*m_log << m_infoTag << "Alocated B-channel: " << params.m_setBCannels.ToString()
                << iLogW::EndRecord;
        }

        // передаем пользователю информацию о канале
        params.m_linkBinder->Connect(m_dss1Call);

        if(m_userCall.IsEmpty())
        {
            ReleaseDss1(Cause(ISDN::IeConstants::NormCallClearing));
            return;
        }
	
		LinkToBChannel(params.m_bChansIntfId, params.m_setBCannels);
	} 

    // ------------------------------------------------------------------------------------------------------

    void Dss1CallForGate::Alerting()
    {
		if(m_log->LogActive(m_infoTag))
		{
			*m_log << m_infoTag << "Alerting." << iLogW::EndRecord;
        }
		m_callInfo->ChangeState(GateCallState::Ringing);
        if(m_cmpChannel != 0  && !m_userCall.IsEmpty()) m_userCall->Alerted(this);
    }

    // ------------------------------------------------------------------------------------------------------

    void Dss1CallForGate::ConnectConfirm()
    {
		if(m_log->LogActive(m_infoTag))
		{
			*m_log << m_infoTag << "Connect confirm." << iLogW::EndRecord;
        }
    }

    // ------------------------------------------------------------------------------------------------------

    void Dss1CallForGate::Connected()
    {
		if(m_log->LogActive(m_infoTag))
		{
			*m_log << m_infoTag << "Connected." << iLogW::EndRecord;
        }
		m_isConnected = true;
		m_stat.Talk();
		m_callInfo->ChangeState(GateCallState::Talk);
        
		m_waitAnswerTimer.Stop();
		if(m_cmpChannel != 0  && !m_userCall.IsEmpty()) m_userCall->Connected(this);
    }

	// ------------------------------------------------------------------------------------------------------

	void Dss1CallForGate::HoldInd()
	{
		ESS_UNIMPLEMENTED;
	}

	// ------------------------------------------------------------------------------------------------------

	void Dss1CallForGate::RetriveInd()
	{
		ESS_UNIMPLEMENTED;
	}

    // ------------------------------------------------------------------------------------------------------

    void Dss1CallForGate::DssCallErrorMsg(boost::shared_ptr<const ISDN::CallWarning> error)
    {
		if(m_log->LogActive(m_infoTag))
		{
			*m_log << m_infoTag << "Error. " << error->ToString() << iLogW::EndRecord;
        }
		
		ClearUserCallRef();

        if(m_dss1Call.Connected()) m_dss1Call.Disconnect();
    }

    // ------------------------------------------------------------------------------------------------------    

    void Dss1CallForGate::Disconnent(boost::shared_ptr<const ISDN::DssCause> cause)
    {
		if(m_log->LogActive(m_infoTag))
		{
			*m_log << m_infoTag << "Disconnent indication. "; 
			if(cause != 0) *m_log << cause->ToString();
			*m_log << iLogW::EndRecord;
        }

        ClearUserCallRef(cause);

		DeleteObj();
    }

    // ------------------------------------------------------------------------------------------------------

    void Dss1CallForGate::MoreInfo( Info info )
    {
        ESS_ASSERT(!m_isAddressComplete);
        
        if(!m_dss1Call.Connected()) return;
    
        m_calledNumber.AppendDigits(info.Num.Digits());
		m_callInfo->DstCall(m_calledNumber.Digits().getAsString().c_str());        

        ProcessAddressForIncomingCall();
    }

    // -------------------------------------------------------------------------------------------------------

    void Dss1CallForGate::ProgressInd(ISDN::IeConstants::ProgressDscr descr)
    {
        // nothing to do
    }

    // ------------------------------------------------------------------------------------------------------

    void Dss1CallForGate::OnObjectLinkConnect(ObjLink::ILinkKeyID &linkID)
    {
        ESS_ASSERT(m_dss1Call.Equal(linkID));

		if(m_log->LogActive(m_debugTag))
		{
			*m_log << m_debugTag << "Object link connect." << iLogW::EndRecord;
        }

        if (m_isIncomingCall)
		{
            m_dss1Call->SetLink(m_myLinkBinderStorage.getBinder<ISDN::IDssCallEvents>(this));
			ProcessAddressForIncomingCall();
		}
    }

    // ------------------------------------------------------------------------------------------------------

    void Dss1CallForGate::OnObjectLinkDisconnect(ObjLink::ILinkKeyID &linkID)
    {
        ESS_ASSERT(m_dss1Call.Equal(linkID));
        
		if(m_log->LogActive(m_debugTag))
		{
			*m_log << m_debugTag << "Object link disconnect." << iLogW::EndRecord;
        }
		
		ClearUserCallRef();
		
		DeleteObj();
    }

    // ------------------------------------------------------------------------------------------------------

    void Dss1CallForGate::OnObjectLinkError(boost::shared_ptr<ObjLink::ObjectLinkError> error)
    {
        ESS_ASSERT(error->Equal(m_dss1Call));    

		if(m_log->LogActive(m_debugTag))
		{
			*m_log << m_debugTag << "Object link error." << error->ToString() << iLogW::EndRecord;
        }

		DeleteObj();
    }

    // ------------------------------------------------------------------------------------------------------

    Domain::DomainClass &Dss1CallForGate::getDomain()
    {
        return m_infra.Domain();
    }

    // ------------------------------------------------------------------------------------------------------
    // IDss1Call impl

    void Dss1CallForGate::SetUserCall(Utils::SafeRef<IDss1CallEvents> user)
    {
        ESS_ASSERT(m_userCall.IsEmpty());

		m_userCall = user;
    }

    // ------------------------------------------------------------------------------------------------------

    std::string Dss1CallForGate::CallName() const
    {
        return m_log->NameStr();
    }

    // ------------------------------------------------------------------------------------------------------

    void Dss1CallForGate::Alert(const IDss1CallEvents *id)
    {
        ESS_ASSERT(m_userCall.IsEqualIntf(id));

		if(m_log->LogActive(m_infoTag))
		{
			*m_log << m_infoTag << "Alert." << iLogW::EndRecord;
        }

		if (!m_dss1Call.Connected()) return;

		if(m_isAllerted) return;
		m_callInfo->ChangeState(GateCallState::Ringing);		
		
		if(m_sendAlertingTone && m_cmpChannel != 0 )	
		{
			m_cmpChannel->SendTone(iDSP::Gen2xProfile::Alert, true);
		}
		m_dss1Call->Alerting();
		m_isAllerted = true;
    }

    // ------------------------------------------------------------------------------------------------------

    void Dss1CallForGate::Connect(const IDss1CallEvents *id)
    {
        ESS_ASSERT(m_userCall.IsEqualIntf(id));

		if(m_log->LogActive(m_infoTag))
		{
			*m_log << m_infoTag << "Connect." << iLogW::EndRecord;
        }
		
		if (!m_dss1Call.Connected()) return;

		if(!m_isAllerted) m_dss1Call->Alerting();

		m_isAllerted = true;
		m_isConnected = true;
		m_callInfo->ChangeState(GateCallState::Talk);
		m_stat.Talk();		
        m_dss1Call->ConnectRsp();
    }

    // ------------------------------------------------------------------------------------------------------

    void Dss1CallForGate::Release(const IDss1CallEvents *id, int rejectCode)
    {
        ESS_ASSERT(m_userCall.IsEqualIntf(id));
        
        if(!m_userCall.IsEmpty()) 
		{
			SetupCallRecord();
			m_userCall.Clear();
		}

        boost::shared_ptr<const ISDN::DssCause> cause(Cause(rejectCode));
		if(m_log->LogActive(m_infoTag))
		{
			*m_log << m_infoTag << "Disconnent from user. " << cause->ToString() << iLogW::EndRecord;
        }

        ReleaseDss1(cause);
    }

	// ------------------------------------------------------------------------------------------------------

	iCmpExt::ICmpChannel &Dss1CallForGate::Rtp(const IDss1CallEvents *id)
	{
        ESS_ASSERT(m_userCall.IsEqualIntf(id));

		return *m_cmpChannel;
	}

	// ------------------------------------------------------------------------------------------------------
	// iCmpExt::ICmpChannelEvent

	void Dss1CallForGate::ChannelCreated(iCmpExt::ICmpChannel *cmpChannel, 
		iCmpExt::CmpChannelId channelId)
	{
		ESS_ASSERT(m_cmpChannel == 0);
		
		ESS_ASSERT(GetCmpChannelId() == channelId);
		
		m_cmpChannel.reset(cmpChannel);
		
		if(m_isIncomingCall)
		{
			if(m_isAddressComplete)	m_gate.IncomingCall(&m_gate, this, m_calledNumber, m_callingNumber);
			return;
		}

		if(m_userCall.IsEmpty()) return;

		m_userCall->Created(this); 

		if(m_callInfo->GetState() == GateCallState::Ringing)
			m_userCall->Alerted(this);
		else if(m_callInfo->GetState() == GateCallState::Talk)
			m_userCall->Connected(this);

	}

	// ------------------------------------------------------------------------------------------------------

	void Dss1CallForGate::ChannelDeleted(iCmpExt::CmpChannelId channelId, const std::string &desc)
	{
		ESS_ASSERT(GetCmpChannelId() == channelId);

		m_cmpChannel.reset();

		if(m_log->LogActive(m_infoTag))
		{
			*m_log << m_infoTag << "Channel error.";
			if(!desc.empty()) *m_log << " " << desc;
			*m_log << iLogW::EndRecord;
		}

		boost::shared_ptr<const ISDN::DssCause> 
			cause(Cause(ISDN::IeConstants::TemporaryFailure));

		ClearUserCallRef(cause);

		ReleaseDss1(cause);
	}
	
	// ------------------------------------------------------------------------------------------------------
	
	void Dss1CallForGate::DtmfEvent(const iCmpExt::ICmpChannel *cmpChannel, const std::string &event)
	{
		if(m_log->LogActive(m_infoTag))
		{
			*m_log << m_infoTag << "Unexpected event: " << event << iLogW::EndRecord;
		}

	}

	// ------------------------------------------------------------------------------------------------------

	ISDN::BinderToUserCall Dss1CallForGate::getBinder()
	{
		return m_myLinkBinderStorage.getBinder<ISDN::IDssCallEvents>(this);
	}

	// ------------------------------------------------------------------------------------------------------

	boost::shared_ptr<const ISDN::DssCause> Dss1CallForGate::Cause(int rejectCode) const 
	{
		return ISDN::DssCause::Create(m_infra.IsdnInfra(),
			ISDN::IeConstants::TN,
			ISDN::IeConstants::CCITT, 
			ISDN::IeConstants::CauseNum(rejectCode));
	}

    // ------------------------------------------------------------------------------------------------------

    void Dss1CallForGate::ReleaseDss1(boost::shared_ptr<const ISDN::DssCause> cause)
    {
        if (!m_dss1Call.Connected()) return;

        m_dss1Call->Disconnent(cause);
        m_dss1Call.Disconnect();
    }

    // ------------------------------------------------------------------------------------------------------

	void Dss1CallForGate::InitLog(iLogW::ILogSessionCreator &logCreator)
	{
		ESS_ASSERT(m_log == 0);
		
		static int i = 0;
		m_log.reset(logCreator.CreateSession("Dss1CallWrapper_" + Utils::IntToString(i++), true));
		m_infoTag = m_log->RegisterRecordKind(L"Info", true);
		m_debugTag = m_log->RegisterRecordKind(L"Debug", true);
	}


    // ------------------------------------------------------------------------------------------------------

    void Dss1CallForGate::ProcessAddressForIncomingCall()
    {
		ESS_ASSERT(!m_isAddressComplete);
		
		if (!m_dss1Call.Connected()) return;

        m_isAddressComplete = 
            m_calledNumber.Digits().getAsString().size() >= m_addressCompleteSize; 

        if(!m_isAddressComplete)
        {
            m_dss1Call->AddressIncomplete();
			return;
        }
		
		m_dss1Call->AddressComplete(); 
		if(m_cmpChannel != 0)	m_gate.IncomingCall(&m_gate, this, m_calledNumber, m_callingNumber);
    }

    // ------------------------------------------------------------------------------------------------------

	void Dss1CallForGate::LinkToBChannel(int interfaceNumber, const ISDN::SetBCannels &bChannels)
	{
		ESS_ASSERT(bChannels.Count() == 1);
	
		m_bChannels = bChannels;
		m_interfaceNumber = interfaceNumber;
		
		ESS_ASSERT(m_cmpChannel == 0);

		int channelNumber = m_bChannels.MinValue();
		m_gate.CmpChannelCreator(m_interfaceNumber).CreateCmpChannel(GetCmpChannelId(),
			channelNumber,
			m_selfRefHost.Create<iCmpExt::ICmpChannelEvent&>(*this), 
			m_log->LogCreatorWeakRef());

		m_gate.SaveCallInfo(m_interfaceNumber, channelNumber, m_callInfo->SafeRef());
	}

	// ------------------------------------------------------------------------------------------------------

	void Dss1CallForGate::WaitAnswerTimeout(iCore::MsgTimer *pT)
	{
		if(m_isConnected) return;

		if(m_log->LogActive(m_infoTag))
		{
			*m_log << m_infoTag << "Wait answer timer expired." << iLogW::EndRecord;
		}

		boost::shared_ptr<const ISDN::DssCause> 
			cause(Cause(ISDN::IeConstants::NoUserResponding));

		ClearUserCallRef(cause);

		ReleaseDss1(cause);
	}

	// ------------------------------------------------------------------------------------------------------

	void Dss1CallForGate::DeleteObj()
	{
		if(m_isDeleted) return;
			
		m_isDeleted = true;
		m_gate.DeleteCall(this);
	}

	// ------------------------------------------------------------------------------------------------------

	void Dss1CallForGate::InitCallInfo()
	{
		m_callInfo.reset(new GateCallInfo(m_isIncomingCall, 
			m_callingNumber.Digits().getAsString().c_str(),
			m_calledNumber.Digits().getAsString().c_str()));
	}

	// ------------------------------------------------------------------------------------------------------

	void Dss1CallForGate::ClearUserCallRef(
		boost::shared_ptr<const ISDN::DssCause> cause)
	{
		if(m_userCall.IsEmpty()) return;

		SetupCallRecord();
			
		m_userCall.Clear()->Disconnented(this, cause);
	}

	// ------------------------------------------------------------------------------------------------------

	void Dss1CallForGate::SetupCallRecord()
	{
		ESS_ASSERT (!m_userCall.IsEmpty());

		if (m_interfaceNumber != -1)
		{
			m_userCall->CallRecord().SetAdditionalInfo(
				QString("Interface : %1, BChannel: %2").
					arg(m_gate.InterfaceName(m_interfaceNumber)).
					arg(m_bChannels.MinValue()));
		}
	}

} 
