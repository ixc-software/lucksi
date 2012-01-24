#include "stdafx.h"

#include "NObjDss1Interface.h"
#include "NObjDss1.h"
#include "NObjLiuProfile.h"
#include "NObjDss1BoardSettings.h"
#include "SafeBiProtoExt/NObjSbpSettings.h"
#include "gatebchannel.h"
#include "GateCallInfo.h"
#include "iCmpExt/NObjCmpConnection.h"
#include "iCmpExt/NObjCmpGlobalSetup.h"
#include "Utils/OutputFormatters.h"
#include "Utils/QtEnumResolver.h"
#include "iReg/ISysReg.h"

namespace 
{
	ISDN::SetBCannels BChannelMask(Dss1ToSip::HardType::Value hardType)
	{
		return ISDN::SetBCannels((hardType == Dss1ToSip::HardType ::E1) ? "1..31" : "0..23");
	}

	Utils::QtEnumResolvHandler<Dss1ToSip::NObjDss1Interface, Dss1ToSip::NObjDss1Interface::ConnectionState> 
		GConnectionStateResolver;

};


namespace Dss1ToSip
{
	class NObjDss1Interface::BChOrderStrategy : boost::noncopyable
	{
	public:
		BChOrderStrategy() : m_currentBChannel(-1){}
		
		ISDN::SetBCannels AllocChannels(int requestedCount, const ISDN::SetBCannels &freeChannels)
		{
			ISDN::SetBCannels res;
			int count = 0;
			int currentBChannel = m_currentBChannel;
			if(currentBChannel == -1) currentBChannel = freeChannels.MinValue();
			m_currentBChannel = -1;

			for(int i = currentBChannel + 1; i <= freeChannels.MaxValue(); ++i)
			{
				if(!freeChannels.IsExist(i)) continue;

				if(m_currentBChannel == -1) m_currentBChannel = i;
				res.Set(i);
				if(++count == requestedCount) break;
			}
			if(count == requestedCount) return res;

			for(int i = freeChannels.MinValue(); i <= currentBChannel; ++i)
			{
				if(!freeChannels.IsExist(i)) continue;

				if(m_currentBChannel == -1) m_currentBChannel = i;
				res.Set(i);
				if(++count == requestedCount) break;
			}
			return (count == requestedCount) ? res : ISDN::SetBCannels();
		}
	private:
		int m_currentBChannel;
	};
};

namespace Dss1ToSip
{
	class NObjDss1Interface::StaticSettings : boost::noncopyable
	{
	public:
		StaticSettings(Domain::NamedObject *pParent, HardType::Value hardType) :
			Board(*new NObjDss1BoardSettings(pParent, 
				"Board", boost::shared_ptr<iCmp::BfInitDataE1>(new iCmp::BfInitDataE1()))),
			Liu(*new NObjLiuProfile(pParent, hardType, "Liu")),
			Global(*new iCmpExt::NObjCmpGlobalSetup(pParent, "Global")),
			Sbp(*new SBProtoExt::NObjSbpSettings(pParent, "Sbp"))
		{}

	public:
		NObjDss1BoardSettings &Board;
		NObjLiuProfile &Liu;
		iCmpExt::NObjCmpGlobalSetup &Global;
		SBProtoExt::NObjSbpSettings &Sbp;
	};
};

// -------------------------------------------------------------------------------

namespace Dss1ToSip
{

	NObjDss1Interface::NObjDss1Interface(NObjDss1 &owner,
		const QString &aliasBoard,
		HardType::Value hardType,
		int intfNumber, int slotDChannel) :
		NamedObject(&owner.getDomain(), QString("i%1").arg(intfNumber), &owner),
		m_owner(owner),
		m_isEnabled(false),
		m_isActivatedByOwner(false),
		m_connectionState(Inactive),
		m_hardType(hardType),
		m_intfNumber(intfNumber),
		m_settings(new StaticSettings(this, m_hardType)),
		m_connection(new iCmpExt::NObjCmpConnection(this, 
			*this, 
			boost::bind(&T::GetCmpChannelSettings, this, _1),
			m_settings->Sbp.Settings(),
			m_settings->Sbp.LogSettings(),
			aliasBoard)),
		m_bChannelsObj(ISDN::SetBCannels::Capacity())
	{
		PropertyWriteEvent(this);
		InitChannelMask(slotDChannel);
		SetConnectionState(Inactive, "Created.");
		m_testMode = false;
		Enabled(true);
	}

	// -------------------------------------------------------------------------------

	NObjDss1Interface::~NObjDss1Interface()
	{
		if(!m_L1Driver.IsEmpty()) m_L1Driver.Clear()->UnlinkHardware(this);

		RegistratorLog("Deleted.");
	}

	// -------------------------------------------------------------------------------

	void NObjDss1Interface::Start()
	{
		ESS_ASSERT(!m_isActivatedByOwner);

		m_isActivatedByOwner = true;

		if(!m_isEnabled) return;

		m_isEnabled = false;
		Enable();
	}

	// -------------------------------------------------------------------------------

	void NObjDss1Interface::Stop()
	{
		ESS_ASSERT(m_isActivatedByOwner);

		m_isActivatedByOwner = false;

		if(!m_isEnabled) return;

		Disable();
		m_isEnabled = true;
	}

	// -------------------------------------------------------------------------------

	void NObjDss1Interface::Calls(DRI::ICmdOutput *pOutput, bool brief)
	{
		QStringList out;
		CallsInfoReq(out, brief);
		pOutput->Add(out);
	}

	// -------------------------------------------------------------------------------

	void NObjDss1Interface::Info(DRI::ICmdOutput *pOutput)
	{
		BoardInfo::List list;
		BoardInfoReq(list);
		pOutput->Add(BoardInfo::ToString(list));
	}

	// -------------------------------------------------------------------------------

	void NObjDss1Interface::LiuInfo(DRI::IAsyncCmd *pAsyncCmd)
	{
		m_connection->InfoReqMsg(pAsyncCmd).LiuInfo();
	}

	// -------------------------------------------------------------------------------

	void NObjDss1Interface::SaveCallInfo(int channelNumber, Utils::WeakRef<GateCallInfo&> callInfo)
	{
		m_bChannelsObj.at(channelNumber).SaveCallInfo(callInfo);
	}

	// -------------------------------------------------------------------------------

	QString NObjDss1Interface::NameBoard() const
	{
		return m_connection->NameBoard().c_str();
	}

    // -------------------------------------------------------------------------------

    bool NObjDss1Interface::IsConnectionActive() const
    {
        return m_connectionState == E1Active;
    }

	// -------------------------------------------------------------------------------

	ISDN::SharedBChansWrapper NObjDss1Interface::CaptureAnyBChannelWrap(int count, bool isOutgoing)
	{
		if(!IsConnectionActive()) return ISDN::SharedBChansWrapper();
		
		ISDN::SetBCannels freeChannels = FreeChannelsForCall(isOutgoing);
		if(freeChannels.Count() < count) return ISDN::SharedBChansWrapper();

		ISDN::SetBCannels allocated = m_chStrategy->AllocChannels(count, freeChannels);

		if(allocated.IsEmpty()) return ISDN::SharedBChansWrapper();

		m_busyChannels.Include(allocated);
		return ISDN::SharedBChansWrapper(new ISDN::BChannelsWrapper(*this, allocated));
	}

	// -------------------------------------------------------------------------------

	ISDN::SharedBChansWrapper NObjDss1Interface::CaptureBChannelWrap(const ISDN::SetBCannels &set, bool isOutgoing)
	{
		ISDN::SetBCannels freeChannels = FreeChannelsForCall(isOutgoing);
		if(!freeChannels.IsContain(set)) return ISDN::SharedBChansWrapper();

		m_busyChannels.Include(set);
		return ISDN::SharedBChansWrapper( new ISDN::BChannelsWrapper(*this, set));
	}

	// -------------------------------------------------------------------------------
	// iCmpExt::ICmpChannelCreator

	void NObjDss1Interface::CreateCmpChannel(iCmpExt::CmpChannelId channelId,
		int channelNumber,
		Utils::WeakRef<iCmpExt::ICmpChannelEvent&> channelOwner, 
		Utils::WeakRef<iLogW::ILogSessionCreator&> logCreator)
	{
		m_connection->CreateCmpChannel(channelId, channelNumber, channelOwner, logCreator);
	}

	// -------------------------------------------------------------------------------
	// ISDN::IIsdnIntf	

	int NObjDss1Interface::NumberInterface() const
	{
		return InterfaceNumber();
	}

	// -------------------------------------------------------------------------------

	ISDN::SetBCannels NObjDss1Interface::CaptureBChannel(int count)
	{
		ESS_ASSERT(!m_testMode);

		ESS_HALT("Unexpected");
		return ISDN::SetBCannels();
	}

	// -------------------------------------------------------------------------------

	bool NObjDss1Interface::CaptureBChannel(const ISDN::SetBCannels &set)
	{
		ESS_ASSERT(!m_testMode);
		ESS_HALT("Unexpected");
		return false;
	}

	// -------------------------------------------------------------------------------

	void NObjDss1Interface::FreeBChannel(const ISDN::SetBCannels &set)
	{
		ESS_ASSERT(m_busyChannels.IsContain(set));
		m_busyChannels.Exclude(set);
	}

	// -------------------------------------------------------------------------------

	ISDN::IeConstants::UserInfo NObjDss1Interface::getPayload() const
	{
		return (m_settings->Board.UseAlaw()) ? ISDN::IeConstants::G711_PCMA : ISDN::IeConstants::G711_PCMU;
	}

	// -------------------------------------------------------------------------------
	// iCmpExt::ICmpConEvents

	void NObjDss1Interface::CmpActivated(const iCmpExt::ICmpConnection *connection)
	{
		ESS_ASSERT(m_connection == connection);

		m_connectionState &= ~Inactive;

		SendGlobalSetupDev();

		SendE1Init();
	}

	// -------------------------------------------------------------------------------		

	void NObjDss1Interface::CmpDeactivated(const iCmpExt::ICmpConnection *connection, const std::string &desc)
	{
		ESS_ASSERT(m_connection == connection);

		CmpDeactivatedImpl();
	}

	// -------------------------------------------------------------------------------		

	void NObjDss1Interface::OnBfTdmEvent(const iCmpExt::ICmpConnection *connection,
		const std::string &src, const std::string &event, const std::string &params)
	{
		ESS_ASSERT(m_connection == connection);

		if(iCmp::BfTdmEvent::CE1Activated() == event)
		{
			SetConnectionState((m_connectionState & (~ E1InActive)) | E1Active, "E1 Activated.");
			if(!m_L1Driver.IsEmpty()) m_L1Driver->Activated();
		}else if(iCmp::BfTdmEvent::CE1Deactivated() == event)
		{
			SetConnectionState((m_connectionState & (~ E1Active)) | E1InActive, "E1 deactivated.");
			if(!m_L1Driver.IsEmpty()) m_L1Driver->Deactivated();
		}
	}

	// --------------------------------------------------------------------------	
	// E1

	void NObjDss1Interface::OnBfHdlcRecv(const iCmpExt::ICmpConnection *connection,
		const std::vector<Platform::byte> &pack)
	{
		ESS_ASSERT(m_connection == connection);

		if(!m_L1Driver.IsEmpty()) m_L1Driver->DataInd(pack);
	}

	// --------------------------------------------------------------------------	

	void NObjDss1Interface::OnBfAbEvent(const iCmpExt::ICmpConnection *connection,
		const std::string &deviceName, int chNumber, 
		const std::string &eventName, const std::string &eventParams)
	{
		ESS_ASSERT(m_connection == connection);

		m_connection->ResetConnection("AbEvent receive: " + eventName); 
	}

	// -------------------------------------------------------------------------------
	// IL1ToHardware

	void NObjDss1Interface::DataReq(const IHardwareToL1 *src, const QVector<Platform::byte> packet)
	{
		ESS_ASSERT(m_L1Driver.IsEqualIntf(src));

		if(!IsConnectionActive()) return;

		m_connection->SbpSender().Send<iCmp::PcHdlcSend>((void*)packet.data(), packet.size());
	}

	// -------------------------------------------------------------------------------
	// IBoardInfoReq

	void NObjDss1Interface::CallsInfoReq(QStringList &info, bool brief) const
	{
		if(!brief)
		{
			info.push_back(QString("Interface:%1; State: %2\n").
				arg(Name().Name()).
				arg(GetShortStateStr()));
		}
		if(!IsConnectionActive()) return;

		if(m_busyChannels.Size() == 0) 
		{
			info.push_back("All channels are free.");
			return;
		}

		Utils::TableOutput table("  ", ' ');
		table.AddRow("BCh", "State", "Call dur", "Talk dur", "Src", "Dst");

		int callCount = 0;
		int talkCount = 0;
		int inCallCount = 0;
		int outCallCount = 0;
		int inTalkCount = 0;
		int outTalkCount = 0;

		for(ISDN::SetBCannels::ConstIterator i = m_busyChannels.Begin(); i != m_busyChannels.End(); ++i)
		{
			GateCallInfo *callInfo = 0;
			{
				Utils::WeakRef<GateCallInfo&>  callInfoRef = 
					m_bChannelsObj.at(*i).GetCallInfo();
				
				if(!callInfoRef.Valid()) continue;
				
				callInfo = &callInfoRef.Value();
			}

			++callCount;
			if(callInfo->GetState() == GateCallState::Talk) ++talkCount;
			if(callInfo->IsIncomingCall()) 
			{
				++inCallCount;
				if(callInfo->GetState() == GateCallState::Talk) ++inTalkCount;
			}
			else
			{
				++outCallCount;
				if(callInfo->GetState() == GateCallState::Talk) ++outTalkCount;
			}
			if(brief) continue;
			QString dir((callInfo->IsIncomingCall()) ? "In  " : "Out ");
			table.AddRow(Utils::IntToString(*i).c_str(),
				dir + callInfo->StateToString(),
				callInfo->CallDuration(), callInfo->TalkDuration(),
				callInfo->SrcCall(), callInfo->DstCall());
		}

		if(!brief && callCount != 0) info << table.Get();

		info << QString("Total calls %1; Total talks: %2; In calls %3; In talks: %4; Out calls: %5; Out talks: %6\n").
			arg(callCount).arg(talkCount).
			arg(inCallCount).arg(inTalkCount).
			arg(outCallCount).arg(outTalkCount);

	}

	// -------------------------------------------------------------------------------

	void NObjDss1Interface::BoardInfoReq(BoardInfo::List &list) const
	{
		BoardInfo info;
		info.DChannel = m_owner.GetShortStateStr();
		info.BoardName = Name().Name();
		info.Alias = m_connection->NameBoard().c_str();
		info.State = GetShortStateStr();

		for(ISDN::SetBCannels::ConstIterator i = m_busyChannels.Begin(); i != m_busyChannels.End(); ++i)
		{
			Utils::WeakRef<GateCallInfo&>  callInfo = 
				m_bChannelsObj.at(*i).GetCallInfo();
			if(!callInfo.Valid()) continue;

			info.Calls++;
			if(callInfo.Value().GetState() == GateCallState::Talk) info.Talks++;
		}
		list.push_back(info);
	}

	// -------------------------------------------------------------------------------
	// Domain::IPropertyWriteEvent impl

	bool NObjDss1Interface::OnPropertyWrite(Domain::NamedObject *pObject, QString propertyName, QString val)
	{
		if(!IsBoardStarted()) return true;

		if(pObject == &m_settings->Global) 
		{
			PutMsg(this, &T::OnGlobalSetup);
			return true;
		}
		
		if(pObject == &m_settings->Board && 
			m_settings->Board.OnPropertyWrite(true, propertyName, val)) return true;		

		ThrowRuntimeException("Board already started.");
		return false;
	}

	// -------------------------------------------------------------------------------
	// property

	QString NObjDss1Interface::GetStateStr() const
	{
		return	QString("%1 | %2 | %3").
			arg((m_isActivatedByOwner) ? "ActivatedByOwner" : "DeactivatedByOwner"). 
			arg((m_isEnabled) ? "Enabled" : "Disabled").
			arg(GConnectionStateResolver.Impl().ResolveSet(m_connectionState).c_str());
	}

	// -------------------------------------------------------------------------------

	QString NObjDss1Interface::GetShortStateStr() const
	{
		if(!m_isActivatedByOwner) return "DeactivatedByOwner";
		if(!m_isEnabled) return "Disabled";	
		
		return GConnectionStateResolver.Impl().ResolveSet(m_connectionState).c_str();
	}

	// -------------------------------------------------------------------------------

	void NObjDss1Interface::Enable()
	{
		if(m_isEnabled) ThrowRuntimeException("Board already enabled.");

		m_isEnabled = true;

		if (IsLogActive(InfoTag)) Log(InfoTag) << "Enabled." << iLogW::EndRecord;

		if(!m_isActivatedByOwner) return;

		if(!m_settings->Liu.Profile().Verify()) ThrowRuntimeException("Wrong liu profile.");
		
		if(!m_settings->Board.IsValid()) ThrowRuntimeException("Wrong device settings.");

		if (IsLogActive(InfoTag)) Log(InfoTag) << "Started." << iLogW::EndRecord;

		
		for(int i = 0; i != m_bChannelsObj.size(); ++i)
		{
			if(m_bChannels.IsExist(i))
				m_bChannelsObj.at(i) = GateBChannel(i);
			else
				m_bChannelsObj.at(i) = GateBChannel();
		}

		m_chStrategy.reset(new BChOrderStrategy);

		ESS_ASSERT(m_L1Driver.IsEmpty());
		m_L1Driver = m_owner.HardwareToL1(InterfaceNumber());  
		if(!m_L1Driver.IsEmpty()) m_L1Driver->LinkHardware(this);
		m_connection->Enable(true);
	}

	// -------------------------------------------------------------------------------

	void NObjDss1Interface::Disable()
	{
		if(!m_isEnabled) ThrowRuntimeException("Board already disabled.");

		if(!m_L1Driver.IsEmpty()) m_L1Driver.Clear()->UnlinkHardware(this);

		for(int i = 0; i != m_bChannelsObj.size(); ++i)
		{
			m_bChannelsObj.at(i) = GateBChannel();
		}

		m_chStrategy.reset();
		m_isEnabled = false;
		SetConnectionState(Inactive, "Disabled.");
		m_connection->Enable(false);
	}

	// -------------------------------------------------------------------------------

	NObjDss1Interface::StrategyType NObjDss1Interface::GetStrategyType() const
	{
		return Lifo;
	}

	// -------------------------------------------------------------------------------

	void NObjDss1Interface::SetStrategyType(NObjDss1Interface::StrategyType type)
	{
		ESS_UNIMPLEMENTED;
	}

	// -------------------------------------------------------------------------------
	
	void NObjDss1Interface::InitChannelMask(int dChannel)
	{		
		ISDN::SetBCannels bChannels = BChannelMask(m_hardType);

		if(dChannel != Dss1Def::CDChannelNPos)
		{
			if(!bChannels.IsExist(dChannel)) ThrowRuntimeException("Wrong DChannel slot.");
			bChannels.Clear(dChannel);
		}
		
		m_bChannels = bChannels;
		m_slotDChannel = dChannel;
	} 

	// -------------------------------------------------------------------------------

	QString  NObjDss1Interface::BChannels() const
	{
		return m_bChannels.ToString().c_str();
	}

	// -------------------------------------------------------------------------------

	QString  NObjDss1Interface::FreeOutCh() const
	{
		ISDN::SetBCannels res(m_bChannels);
		res.Exclude(m_busyChannels);
		res.Exclude(m_outBlocking);
		return res.ToString().c_str();
	}

	// -------------------------------------------------------------------------------

	QString  NObjDss1Interface::FreeInCh() const
	{
		ISDN::SetBCannels res(m_bChannels);
		res.Exclude(m_busyChannels);
		res.Exclude(m_inBlocking);
		return res.ToString().c_str();
	}

	// -------------------------------------------------------------------------------

	QString  NObjDss1Interface::BusyCh() const
	{
		return m_busyChannels.ToString().c_str();
	}

	// -------------------------------------------------------------------------------

	QString  NObjDss1Interface::OutBlockingCh() const
	{
		return m_outBlocking.ToString().c_str();
	}

	// -------------------------------------------------------------------------------

	void NObjDss1Interface::OutBlockingCh(QString par)
	{
		ISDN::SetBCannels res(par.toStdString());
		m_outBlocking = res;
	}

	// -------------------------------------------------------------------------------

	QString NObjDss1Interface::InBlockingCh() const
	{
		return m_inBlocking.ToString().c_str();
	}

	// -------------------------------------------------------------------------------

	void NObjDss1Interface::InBlockingCh(QString par)
	{
		ISDN::SetBCannels res(par.toStdString());
		m_inBlocking = res;
	}

	// --------------------------------------------------------------------------	

	iCmpExt::CmpChannelSettings NObjDss1Interface::GetCmpChannelSettings(int channel) const
	{
		iCmpExt::CmpChannelSettings settings;
		settings.ChannelRtpAddr = (!m_bChannels.IsExist(channel)) ?
			Utils::HostInf() :
			m_bChannelsObj.at(channel).LocalRtp();

		settings.UseAlaw = m_settings->Board.UseAlaw();
		settings.EchoTape = m_settings->Board.EchoTape();
		settings.UseInternalEcho = m_settings->Board.UseInternalEcho();

		return settings;
	}

	// -------------------------------------------------------------------------------

	void NObjDss1Interface::SendGlobalSetupDev()
	{
		SetConnectionState(m_connectionState | WaitGlobalSetupResp);
		
		m_connection->BoardMsg().GlobalSetup(
			boost::bind(&T::GlobalSetupResponseOk, this),
			m_settings->Global.GlobalSetupInf());
	}

	// -------------------------------------------------------------------------------

	void NObjDss1Interface::GlobalSetupResponseOk()
	{
		SetConnectionState(m_connectionState & (~ WaitGlobalSetupResp));
	}

	// -------------------------------------------------------------------------------

	void NObjDss1Interface::SendE1Init()
	{
		SetConnectionState(m_connectionState | WaitE1InitResp);

		m_connection->BoardMsg().BoardInit(boost::bind(&T::E1InitResponse, this, _1)).
			Send<iCmp::PcCmdInitE1>(GetInitData());
	}

	// -------------------------------------------------------------------------------

	void NObjDss1Interface::E1InitResponse(const iCmp::BfInitInfo &val)
	{
		if (Log().LogActive()) 
		{
			if (IsLogActive(InfoTag)) Log(InfoTag) << "Connected. Board info: " 
				<< val.BuildString
				<< " FreeHeapBytes: " << val.FreeHeapBytes
				<< iLogW::EndRecord;
		}

		SetConnectionState((m_connectionState & (~ WaitE1InitResp)) | E1InActive, "Init completed.");

		int indexBlacfinPort = 0;
		for(int i = 0; i < m_bChannelsObj.size(); ++i)
		{
			if(!m_bChannelsObj.at(i).IsValid()) continue;

			Utils::HostInf channelRtp(m_connection->BoardHost().Address(), 
				val.RtpPorts[indexBlacfinPort++]);

			m_bChannelsObj.at(i).CmpActivated(channelRtp);
		}

		ESS_ASSERT(val.RtpPorts.size() == indexBlacfinPort);
	}

	// -------------------------------------------------------------------------------

	void NObjDss1Interface::CmpDeactivatedImpl()
	{
		SetConnectionState(Inactive, "Cmp deactivated.");

		if(!m_L1Driver.IsEmpty()) m_L1Driver->Deactivated();

		for(int i = 0; i < m_bChannelsObj.size(); ++i)
		{
			m_bChannelsObj.at(i).CmpDeactivated();
		}

	}

	// -------------------------------------------------------------------------------

	void NObjDss1Interface::ResetConnection(const std::string &msg)
	{
		PutMsg(this, &T::OnReset, msg);
	}

	// -------------------------------------------------------------------------------

	iCmp::BfInitDataE1 NObjDss1Interface::GetInitData() const
	{
		iCmp::BfInitDataE1 init;
		m_settings->Board.InitSettings(init);
		init.DeviceName = m_connection->NameBoard();
		init.ProfileLIU = m_settings->Liu.Profile();
		init.VoiceChannels = m_bChannels.ToString();
		init.HdlcCh = DChannel();
		ESS_ASSERT(init.IsValid());
		return init;
	}

	// -------------------------------------------------------------------------------

	void NObjDss1Interface::OnReset(const std::string &msg)
	{
		CmpDeactivatedImpl();
		m_connection->ResetConnection(msg);
	}

	// -------------------------------------------------------------------------------

	void NObjDss1Interface::SetConnectionState(int state, const std::string &comment)
	{
		m_connectionState = state;

		RegistratorLog("State: (" + GetStateStr() + ") "+ comment.c_str());
	}

	// -------------------------------------------------------------------------------

	void NObjDss1Interface::OnGlobalSetup()
	{
		if(IsBoardStarted()) SendGlobalSetupDev();
	}

	// -------------------------------------------------------------------------------
		
	ISDN::SetBCannels NObjDss1Interface::FreeChannelsForCall(bool isOutgoing) const
	{
		ISDN::SetBCannels bChannels(m_bChannels);
		bChannels.Exclude(m_busyChannels);
		bChannels.Exclude((isOutgoing) ? m_outBlocking : m_inBlocking);
		return bChannels;
	}

	// -------------------------------------------------------------------------------------

	void NObjDss1Interface::RegistratorLog(const QString &str)
	{
		if (IsLogActive(InfoTag))
		{
			Log(InfoTag) << str.toStdString() << iLogW::EndRecord;
		}

		getDomain().SysReg().EventInfo(Name(), str);
	}


};


