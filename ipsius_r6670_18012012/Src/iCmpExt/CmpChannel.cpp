
#include "stdafx.h"
#include "CmpChannel.h"
#include "CmpRespReceiver.h"
#include "ChDataCapture.h"
#include "Utils/IntToString.h"

namespace iCmpExt
{

	class CmpChannel::ResponseReceiver : public CmpRespReceiver
	{
	public:
		ResponseReceiver(Utils::WeakRef<CmpChannel&> owner, 
			ICmpConnection &unexpectedMsgHandler) : 
			CmpRespReceiver(unexpectedMsgHandler),
			m_owner(owner)
		{}

	// iCmpExt::CmpRespReceiver
	private:
		void OnBfResponse(int err, const std::string &desc)
		{
			if(m_owner.Valid() && err != 0) m_owner.Value().ErrorOccur(iCmp::BfResponse::ResponseToString(err, desc));
		}

	private:
		Utils::WeakRef<CmpChannel&> m_owner;

	};

};

namespace iCmpExt
{
	CmpChannel::CmpChannel(CmpChannelId channelId,
		const std::string &boardName,
		int channelNumber,
		Utils::WeakRef<ICmpConnection&> cmpConnection,
		const CmpChannelSettings &settings,
		ICmpChannelEvent &user,
		iLogW::ILogSessionCreator &logCreator) :
		m_channelId(channelId),
		m_boardName(boardName),
		m_channelNumber(channelNumber),
		m_cmpConnection(cmpConnection),
		m_settings(settings),
		m_user(user),
		m_log(logCreator, m_boardName + "_CmpCh_" + Utils::IntToString(channelNumber))
	{
		m_isReceiveOn = false;
		m_isToneSendToLine = false;
		m_isToneSendFromLine = false;
		m_isRingOn = false;
		
		ESS_ASSERT(ChannelRtpAddr().IsValid());

		if(m_log.LogActive()) m_log.Log(
			"Created. Board: " +  m_boardName + " Channel: " + Utils::IntToString(channelNumber));
		m_user.ChannelCreated(this, m_channelId);
	}

	// ----------------------------------------------------------------------------------

	CmpChannel::~CmpChannel()
	{
		StopAll();

		if(m_log.LogActive()) m_log.Log("Deleted.");
	}

	// ----------------------------------------------------------------------------------

	Utils::WeakRef<CmpChannel&> CmpChannel::SelfRef()
	{
		return m_selfRefHost.Create<CmpChannel&>(*this);
	}

	// ----------------------------------------------------------------------------------
	// ICmpChannelEvent

	void CmpChannel::DtmfEvent(int channelNumber, const std::string &event)
	{
		ESS_ASSERT(channelNumber == m_channelNumber);

		m_user.DtmfEvent(this, event);
	}

	// ----------------------------------------------------------------------------------

	void CmpChannel::DeleteChannel(int channelNumber, const std::string &desc)
	{
		ESS_ASSERT(m_channelNumber == channelNumber);

		m_user.ChannelDeleted(m_channelId, desc);
	}

	// ----------------------------------------------------------------------------------
	// ICmpChannel

	void CmpChannel::StartRecv()
	{
		if(m_isReceiveOn) return;
		
		m_isReceiveOn = true;

		StopTone(true);
		
		if(m_log.LogActive()) m_log.Log("StartRecv.");

		Sender().Send<iCmp::PcCmdStartRecv>(m_boardName, m_channelNumber);
	}

	// ----------------------------------------------------------------------------------

	void CmpChannel::StopRecv()
	{
		if(!m_isReceiveOn) return;
		
		m_isReceiveOn = false;

		if(m_log.LogActive()) m_log.Log("StopRecv.");
		
		Sender().Send<iCmp::PcCmdStopRecv>(m_boardName, m_channelNumber);
	}

	// ----------------------------------------------------------------------------------

	void CmpChannel::StartSend(const Utils::HostInf &remoteRtp)
	{
		if(m_log.LogActive()) m_log.Log("StartSend. " + remoteRtp.ToString());

		StopTone(false);

		if(m_remoteRtp.IsValid()) StopSend(m_remoteRtp);
		m_remoteRtp = remoteRtp;
		
		EchoSuppressImpl(m_settings.EchoTape, m_settings.UseInternalEcho);
		
		Sender().Send<iCmp::PcCmdStartSend>(m_boardName, m_channelNumber, remoteRtp);

		if(m_chDataCapture == 0 && !m_cmpConnection.Empty()) 
		{
			m_chDataCapture.reset(m_cmpConnection.Value().CreateChDataCapture(m_channelNumber, m_log.Log().NameStr()));
		}
	}

	// ----------------------------------------------------------------------------------

	void CmpChannel::StopSend(const Utils::HostInf &remoteRtp)
	{
		ESS_ASSERT(m_remoteRtp ==remoteRtp);
		
		StopSend();
	}

	// ----------------------------------------------------------------------------------

	void CmpChannel::StopSend()
	{
		if(!m_remoteRtp.IsValid()) return;

		if(m_log.LogActive()) m_log.Log("StopSend.");

		EchoSuppressImpl(0, false);

		Sender().Send<iCmp::PcCmdStopSend>(m_boardName, m_channelNumber, m_remoteRtp);

		m_remoteRtp.Clear();
	}

	// ----------------------------------------------------------------------------------

	void CmpChannel::SendTone(const iDSP::Gen2xProfile &tone, bool toLine)
	{
		if(toLine) 
		{
			m_isToneSendToLine = true;
			StopRecv();
		}
		else
		{
			m_isToneSendFromLine = true;
			StopSend();
		}

		if(m_log.LogActive()) 
		{
			m_log.Log() << "SendTone. " 
				<< ((toLine) ? "to line: " : "from line: ")
				<< tone.ToString() 
				<< iLogW::EndRecord;
		}

		Sender().Send<iCmp::PcCmdStartGen>(m_boardName, m_channelNumber, toLine, tone);
	}

	// ----------------------------------------------------------------------------------

	void CmpChannel::StopTone(bool toLine)
	{

		bool &dir = (toLine) ? m_isToneSendToLine : m_isToneSendFromLine;
		
		if(!dir) return;

		dir = false;

		if(m_log.LogActive()) m_log.Log("StopTone.");

		Sender().Send<iCmp::PcCmdStopGen>(m_boardName, m_channelNumber, toLine);
	}

	// ----------------------------------------------------------------------------------

	void CmpChannel::StartRing(int pulseTime, int silenceTime)
	{
		m_isRingOn = true;
		
		if(m_log.LogActive()) m_log.Log("StartRing.");

		Sender().Send<iCmp::PcCmdAbCommand>(m_boardName, 1 << m_channelNumber, iCmp::PcCmdAbCommand::CStartRing(), 				
			iCmp::ChMngProtoParams::ToParams(
			pulseTime, 
			silenceTime));
	}

	// ----------------------------------------------------------------------------------

	void CmpChannel::StopRing()
	{
		if(!m_isRingOn) return;
		m_isRingOn = false;
		
		if(m_log.LogActive()) m_log.Log("StopRing.");

		Sender().Send<iCmp::PcCmdAbCommand>(m_boardName, 1 << m_channelNumber, iCmp::PcCmdAbCommand::CStopRing());
	}

	// ----------------------------------------------------------------------------------

	void CmpChannel::EchoSuppress(int echoTape, bool useInternal)
	{
		EchoSuppressImpl(echoTape, useInternal);

		m_settings.EchoTape = echoTape;
		m_settings.UseInternalEcho = useInternal;
	}

	// ----------------------------------------------------------------------------------

	void CmpChannel::DtmfDetect(bool useInternal, iCmp::FreqRecvMode mode)
	{
		Sender().Send<iCmp::PcCmdSetFreqRecvMode>(m_boardName, 1 << m_channelNumber, useInternal, 
			iCmp::PcCmdSetFreqRecvMode::CRecvDTMF(), mode);
	}

	// ----------------------------------------------------------------------------------

	void CmpChannel::ErrorOccur(const std::string &data)
	{
		m_user.ChannelDeleted(m_channelId, data);
	}

	// ----------------------------------------------------------------------------------

	SBProtoExt::SbpMsgSender CmpChannel::Sender()
	{
		if(m_cmpConnection.Empty()) return SBProtoExt::SbpMsgSender();

		return m_cmpConnection.Value().SbpSender(
			new ResponseReceiver(m_selfRefHost.Create<CmpChannel&>(*this), m_cmpConnection.Value()));
	}

	// ----------------------------------------------------------------------------------

	void CmpChannel::StopAll()
	{
		StopSend();
		StopTone(true);
		StopTone(false);
		StopRing();
		StopRecv();
	}

	// ----------------------------------------------------------------------------------

	void CmpChannel::EchoSuppressImpl(int echoTape, bool useInternal)
	{
		if(m_log.LogActive()) 
		{
			m_log.Log() << "EchoSuppress. " 
				<< m_settings.EchoTape 
				<< ((m_settings.UseInternalEcho) ? " Internal" : " External")
				<< iLogW::EndRecord;
		}

		Sender().Send<iCmp::PcCmdChEchoSuppressMode>(
			m_boardName, m_channelNumber, m_settings.EchoTape, m_settings.UseInternalEcho);
	}
}

