#include "stdafx.h"

#include "NObjCmpConnection.h"
#include "CmpChannel.h"
#include "CmpConfManager.h"
#include "IpsiusService/NObjHwFinder.h"
#include "SafeBiProto/SbpTcpTransportFactory.h"
#include "SafeBiProtoExt/SbpConnection.h" 
#include "SafeBiProto/SbpSettings.h"
#include "SafeBiProtoExt/SbpLogSettings.h"
#include "Utils/DateTimeCapture.h"
#include "Utils/FullEnumResolver.h"


namespace 
{
	class DataCaptureRespReceiver : public iCmpExt::CmpRespReceiver
	{
	public:
		DataCaptureRespReceiver(boost::function<void (const std::string &)> errorHandler,
			iCmpExt::ICmpConnection &unexpectedMsgHandler) : 
			CmpRespReceiver(unexpectedMsgHandler),
			m_errorHandler(errorHandler)
		{}

	// iCmpExt::CmpRespReceiver
	private:
		void OnBfResponse(int err, const std::string &desc)
		{
			if(err != 0) m_errorHandler(iCmp::BfResponse::ResponseToString(err, desc));
		}

	private:
		boost::function<void (const std::string &)> m_errorHandler;
	};
};

// -------------------------------------------------------------------------------

namespace iCmpExt
{
    using namespace SBProtoExt;

	NObjCmpConnection::NObjCmpConnection(Domain::NamedObject *pParent, 
		ICmpConEvents &user,
		boost::function<CmpChannelSettings (int)> cmpChannelSettings,
//		const ICmpChannelSettings *cmpChannelSettings,
		const SBProto::SbpSettings &sbpSettings,
		boost::shared_ptr<SBProtoExt::SbpLogSettings> sbpLogSettings,
		const QString &boardAlias) :
		NamedObject(&pParent->getDomain(), "Cmp", pParent),
		m_sbpSettings(sbpSettings),
		m_sbpLogSettings(sbpLogSettings),
		m_timer(this, &T::OnProtoProcess),
		m_user(user),
		m_cmpChannelSettings(cmpChannelSettings),
		m_boardAlias(boardAlias.toStdString()),
		m_state(Disabled),
		m_dataCaptureMode(ChDataCaptureMode::None),
		m_maxDataCaptureCount(5),
		m_currDataCaptureCount(0),
		m_countActivation(0),
		m_cmpConfManager(new CmpConfManager(SelfRef()))
	{
		m_finder = dynamic_cast<IpsiusService::NObjHwFinder*>(
            getDomain().FindFromRoot(Domain::ObjectName(IpsiusService::NObjHwFinder::DriName())));

		if(m_finder.IsEmpty()) ThrowRuntimeException("HwFinder not found.");

	}

	// -------------------------------------------------------------------------------

	NObjCmpConnection::~NObjCmpConnection()
	{
		if (IsLogActive(InfoTag)) Log(InfoTag) << "Deleted." << iLogW::EndRecord;

		if(m_state != Disabled) m_finder.Clear()->FreeBoard(DriNameBoard());

		for(int i = 0; i < m_channels.size(); ++i)
		{
			if(!m_channels[i].Valid()) continue;
				
			m_channels[i].Value().DeleteChannel(i, "Cmp connection deleted");
		}
	}

	// -------------------------------------------------------------------------------

	void NObjCmpConnection::Enable(bool val)
	{
		ESS_ASSERT((val && m_state == Disabled) || (!val && m_state != Disabled));

		if(val)
		{
			ChangeState(Allocating);
			m_finder->AllocBoard(DriNameBoard(), this);
		}
		else
		{
			ChangeState(Disabled);
			m_finder->FreeBoard(DriNameBoard());
			StopSbp();
		}
	}

	// -------------------------------------------------------------------------------	

	void NObjCmpConnection::ResetConnection(const std::string &reason)
	{
		ChangeState(Connecting, reason);

		PutMsg(this, &T::OnResetConnection, reason);
	}

	// -------------------------------------------------------------------------------

	void NObjCmpConnection::StateInfo(DRI::IAsyncCmd *pAsyncCmd)
	{
		InfoReqMsg(pAsyncCmd).StateInfo();
	}

	// -------------------------------------------------------------------------------

	void NObjCmpConnection::TdmInfo(DRI::IAsyncCmd *pAsyncCmd)
	{
		InfoReqMsg(pAsyncCmd).TdmInfo();
	}

	// -------------------------------------------------------------------------------

	void NObjCmpConnection::ChannelInfo(DRI::IAsyncCmd *pAsyncCmd, int chNum, bool lastCall)
	{
		InfoReqMsg(pAsyncCmd).ChannelInfo(chNum, lastCall);
	}

	// -------------------------------------------------------------------------------	

	void NObjCmpConnection::ConferenceInfo(DRI::IAsyncCmd *pAsyncCmd, int confHandle)
	{
		InfoReqMsg(pAsyncCmd).ConferenceInfo(confHandle);
	}

	// -------------------------------------------------------------------------------	

	CmpBoardMsg NObjCmpConnection::BoardMsg()
	{
		return CmpBoardMsg(*this);
	}

	// -------------------------------------------------------------------------------

	CmpAozMsg NObjCmpConnection::AozMsg()
	{
		return CmpAozMsg(*this);
	}

	// -------------------------------------------------------------------------------

	CmpInfoReqMsg NObjCmpConnection::InfoReqMsg(DRI::IAsyncCmd *pAsyncCmd)
	{
		if(m_state != Active) ThrowRuntimeException("Connection isn't active.");

		AsyncBegin(pAsyncCmd, 
			boost::bind(&T::ReqToBoardHalted, this, "Abort DRI session"));

		return CmpInfoReqMsg(*this,
			boost::bind(&T::ReqToBoardComplete, this, _1),
			boost::bind(&T::ReqToBoardIncomplete, this, _1));
	}


	// -------------------------------------------------------------------------------
	// ICmpPointCreator

	void NObjCmpConnection::CreateCmpChannel(CmpChannelId channelId,
		int channelNumber, 
		Utils::WeakRef<ICmpChannelEvent&> channelOwner,
		Utils::WeakRef<iLogW::ILogSessionCreator&> logCreator)
	{
		ESS_ASSERT(!m_cmpChannelSettings.empty());		

		if(channelOwner.Empty()) return;
		
		if(channelNumber + 1 > m_channels.size()) m_channels.resize(channelNumber +1);
		
		PutFunctor(boost::bind(&T::OnReadyChannel, this, 
			channelId, channelNumber, channelOwner, logCreator));

	}

	// -------------------------------------------------------------------------------
	// iCmpExt::ICmpManager	

	ICmpConfManager &NObjCmpConnection::ConfManager()
	{
		return *m_cmpConfManager;
	}

	// -------------------------------------------------------------------------------
	// ICmpConnection

	bool NObjCmpConnection::IsCmpActive() const
	{
		return m_state == Active;
	}

	// -------------------------------------------------------------------------------

	Utils::HostInf NObjCmpConnection::BoardHost() const
	{
		ESS_ASSERT(IsCmpActive());
		return m_boardHost;
	}

	// -------------------------------------------------------------------------------

	SBProtoExt::SbpMsgSender NObjCmpConnection::SbpSender(iCmp::IPcSideRecv *receiver)
	{
		if(!IsCmpActive() || m_sbpConnection == 0)	return SBProtoExt::SbpMsgSender();
		if(receiver != 0) m_commands.push_back(receiver);
		return SBProtoExt::SbpMsgSender(&m_sbpConnection->Proto());
	}

	// -------------------------------------------------------------------------------

	iCmpExt::ChDataCapture* NObjCmpConnection::CreateChDataCapture(int channel, const std::string &ownerDesc)
	{
		if(m_dataCaptureMode ==  iCmpExt::ChDataCaptureMode::None) return 0;

		if(m_currDataCaptureCount == m_maxDataCaptureCount) return 0;

		ChDataCaptureList::iterator empty = m_captureList.end();

		for(ChDataCaptureList::iterator i = m_captureList.begin();
			i != m_captureList.end(); ++i)
		{
			if(i->Empty()) 
			{
				empty = i;
				continue;
			}

			if(i->Value().ChannelNumber() == channel) return 0;

		}

		iCmpExt::ChDataCapture *capture = new iCmpExt::ChDataCapture(
			ownerDesc, 
			m_dataCaptureMode, 
			m_selfRefHost.Create<IChDataCaptureSender&>(*this), 
			channel);

		if(empty != m_captureList.end())
			*empty = capture->SelfRef();
		else
			m_captureList.push_back(capture->SelfRef());

		return capture;
	}

	// -------------------------------------------------------------------------------	
	// IpsiusService::IAllocBoard

	void NObjCmpConnection::BoardAllocated(QString alias, const BfBootCore::BroadcastMsg &msg)
	{
		ESS_ASSERT(DriNameBoard() == alias);

		m_boardHost = msg.CmpHost();

		PutMsg(this, &T::OnStartConnection);
	}

	// -------------------------------------------------------------------------------

	void NObjCmpConnection::BoardUnallocated(QString alias)
	{
		ESS_ASSERT(DriNameBoard() == alias);

		ConnectionDeactivatedImpl("Board unallocated");
	}

	// -------------------------------------------------------------------------------
	// SBProtoExt::ISbpConnectionEvents

	void NObjCmpConnection::CommandReceived(Utils::SafeRef<ISbpConnection> src,
		boost::shared_ptr<SBProto::SbpRecvPack> data)
	{
		ESS_ASSERT(src.IsEqualIntf(m_sbpConnection.get()));

		OnMsgReceived(data);
	}

	// -------------------------------------------------------------------------------

	void NObjCmpConnection::ResponseReceived(Utils::SafeRef<ISbpConnection> src,
		boost::shared_ptr<SBProto::SbpRecvPack> data)
	{
		ESS_ASSERT(src.IsEqualIntf(m_sbpConnection.get()));

		OnMsgReceived(data);
	}

	// -------------------------------------------------------------------------------

	void NObjCmpConnection::InfoReceived(Utils::SafeRef<ISbpConnection> src,
		boost::shared_ptr<SBProto::SbpRecvPack> data)
	{
		ESS_ASSERT(src.IsEqualIntf(m_sbpConnection.get()));

		OnMsgReceived(data);
	}

	// -------------------------------------------------------------------------------

	void NObjCmpConnection::ProtocolError(Utils::SafeRef<ISbpConnection> src,
		boost::shared_ptr<SBProto::SbpError> err)
	{
		ESS_ASSERT(src.IsEqualIntf(m_sbpConnection.get()));

		if(m_state != Connecting && m_state != Active) return;

		ProtocolErrorImpl((err == 0) ? "Unknown error" : err->ToString());
	}

	// -------------------------------------------------------------------------------

	void NObjCmpConnection::ConnectionActivated(Utils::SafeRef<ISbpConnection> src)
	{
		ESS_ASSERT(src.IsEqualIntf(m_sbpConnection.get()));
		
		ESS_ASSERT(m_state != Active);

		ESS_ASSERT(m_commands.empty());

		ChangeState(Active);

		++m_countActivation;
		m_lastActivation = QDateTime::currentDateTime();

		m_user.CmpActivated(this);		
	}

	// -------------------------------------------------------------------------------

	void NObjCmpConnection::ConnectionDeactivated(Utils::SafeRef<ISbpConnection> src,
		const std::string &err)
	{
		ESS_ASSERT(src.IsEqualIntf(m_sbpConnection.get()));
	
		ConnectionDeactivatedImpl(err);
	}

	// -------------------------------------------------------------------------------
	// iCmp::IPcSideEventRecv

	void NObjCmpConnection::OnBfTdmEvent(const std::string &src, const std::string &parEvent, const std::string &params)
	{
		std::string event;
		std::string eventDetail;
		iCmp::BfTdmEvent::ParseEvent(parEvent, event, eventDetail);
		if(eventDetail.empty() && iCmp::BfTdmEvent::CForcedStopSend() != event)
		{
			m_user.OnBfTdmEvent(this, src, event, params);
			return;
		}
		
		// channel related event
		int channelNumber = -1;
		std::string nameDev;
		if(!iCmp::BfTdmEvent::ParseSrc(src, nameDev, channelNumber))
		{
			ProtocolErrorImpl("BfTdmEvent: wrong Src - " + src);
			return;
		}

		if(channelNumber > m_channels.size()) 
		{
			ProtocolErrorImpl("BfTdmEvent: illegal channel number - " + src);
			return;
		}
		
		if(!m_channels[channelNumber].Valid()) return;

		if(iCmp::BfTdmEvent::CForcedStopSend() == event)
		{
			m_channels[channelNumber].Value().
				DeleteChannel(channelNumber, "Wrong tdm event: " + event);
			return;
		}
		if(iCmp::BfTdmEvent::CFreqRecv() == event &&
			iCmp::PcCmdSetFreqRecvMode::CRecvDTMF() == eventDetail)
		{
			m_channels[channelNumber].Value().DtmfEvent(channelNumber, params);
			return;
		}
	}

	// -------------------------------------------------------------------------------

    void NObjCmpConnection::OnBfGetChannelData(const std::string &devName, int chNum, 
        const std::vector<Platform::byte> &rx, 
        const std::vector<Platform::byte> &tx,
        const std::vector<Platform::byte> &rxFixed)
	{
		for(ChDataCaptureList::iterator i = m_captureList.begin();
			i != m_captureList.end(); ++i)
		{
			if(i->Empty() || 
				i->Value().ChannelNumber() != chNum) continue;
			
			i->Value().Event(rx, tx, rxFixed);
			break;
		}
	}

	// -------------------------------------------------------------------------------
	// E1
	void NObjCmpConnection::OnBfHdlcRecv(const std::vector<Platform::byte> &pack)
	{
		m_user.OnBfHdlcRecv(this, pack);
	}

	// -------------------------------------------------------------------------------
	// AOZ

	void NObjCmpConnection::OnBfAbEvent(const std::string &deviceName, int chNumber, 
		const std::string &eventName, const std::string &eventParams)
	{
		m_user.OnBfAbEvent(this, deviceName, chNumber, eventName, eventParams);
	}

	// end IAsyncCmpRespOwner

	// -------------------------------------------------------------------------------
	// IChDataCaptureSender

	void NObjCmpConnection::SendDataCapture(int channel, bool enable)
	{
		SbpSender(new DataCaptureRespReceiver(
			boost::bind(&T::DataCaptureError, this, _1), *this)).
			Send<iCmp::PcCmdChCaptureMode>(NameBoard(), channel, enable);

		if (enable)
			++m_currDataCaptureCount;
		else
			--m_currDataCaptureCount;		
	}


	// -------------------------------------------------------------------------------

	void NObjCmpConnection::ReqToBoardComplete(const std::string &result)
	{
		if(!AsyncActive()) return;

		AsyncOutput(result.c_str());
		AsyncComplete(true);
	}

	// -------------------------------------------------------------------------------

	void NObjCmpConnection::ReqToBoardIncomplete(const std::string &errorMsg)
	{
		if(AsyncActive()) AsyncComplete(false, errorMsg.c_str());


//		ProtocolErrorImpl(errorMsg);
	}

	// -------------------------------------------------------------------------------

	void NObjCmpConnection::ReqToBoardHalted(const std::string &desc)
	{
		if(AsyncActive()) AsyncComplete(false, desc.c_str());			
	}

	// -------------------------------------------------------------------------------

	QString NObjCmpConnection::GetState() const
	{
		return Utils::EnumResolve(m_state).c_str();
	}

	// -------------------------------------------------------------------------------

	void NObjCmpConnection::OnProtoProcess(iCore::MsgTimer *pT)
	{
		if(m_sbpConnection != 0) m_sbpConnection->Process();
	}

	// -------------------------------------------------------------------------------

	void NObjCmpConnection::OnMsgReceived(boost::shared_ptr<SBProto::SbpRecvPack> data)
	{
		if(!IsCmpActive()) return;
		
		if(iCmp::PcEventRecv(*this).Process(*data)) return;

		if(m_commands.size() != 0)
		{
			iCmp::PcRecv(m_commands.front()).Process(*data);
			m_commands.pop_front();
		}
	}

	// -------------------------------------------------------------------------------

	void NObjCmpConnection::ResetImpl(const std::string &err)
	{
		StopSbp();
		ReqToBoardHalted(err);
		m_commands.clear();
	}

	// -------------------------------------------------------------------------------

	void NObjCmpConnection::ConnectionDeactivatedImpl(const std::string &reason)
	{
		bool activeInd = (m_state == Active);
		ChangeState(Allocating, reason);
		m_boardHost.Clear();
		ResetImpl(reason);
		if(activeInd) m_user.CmpDeactivated(this, reason);
		m_finder->FreeBoard(DriNameBoard());
		m_finder->AllocBoard(DriNameBoard(), this);
	}

	// -------------------------------------------------------------------------------

	void NObjCmpConnection::ProtocolErrorImpl(const std::string &err)
	{
		ResetConnection(err);
		m_user.CmpDeactivated(this, err);
	}

	// -------------------------------------------------------------------------------

	void NObjCmpConnection::StartSbp()
	{
		ChangeState(Connecting, std::string(" Board addr: ") + m_boardHost.ToString());

		SBProtoExt::SbpConnProfile sbpProfile;
		sbpProfile.m_logCreator  = Log().LogCreator();
		sbpProfile.m_sbpSettings = m_sbpSettings;
		sbpProfile.m_sbpSettings.setMaxReceiveSize(iCmp::CPacketMaxSizeBytes); 
		sbpProfile.m_user = this;
		sbpProfile.SetTransport(SBProto::SbpTcpTransportFactory::CreateTransport(getMsgThread(), m_boardHost));
		m_sbpConnection.reset(new SbpConnection(getMsgThread(), sbpProfile, m_sbpLogSettings));
		m_sbpConnection->ActivateConnection();
		m_timer.Start(m_sbpSettings.getPoolTimeout(), true);
	}

	// -------------------------------------------------------------------------------

	void NObjCmpConnection::StopSbp()
	{
		if(m_sbpConnection == 0) return;
		
		m_timer.Stop();
		m_lastDeactivation = QDateTime::currentDateTime();
		
		m_sbpConnection->UnbindUser();
		PutMsg(this, &T::OnDeleteConnection, m_sbpConnection->Name());
	}

	// -------------------------------------------------------------------------------

	void NObjCmpConnection::OnResetConnection(const std::string &reason)
	{
		ResetImpl(reason);

		OnStartConnection();
	}

	// -------------------------------------------------------------------------------

	void NObjCmpConnection::OnStartConnection()
	{
		if(!m_boardHost.Empty() && m_sbpConnection == 0) StartSbp();
	}

	// -------------------------------------------------------------------------------

	void NObjCmpConnection::OnDeleteConnection(const std::string &nameConnection)
	{
		if(m_sbpConnection->Name() == nameConnection) m_sbpConnection.reset();
	}

	// -------------------------------------------------------------------------------

	QString NObjCmpConnection::LastActivation() const
	{
		return m_lastActivation.toString(Qt::SystemLocaleShortDate);
	}

	// -------------------------------------------------------------------------------

	QString NObjCmpConnection::LastDeactivation() const
	{
		return m_lastDeactivation.toString(Qt::SystemLocaleShortDate);
	}

	// -------------------------------------------------------------------------------

	void NObjCmpConnection::ChangeState(CmpState newState, const std::string &desc)
	{
		m_state = newState;
		
		if (IsLogActive(InfoTag))  
		{
			Log(InfoTag) << "State: " << Utils::EnumResolve(m_state);

			if(!desc.empty()) Log() << " Reason: " << desc;

			Log() << iLogW::EndRecord;
		}
	}

	// -------------------------------------------------------------------------------

	void NObjCmpConnection::DataCaptureError(const std::string &desc)
	{
		if (IsLogActive(InfoTag)) Log(InfoTag) << "Data capture error. " << desc << iLogW::EndRecord;
	}

	// -------------------------------------------------------------------------------

	void NObjCmpConnection::OnReadyChannel(CmpChannelId channelId,
		int channelNumber,
		Utils::WeakRef<ICmpChannelEvent&> channelOwner,
		Utils::WeakRef<iLogW::ILogSessionCreator&> logCreator)
	{
		
		ESS_ASSERT(!m_cmpChannelSettings.empty());

		if(!channelOwner.Valid() || !logCreator.Valid()) return;
		
		if(m_channels.at(channelNumber).Valid()) 
		{
			channelOwner.Value().ChannelDeleted(channelId, "Wrong number");
			return;	
		}
		
		CmpChannelSettings cmpChannelSettings(m_cmpChannelSettings(channelNumber));

		if(!cmpChannelSettings.ChannelRtpAddr.IsValid())
		{
			channelOwner.Value().ChannelDeleted(channelId, "Invalid Rtp info");
			return;	
		}

		CmpChannel *channel = new CmpChannel(channelId, 
			NameBoard(),
			channelNumber, 
			SelfRef(), 
			cmpChannelSettings, 
			channelOwner.Value(), 
			logCreator.Value());

		m_channels.at(channelNumber) = channel->SelfRef();
	}

	// -------------------------------------------------------------------------------

	Utils::WeakRef<ICmpConnection&> NObjCmpConnection::SelfRef()
	{
		return m_selfRefHost.Create<ICmpConnection&>(*this);
	}
}

