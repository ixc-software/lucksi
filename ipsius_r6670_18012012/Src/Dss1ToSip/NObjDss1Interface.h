#pragma once

#include "stdafx.h"
#include "Dss1Def.h"
#include "GateBoardInfoProc.h"
#include "gatebchannel.h" 
#include "IHardwareL1.h"
#include "Domain/NamedObject.h"
#include "Domain/DomainClass.h"
#include "DRI/INonCreatable.h"
#include "ISDN/iisdnintf.h"
#include "ISDN/BChannelsWrapper.h"
#include "iCmp/ChMngProto.h"
#include "iCmpExt/ICmpConnection.h"
#include "iCmpExt/ICmpChannel.h"
#include "iMedia/iCodec.h"
#include "Utils/WeakRef.h"

namespace iCmpExt		
{	
	class NObjCmpDevSettings;
	class NObjCmpConnection;
};

namespace Dss1ToSip
{
	class NObjDss1;
	class IHardwareToL1;
	class NObjLiuProfile;
	class GateCallInfo;

	class NObjDss1Interface : public Domain::NamedObject,
		public DRI::INonCreatable,
		public ISDN::IIsdnIntf,
		iCmpExt::ICmpConEvents,
		public iCmpExt::ICmpChannelCreator,
		public IL1ToHardware,
		public IBoardInfoReq
	{
		Q_OBJECT;
		typedef NObjDss1Interface T;\

	public:

		NObjDss1Interface(NObjDss1 &owner,
			const QString &aliasBoard,
			HardType::Value hardType,
			int intfNumber, int slotDChannel = Dss1Def::CDChannelNPos);

		~NObjDss1Interface();

		Q_PROPERTY(int InterfaceNumber READ InterfaceNumber);
		int InterfaceNumber() const {	return m_intfNumber; }

		Q_INVOKABLE void Enabled(bool par) {	(par) ? Enable() : Disable(); }
		Q_PROPERTY(QString State READ GetStateStr());

		void Start();
		void Stop();
		
		Q_INVOKABLE void Calls(DRI::ICmdOutput *pOutput, bool brief = false);
		Q_INVOKABLE void Info(DRI::ICmdOutput *pOutput);

		enum StrategyType 
		{ 
//			Fifo, 
			Lifo, 
//			Even,
//			Odd  
		};
		Q_ENUMS(StrategyType);

		enum ConnectionState
		{
			Inactive = 1,
			WaitGlobalSetupResp = 2,
			WaitE1InitResp = 4,
			E1InActive = 8,
			E1Active = 0x10
		};
		Q_ENUMS(ConnectionState);

		//Q_PROPERTY(StrategyType StrategyType READ GetStrategyType  WRITE SetStrategyType);

		Q_PROPERTY(int DChannel READ DChannel);//  WRITE DChannel);
		int DChannel() const {	return m_slotDChannel;	}
		Q_PROPERTY(QString BChannels READ BChannels); // WRITE BChannels);
		Q_PROPERTY(QString FreeOutCh READ FreeOutCh);
		Q_PROPERTY(QString FreeInCh READ FreeInCh );
		Q_PROPERTY(QString BusyCh READ BusyCh);
		Q_PROPERTY(QString OutBlockingCh READ OutBlockingCh WRITE OutBlockingCh);
		Q_PROPERTY(QString InBlockingCh READ InBlockingCh WRITE InBlockingCh);
		
		Q_INVOKABLE void LiuInfo(DRI::IAsyncCmd *pAsyncCmd);

		Q_PROPERTY(bool TestMode READ m_testMode WRITE m_testMode);

		void SaveCallInfo(int channelNumber, Utils::WeakRef<GateCallInfo&> callInfo);
		
		QString NameBoard() const;
		
        bool IsConnectionActive() const;
	
		ISDN::SharedBChansWrapper CaptureAnyBChannelWrap(int count, bool isOutgoing);
		ISDN::SharedBChansWrapper CaptureBChannelWrap(const ISDN::SetBCannels &inquredSet, bool isOutgoing);

	// iCmpExt::ICmpChannelCreator
	public:
		void CreateCmpChannel(iCmpExt::CmpChannelId id,
			int channelNumber, 
			Utils::WeakRef<iCmpExt::ICmpChannelEvent&> channelOwner,
			Utils::WeakRef<iLogW::ILogSessionCreator&> logCreator);
	
	// ISDN::IIsdnIntf	
	private:
		int NumberInterface() const;
		ISDN::SetBCannels CaptureBChannel(int count);
		bool CaptureBChannel(const ISDN::SetBCannels &);
		void FreeBChannel(const ISDN::SetBCannels &set);
		ISDN::IeConstants::UserInfo getPayload() const;

	// iCmpExt::ICmpConEvents
	private:
		void CmpActivated(const iCmpExt::ICmpConnection *connection);
		
		void CmpDeactivated(const iCmpExt::ICmpConnection *connection, 
			const std::string &desc);
		
		void OnBfTdmEvent(const iCmpExt::ICmpConnection *connection,
			const std::string &src, const std::string &event, const std::string &params);
		
		// E1
		void OnBfHdlcRecv(const iCmpExt::ICmpConnection *connection,
			const std::vector<Platform::byte> &pack);
		
		// AOZ
		void OnBfAbEvent(const iCmpExt::ICmpConnection *connection,
			const std::string &deviceName, int chNumber, 
			const std::string &eventName, const std::string &eventParams);

	// IL1ToHardware
	private:
		void DataReq(const IHardwareToL1*, const QVector<Platform::byte> packet);

	// IBoardInfoReq
	private:
		void CallsInfoReq(QStringList &, bool brief) const;
		void BoardInfoReq(BoardInfo::List &info) const;

	// Domain::IPropertyWriteEvent impl
	private:
		bool OnPropertyWrite(Domain::NamedObject *pObject, QString propertyName, QString val);

	// property
	private:
		QString GetStateStr() const;
		QString GetShortStateStr() const;
		void Enable();
		void Disable();

		StrategyType GetStrategyType() const;
		void SetStrategyType(StrategyType type);
		void InitChannelMask(int dChannel);
		QString  BChannels() const;
		QString  FreeOutCh() const;
		QString  FreeInCh() const;
		QString  BusyCh() const;
		QString  OutBlockingCh() const;
		void OutBlockingCh(QString par); 
		QString  InBlockingCh() const;
		void InBlockingCh(QString par);

	private:
		bool IsBoardStarted() const { return m_isEnabled && m_isActivatedByOwner; }

		iCmpExt::CmpChannelSettings GetCmpChannelSettings(int channel) const;

		void SendGlobalSetupDev();
		void GlobalSetupResponseOk();
		void SendE1Init();
		void E1InitResponse(const iCmp::BfInitInfo &);
		void CmpDeactivatedImpl();
		void ResetConnection(const std::string &msg);
		iCmp::BfInitDataE1 GetInitData() const;
		void OnReset(const std::string &msg);
		void SetConnectionState(int state, const std::string &comment = "");
		void OnGlobalSetup();
		ISDN::SetBCannels FreeChannelsForCall(bool isOutgoing) const;
		void RegistratorLog(const QString &str);

	private:
		NObjDss1 &m_owner;
		bool m_isEnabled;
		bool m_isActivatedByOwner;
		int  m_connectionState; // ConnectionState

		bool m_testMode;
		HardType::Value m_hardType;
		int m_intfNumber;
		int m_slotDChannel;
		ISDN::SetBCannels m_bChannels;
		ISDN::SetBCannels m_busyChannels;
		ISDN::SetBCannels m_outBlocking;
		ISDN::SetBCannels m_inBlocking;
		class BChOrderStrategy;
		boost::scoped_ptr<BChOrderStrategy> m_chStrategy;

		Utils::SafeRef<IHardwareToL1> m_L1Driver;
		class StaticSettings;
		boost::scoped_ptr<StaticSettings> m_settings;
		iCmpExt::NObjCmpConnection *m_connection;
		typedef std::vector<GateBChannel> BChannelsList;
		BChannelsList m_bChannelsObj;
	};
}


