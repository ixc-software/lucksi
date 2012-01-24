#ifndef _NOBJ_CMP_CONNECTION_H_
#define _NOBJ_CMP_CONNECTION_H_

#include "stdafx.h"
#include "Domain/NamedObject.h"
#include "Domain/DomainClass.h"
#include "DRI/INonCreatable.h"
#include "IpsiusService/IAllocBoard.h"
#include "SafeBiProtoExt/ISbpConnection.h"
#include "SafeBiProto/SbpSettings.h"
#include "iCmp/ChMngProto.h"
#include "ICmpChannel.h"
#include "ChDataCapture.h"
#include "ICmpConnection.h"
#include "iCmpConfPoint.h"
#include "CmpHandleManager.h"
#include "CmpBoardMsg.h"
#include "CmpAozMsg.h"

namespace SBProto	 {	class SbpSettings; };
namespace SBProtoExt {	class SbpLogSettings; };

namespace IpsiusService {	class NObjHwFinder; };

namespace iCmpExt
{

	class CmpChannel;

	class NObjCmpConnection  :  public Domain::NamedObject,
		public DRI::INonCreatable,
		public IpsiusService::IAllocBoard,
		public ICmpConnection,
		public SBProtoExt::ISbpConnectionEvents,
		iCmp::IPcSideEventRecv,
		public ICmpChannelCreator,
		public ICmpManager,
		IChDataCaptureSender
	{
		Q_OBJECT;
		typedef NObjCmpConnection T;

	public:

		NObjCmpConnection(Domain::NamedObject *pParent,
			ICmpConEvents &user,
			boost::function<CmpChannelSettings (int)> cmpChannelSettings,
			const SBProto::SbpSettings &sbpSettings,
			boost::shared_ptr<SBProtoExt::SbpLogSettings> sbpLogSettings,
			const QString &boardAlias);

		~NObjCmpConnection();

		Q_PROPERTY(QString NameBoard READ DriNameBoard);

	public:
		void Enable(bool val);
		void ResetConnection(const std::string &reason);

	// DRI
	public:

		enum CmpState
		{ 
			Disabled,
			Allocating,
			Connecting,
			Active
		};

		Q_ENUMS(CmpState);
		Q_PROPERTY(QString State READ GetState);

		Q_PROPERTY(int CountActivation READ m_countActivation);
		Q_PROPERTY(QString LastActivation READ LastActivation);		
		Q_PROPERTY(QString LastDeactivation READ LastDeactivation);		
		Q_PROPERTY(QString LastBoardError READ m_lastBoardError);
	
		Q_PROPERTY(iCmpExt::ChDataCaptureMode::Value DataCaptureMode READ m_dataCaptureMode WRITE m_dataCaptureMode);
		Q_PROPERTY(int MaxDataCaptureCount READ m_maxDataCaptureCount WRITE m_maxDataCaptureCount);
		Q_PROPERTY(int CurrDataCaptureCount READ m_currDataCaptureCount WRITE m_currDataCaptureCount);

		Q_INVOKABLE void StateInfo(DRI::IAsyncCmd *pAsyncCmd);
		Q_INVOKABLE void TdmInfo(DRI::IAsyncCmd *pAsyncCmd);
		Q_INVOKABLE void ChannelInfo(DRI::IAsyncCmd *pAsyncCmd, int chNum, bool lastCall);
		Q_INVOKABLE void ConferenceInfo(DRI::IAsyncCmd *pAsyncCmd, int confHandle);

		CmpBoardMsg BoardMsg();

		CmpInfoReqMsg InfoReqMsg(DRI::IAsyncCmd *pAsyncCmd);

		CmpAozMsg AozMsg();

	// ICmpChannelCreator
	public:
		void CreateCmpChannel(CmpChannelId id,
			int channelNumber, 
			Utils::WeakRef<ICmpChannelEvent&> channelOwner,
			Utils::WeakRef<iLogW::ILogSessionCreator&> logCreator);

	// ICmpManager	
	public:
		ICmpConfManager &ConfManager();

	// ICmpConnection
	public:
		bool IsCmpActive() const;

		Utils::HostInf BoardHost() const;

		const std::string& NameBoard() const {	return m_boardAlias; }

		SBProtoExt::SbpMsgSender SbpSender(iCmp::IPcSideRecv *receiver = 0);

		ChDataCapture* CreateChDataCapture(int channel, 
			const std::string &ownerDesc);

	// IpsiusService::IAllocBoard
	private:
		void BoardAllocated(QString alias, const BfBootCore::BroadcastMsg &msg);
        void BoardUnallocated(QString alias);

	// SBProtoExt::ISbpConnectionEvents
	private:
		void CommandReceived(Utils::SafeRef<SBProtoExt::ISbpConnection> src,
			boost::shared_ptr<SBProto::SbpRecvPack> data);
		void ResponseReceived(Utils::SafeRef<SBProtoExt::ISbpConnection> src,
			boost::shared_ptr<SBProto::SbpRecvPack> data);
		void InfoReceived(Utils::SafeRef<SBProtoExt::ISbpConnection> src,
			boost::shared_ptr<SBProto::SbpRecvPack> data);
		void ProtocolError(Utils::SafeRef<SBProtoExt::ISbpConnection> src,
			boost::shared_ptr<SBProto::SbpError> err);

		void ConnectionActivated(Utils::SafeRef<SBProtoExt::ISbpConnection> src);
		void ConnectionDeactivated(Utils::SafeRef<SBProtoExt::ISbpConnection> src,
			const std::string &);

	// iCmp::IPcSideEventRecv
	private:
		void OnBfTdmEvent(const std::string &src, const std::string &event, const std::string &params);
        void OnBfGetChannelData(const std::string &devName, int chNum, 
            const std::vector<Platform::byte> &rx, 
            const std::vector<Platform::byte> &tx,
            const std::vector<Platform::byte> &rxFixed);		
		// E1
		void OnBfHdlcRecv(const std::vector<Platform::byte> &pack);
		// AOZ
		void OnBfAbEvent(const std::string &deviceName, int chNumber, 
			const std::string &eventName, const std::string &eventParams);

	// IChDataCaptureSender
	private:
		void SendDataCapture(int channel, bool enable);

	private:
		void ReqToBoardComplete(const std::string &result);
		void ReqToBoardIncomplete(const std::string &errorMsg);
		void ReqToBoardHalted(const std::string &result);

		QString GetState() const;

		void OnProtoProcess(iCore::MsgTimer *pT);
		void OnMsgReceived(boost::shared_ptr<SBProto::SbpRecvPack> data);
		void ResetImpl(const std::string &err);
		void ConnectionDeactivatedImpl(const std::string &);

		void ProtocolErrorImpl(const std::string &err);

		void StartSbp();
		void StopSbp();
		void OnResetConnection(const std::string &reason);
		void OnStartConnection();
		void OnDeleteConnection(const std::string &nameConnection);

		QString LastActivation() const;
		QString LastDeactivation() const;
		void ChangeState(CmpState newState, const std::string &desc = "");

		QString DriNameBoard() const {	return m_boardAlias.c_str(); }

	private:
		void DataCaptureError(const std::string &desc);

	private:
		void OnReadyChannel(CmpChannelId channelId,
			int channelNumber,
			Utils::WeakRef<ICmpChannelEvent&> channelOwner,
			Utils::WeakRef<iLogW::ILogSessionCreator&> logCreator);
	
	private:
		Utils::WeakRef<ICmpConnection&> SelfRef();

	private:
		SBProto::SbpSettings m_sbpSettings;
		boost::shared_ptr<SBProtoExt::SbpLogSettings> m_sbpLogSettings;
		iCore::MsgTimer m_timer;
		ICmpConEvents &m_user;
		Utils::WeakRefHost m_selfRefHost;
		Utils::SafeRef<IpsiusService::NObjHwFinder> m_finder;

		boost::function<CmpChannelSettings (int)> m_cmpChannelSettings;

		std::string m_boardAlias;
		Utils::HostInf m_boardHost;
		
		CmpState m_state;
		
		// ChDataCapture

		ChDataCaptureMode::Value m_dataCaptureMode;
		typedef std::vector<Utils::WeakRef<IChDataCapture&> > ChDataCaptureList;
		ChDataCaptureList m_captureList;

		int m_maxDataCaptureCount;
		int m_currDataCaptureCount;

		// ---

		int  m_countActivation;
		QDateTime m_lastActivation;
		QDateTime m_lastDeactivation;
		QString m_lastBoardError;

		boost::scoped_ptr<SBProtoExt::ISbpConnection> m_sbpConnection;
		
		typedef boost::ptr_list<iCmp::IPcSideRecv> ListOfCommands;
		ListOfCommands m_commands;

		typedef std::vector<Utils::WeakRef<CmpChannel&> > CmpChannels;
		CmpChannels m_channels;

		CmpHandleManager m_confHandlers;
		CmpHandleManager m_pointHandlers;
		typedef boost::ptr_list<ICmpConfPoint> m_rtpConfCreator;

		boost::scoped_ptr<ICmpConfManager> m_cmpConfManager;

	};

};

#endif

