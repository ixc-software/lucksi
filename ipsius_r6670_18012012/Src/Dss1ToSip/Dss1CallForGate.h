#pragma once

#include "stdafx.h"
#include "iCore/MsgObject.h"
#include "iCore/MsgTimer.h"
#include "ObjLink/ObjectLinksServer.h"
#include "ObjLink/ObjectLinkBinder.h"
#include "iLog/LogWrapper.h"
#include "Utils/TimerTicks.h"
#include "Calls/StatisticCalls.h"
#include "ISDN/DssUpIntf.h"
#include "IGateDss1ForCall.h"
#include "IDss1CallForGate.h"
#include "Dss1Def.h"

namespace Domain { class DomainClass; }
namespace ISDN
{ 
    class DssPhoneNumber;
    class DssCallParams;
	class IDssCallEvents;
};

namespace Dss1ToSip
{
	using ISDN::BinderToDssCall;
	using ISDN::DssCallParams;
	class GateCallInfo;

	class InfraGateDss1;

	class Dss1CallForGate : public iCore::MsgObject,
		public ObjLink::IObjectLinkOwner, // client
		public ObjLink::IObjectLinksHost,   //server 
		public ISDN::IDssCallEvents,
		iCmpExt::ICmpChannelEvent,
		public IDss1Call
	{
		typedef Dss1CallForGate T;

	public:
		struct Profile
		{
			Profile() : m_infra(0), m_owner(0), m_stat(0)
			{
				m_addressComplete = 0;
				m_infra = 0;
				m_sendAlertingTone = false;
				m_waitAnswerTimeout = 0;
				m_delSharpFromNumber = true;
			}

			bool IsValid() const
			{
				return m_infra != 0 && 
					m_owner != 0 && 
					m_stat != 0 &&
					!m_logCreator.IsEmpty();
			}
			InfraGateDss1 *m_infra;
			IGateDss1ForCall *m_owner;
			Calls::StatisticCalls *m_stat;
			int m_addressComplete;
			bool m_sendAlertingTone;
			int m_waitAnswerTimeout;
			bool m_delSharpFromNumber;
			Utils::SafeRef<iLogW::ILogSessionCreator> m_logCreator;
		};

		// Incoming call
		Dss1CallForGate(Dss1CallForGate::Profile &profile,
			ISDN::ICallbackDss::IncommingCallParametrs parametrs);

		// Outgoing call
		Dss1CallForGate(Utils::SafeRef<IDss1CallEvents> user,
			Dss1CallForGate::Profile &profile,
			ISDN::ILayerDss &dss1,
			boost::shared_ptr<ISDN::DssCallParams> dss1CallParams);

		~Dss1CallForGate();

		void Process();

	// IObjectLinkOwner impl
	private:
		void OnObjectLinkConnect(ObjLink::ILinkKeyID &linkID);

		void OnObjectLinkDisconnect(ObjLink::ILinkKeyID &linkID);

		void OnObjectLinkError(boost::shared_ptr<ObjLink::ObjectLinkError> error); 

		Domain::DomainClass &getDomain();

	// IObjectLinksHost impl
	private:
		Utils::SafeRef<ObjLink::IObjectLinksServer> getObjectLinksServer()
		{
			return m_server.getInterface();
		}

	// ISDN::IDssCallEvents Impl
	private:
		void DssCallCreated(DssCallCreatedParametrs params);
		void Alerting();        
		void ConnectConfirm();
		void Connected();
		void HoldInd();
		void RetriveInd();
		void DssCallErrorMsg(boost::shared_ptr<const ISDN::CallWarning> error);
		void Disconnent(boost::shared_ptr<const ISDN::DssCause> cause);
		void MoreInfo( Info info ); 
		void ProgressInd(ISDN::IeConstants::ProgressDscr descr);

	// IDss1Call impl
	private:
		void SetUserCall(Utils::SafeRef<IDss1CallEvents>);
		std::string CallName() const;
		void Alert(const IDss1CallEvents *);
		void Connect(const IDss1CallEvents *);
		void Release(const IDss1CallEvents *, int rejectCode = 0);
		iCmpExt::ICmpChannel &Rtp(const IDss1CallEvents *id);

	// iCmpExt::ICmpChannelEvent
	private:
		void ChannelCreated(iCmpExt::ICmpChannel *cmpChannel, iCmpExt::CmpChannelId channelId);
		void ChannelDeleted(iCmpExt::CmpChannelId channelId, const std::string &desc);
		void DtmfEvent(const iCmpExt::ICmpChannel *cmpChannel, const std::string &event);

	private:
		ISDN::BinderToUserCall getBinder();
		boost::shared_ptr<const ISDN::DssCause> Cause(int rejectCode) const;
		void ReleaseDss1(boost::shared_ptr<const ISDN::DssCause>);

	private:
		void InitLog(iLogW::ILogSessionCreator &logCreator);
		void LinkToBChannel(int interfaceNumber, const ISDN::SetBCannels &bChannels);
		void ProcessAddressForIncomingCall();
		void InitCallInfo();
		void WaitAnswerTimeout(iCore::MsgTimer *pT);
		void DeleteObj();
		iCmpExt::CmpChannelId GetCmpChannelId() const {	return this; }
		void ClearUserCallRef(boost::shared_ptr<const ISDN::DssCause> 
			cause = boost::shared_ptr<const ISDN::DssCause>());
		void SetupCallRecord();


	// members
	private:
		class CheckProfile : boost::noncopyable
		{
		public:
			CheckProfile(Profile &profile)
			{
				ESS_ASSERT(profile.IsValid());
			}
		};

		CheckProfile m_checkProfile;
		Utils::WeakRefHost m_selfRefHost; 
		boost::shared_ptr<GateCallInfo> m_callInfo;
		Calls::StatisticCalls::Call m_stat;
		boost::scoped_ptr<iLogW::LogSession>  m_log;
		iLogW::LogRecordTag m_infoTag;
		iLogW::LogRecordTag m_debugTag;
		iCore::MsgTimer m_waitAnswerTimer;
		InfraGateDss1 &m_infra;
		IGateDss1ForCall &m_gate;
		bool m_isIncomingCall;
		bool m_isDeleted;
		bool m_isAddressComplete;
		bool m_isAllerted;
		bool m_isConnected;
		int m_addressCompleteSize;
		bool m_sendAlertingTone;
		ISDN::DssPhoneNumber m_calledNumber;
		ISDN::DssPhoneNumber m_callingNumber;
		ObjLink::ObjectLinksServer m_server;
		ObjLink::ObjectLink<ISDN::IDssCall> m_dss1Call;
		ObjLink::ObjLinkStoreBinder m_myLinkBinderStorage;
		Utils::SafeRef<IDss1CallEvents> m_userCall;
		ISDN::SetBCannels m_bChannels;	
		int m_interfaceNumber;
		boost::scoped_ptr<iCmpExt::ICmpChannel> m_cmpChannel;
	};

}



