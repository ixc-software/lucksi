#pragma once

#include "stdafx.h"
#include "Domain/NamedObject.h"
#include "Domain/DomainClass.h"
#include "DRI/INonCreatable.h"
#include "Utils/ManagedList.h"
#include "Utils/IVirtualDestroy.h"
#include "ObjLink/ObjectLinksServer.h"
#include "ObjLink/ObjectLinkBinder.h"
#include "Calls/StatisticCalls.h"
#include "ISDN/DssUpIntf.h"
#include "ISDN/idsstogroup.h"
#include "Dss1Def.h"
#include "GateBoardInfoProc.h"
#include "IGateDss1.h"
#include "IDss1CallForGate.h"
#include "IGateDss1ForCall.h"

namespace iLogW {	class LogManager; };
namespace ISDN	
{	
	class NObjL3Profile;
	class IsdnStack; 
	struct IsdnStackSettings;
};

namespace Dss1ToSip
{
	class NObjDss1Interface;
	class NObjL1DriverForGate;
	class IHardwareToL1;
	class InfraGateDss1;

	class NObjDss1 : public Domain::NamedObject,
		public DRI::INonCreatable,
		public ObjLink::IObjectLinksHost,   //server 
		public ISDN::ICallbackDss,
		public IGateDss1,
		public IGateDss1ForCall,
		public ISDN::IDssToGroup,
		public IBoardInfoReq
	{
		Q_OBJECT;
		typedef NObjDss1 T;
	public:

		NObjDss1(Domain::NamedObject *pParent, 
			const Domain::ObjectName &name,
			IGateDss1Events &user,
			HardType::Value hardType,  
			ISDN::IeConstants::SideType
			sideType,  
			const QString &boardAlias, 
			int intfNumber = 0, 
			int DChannelSlot = Dss1Def::CDChannelDef);
		~NObjDss1();
		
		Q_INVOKABLE void Enable(bool par);

		void Start();
		void Stop();
		
		enum StateType
		{ 
			NotActive,
			Active, 
			WaitRestartComplete
		};
		Q_ENUMS(StateType);
		Q_PROPERTY(QString State READ GetStateStr());
		QString GetStateStr() const;
		QString GetShortStateStr() const;
		Q_PROPERTY(int ActivationCount READ m_activationCount);		
		Q_PROPERTY(QString LastActivation READ LastActivation);		
		Q_PROPERTY(QString LastDeactivation READ LastDeactivation);		

		Q_PROPERTY(QString HwType READ HardwareType);

		Q_INVOKABLE void AddInterface(const QString &boardAlias, int intfNumber);
		Q_INVOKABLE void DeleteInterface(const QString &boardAlias);

		Q_PROPERTY(int SigInterface READ SigInterface WRITE SigInterface);
		Q_PROPERTY(int AddressCompleteSize READ m_addressCompleteSize WRITE m_addressCompleteSize);
		Q_PROPERTY(bool SendAlertingTone READ m_sendAlertingTone WRITE m_sendAlertingTone);

		Q_PROPERTY(bool UseSharpAsNumberComplete READ m_useSharpAsNumberComplete WRITE m_useSharpAsNumberComplete);
//		Q_PROPERTY(bool DelHashFromNumber READ m_delSharpFromNumber WRITE m_delSharpFromNumber);

		Q_PROPERTY(bool RelAnonymCall READ m_relAnonymCall WRITE m_relAnonymCall);
		Q_PROPERTY(QString AnonymCallingAddr READ AnonymCallingAddr WRITE AnonymCallingAddr);
		QString AnonymCallingAddr() const {	return m_anonymCalledAddr.c_str(); }

		Q_PROPERTY(bool RestartReq READ m_restartReq WRITE m_restartReq);

		Q_PROPERTY(bool L2TraceInd READ L2TraceInd WRITE L2TraceInd);
		Q_PROPERTY(int  WaitAnswerTimeout READ m_waitAnswerTimeout WRITE m_waitAnswerTimeout);		

		Q_INVOKABLE void Calls(DRI::ICmdOutput *pOutput, bool brief = false);
		Q_INVOKABLE void Info(DRI::ICmdOutput *pOutput);
		Q_INVOKABLE void Stat(DRI::ICmdOutput *pOutput);
		Q_INVOKABLE void RestartStat();
		Q_INVOKABLE void CallsDetail(QString file);


		Q_INVOKABLE void L2Statistic(DRI::ICmdOutput *pOutput);
		
		Q_INVOKABLE void ClearL2Statistic();

		Q_INVOKABLE void L3Statistic(DRI::ICmdOutput *pOutput);

		Q_INVOKABLE void ClearL3Statistic();

		void L1Activated();
		void L1Deactivated();
		Utils::SafeRef<IHardwareToL1> HardwareToL1(int interfaceNumber);

	// IObjectLinksHost impl
	private:
		Utils::SafeRef<ObjLink::IObjectLinksServer> getObjectLinksServer();

	// ISDN::ICallBackDss
	private:
		void Activated();
		void Deactivated(); 
		void IncomingCall(IncommingCallParametrs par); 
		void DssErrorMsg(boost::shared_ptr<const ISDN::DssWarning> pWarning);
        void RestartComplete(CompleteInfo info);

	// IGateDss1
	public:
		Utils::SafeRef<IDss1Call> RunOutCall(
			Utils::SafeRef<IDss1CallEvents> user,
			const ISDN::DssPhoneNumber &calledNum, 
			const ISDN::DssPhoneNumber &callingNum);

		bool isDss1Active() const;

	// IGateDss1ForCall
	private:
		void IncomingCall(const IGateDss1ForCall *id,
			Utils::SafeRef<IDss1Call> call,
			const ISDN::DssPhoneNumber& calledAddress, 
			const ISDN::DssPhoneNumber& callingAddress);
		
		iCmpExt::ICmpChannelCreator& CmpChannelCreator(int interfaceNumber);
		
		void SaveCallInfo(int interfaceNumber,
			int channelNumber, Utils::WeakRef<GateCallInfo&> callInfo);
		
		QString InterfaceName(int interfaceNumber) const;

		void DeleteCall(const IDss1Call *call);

	// IDssToGroup
	private:
		int CountIntf() const;
		ISDN::SharedBChansWrapper CaptureAny(int count, int interfaceNumber, bool isOutgoing);
		ISDN::SharedBChansWrapper Capture(const ISDN::SetBCannels& inquredSet, 
			int intfId, bool exclusive, bool isOutgoing);

	// IBoardInfoReq
	private:
		void CallsInfoReq(QStringList &, bool brief) const;
		void BoardInfoReq(BoardInfo::List &inf) const;

	// property
	private:
		void SigInterface(int number);
		int SigInterface() const   {	return m_sigInterface; }
		
		QString LastActivation() const;
		QString LastDeactivation() const;

		QString	HardwareType() const;
		
		void AnonymCallingAddr(QString par) {	m_anonymCalledAddr = par.toStdString(); }

		bool L2TraceInd() const;
		void L2TraceInd(bool par);

		bool L3TraceInd() const;
		void L3TraceInd(bool par);

	private:
		NObjDss1Interface* FindBoard(int number);
		const NObjDss1Interface* FindBoard(int number) const;
		NObjDss1Interface* FindNextBoard(int number);
		bool IsStarted() const 
		{
			return m_isEnabled && m_isActivedByOwner;
		}
		void StartImpl();
		void StopImpl();
		void OnActivateDss1Stack();
		void OnDeactivateDss1Stack();
		void OnDeleteCall(IDss1Call*);
        ISDN::IsdnStackSettings GetIsdnSetting();
		void SetState(StateType state, const std::string &comment = "");
		void RegistratorLog(const QString &str);
		class CallProfile;

	private:
		bool m_isEnabled;
		bool m_isActivedByOwner;
		StateType m_state; 
		int m_activationCount;
		QDateTime m_lastActivation;
		QDateTime m_lastDeactivation;
		HardType::Value m_hardType;
		int m_sigInterface;
		bool m_useSharpAsNumberComplete;
		bool m_delSharpFromNumber;

		int m_addressCompleteSize;
		bool m_sendAlertingTone;
		bool m_relAnonymCall;
		std::string m_anonymCalledAddr;
		bool m_restartReq;
		bool m_l2traceInd;
		bool m_callTraceInd;
		int m_waitAnswerTimeout;
		Calls::StatisticCalls m_stat;
		IGateDss1Events &m_user;
		ISDN::NObjL3Profile *m_l3Profile;
		NObjL1DriverForGate *m_driverL1;
		typedef std::list<NObjDss1Interface*> ListBoards;
		ListBoards m_boards;
		class BoardOrderStrategy;
		boost::scoped_ptr<BoardOrderStrategy> m_boardStrategy;

		ObjLink::ObjectLinksServer m_server; // server
		ObjLink::ObjLinkStoreBinder m_myLinkBinderStorage;
		boost::scoped_ptr<InfraGateDss1> m_infra;


		boost::scoped_ptr<ISDN::IsdnStack> m_stackIsdn;
		typedef boost::ptr_list<IDss1Call> ListOfCalls;
		ListOfCalls m_calls;
	};
}

