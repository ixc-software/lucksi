#pragma once

#include "stdafx.h"
#include "iLog/LogWrapper.h"
#include "Calls/StatisticCalls.h"
#include "iSip/ISipAccessPoint.h"
#include "ISDN/DssUpIntf.h"
#include "IDss1CallForGate.h"

namespace sdpcontainer	{	class Sdp;	}

namespace iSip	
{	
	class MediaLineList;	
	class MediaToCmpAdapter;
}

namespace Dss1ToSip 
{
	class NObjDss1ToSip;
	class IRtpSessionForGate;
	class RtpForGate;
	class IGateDss1;

    class Dss1ToSipCall : boost::noncopyable,
        public virtual Utils::SafeRefServer,
        public iSip::IRemoteParticipantUser,
		public IDss1CallEvents
    {
		typedef Dss1ToSipCall T;

		typedef boost::function<void (T*, bool)> Registrator;

    public:
	
		// Sip->Dss1 call
        Dss1ToSipCall(
			Registrator registrator,
			Calls::StatisticCalls &stat,
			iLogW::ILogSessionCreator &logCreator,
            Utils::SafeRef<iSip::IRemoteParticipant> callSip,
			IGateDss1 &gateDss1,
			const ISDN::DssPhoneNumber& calledNum, 
			const ISDN::DssPhoneNumber& callingNum); 
        
        // Dss1->Sip call
        Dss1ToSipCall(
			Registrator registrator,
			Calls::StatisticCalls &stat,
			iLogW::ILogSessionCreator &logCreator,
            Utils::SafeRef<IDss1Call> dss1Call,
			boost::shared_ptr<iReg::CallRecordWrapper> callRecord,
			Utils::SafeRef<iSip::ISipAccessPoint> sipPoint,
			boost::shared_ptr<iSip::IRemoteParticipantCreator::Parameters> parameters);

        ~Dss1ToSipCall();

    // iSip::IRemoteParticipantUser
    private:

		std::string CallUserName() const;

		iSip::ISipMedia* Media();

		const iSip::ISipMedia* Media() const;

		boost::shared_ptr<iReg::CallRecordWrapper> CallRecordByUser();

		void OnParticipantTerminated(const iSip::IRemoteParticipant *src, 
			unsigned int statusCode = 0);

		void OnRefer(const iSip::IRemoteParticipant *src, const resip::SipMessage &msg);

		void OnParticipantAlerting(const iSip::IRemoteParticipant *src, 
			const resip::SipMessage& msg);
		
		void OnParticipantConnected(const iSip::IRemoteParticipant *src,
			const resip::SipMessage &msg);

		void OnParticipantRedirectSuccess(const iSip::IRemoteParticipant *src);

		void OnParticipantRedirectFailure(const iSip::IRemoteParticipant *src, 
			unsigned int statusCode);

		void OnForkingOccured(const iSip::IRemoteParticipant *src, 
			Utils::SafeRef<iSip::IRemoteParticipant> newCall,
			const resip::SipMessage &msg);

    //impl IDss1CallEvents
    private:

		void Created(const IDss1Call *dss1Call);
		void Alerted(const IDss1Call *dss1Call); 
		void Connected(const IDss1Call *dss1Call); 
		void Disconnented(const IDss1Call *dss1Call,
			boost::shared_ptr<const ISDN::DssCause> pCause);
		iReg::CallSideRecord& CallRecord();

    private:

		void InitLog(iLogW::ILogSessionCreator &logCreator);
		void DestroyCall(int statusCode = 0);
		void InitRemoteSdp(boost::shared_ptr<resip::SdpContents> offer);

		iCmpExt::ICmpChannel* CmpChResolver();

		void CreateMediaLine();

	private:

		Registrator m_registrator;

		bool m_deleteProcess;
		Calls::StatisticCalls::Call m_stat;
        boost::scoped_ptr<iLogW::LogSession>  m_log;
        iLogW::LogRecordTag m_infoTag;
		
		boost::scoped_ptr<iSip::MediaLineList> m_media;
		boost::scoped_ptr<iSip::MediaToCmpAdapter> m_mediaLine;

        Utils::SafeRef<IDss1Call> m_callDss1;
        Utils::SafeRef<iSip::IRemoteParticipant> m_callSip;

		boost::shared_ptr<iReg::CallRecordWrapper> m_callRecord;
		iReg::CallSideRecord &m_regRecord;

	};
}; 


