#include "stdafx.h"

#include "Utils/RawCRC32.h"
#include "Utils/IntToString.h"

#include "BfDev/SysProperties.h"
#include "TdmMng/TdmException.h"
#include "SafeBiProtoExt/ISbpConnection.h"
#include "Ds2155/HdlcStats.h"
#include "E1AppConfig.h"
#include "E1App/E1AppMisc.h"
#include "DrvAoz/AozBoard.h"
#include "E1App/MngLwip.h"
#include "SndMix/ConferenceMng.h"
#include "psbCofidec/PsbCofidecDrv.h"

#include "EchoSfxClient.h"
#include "E1AppBody.h"

namespace 
{

    enum
    {
        CEnableChipProtection = true,
    };

	enum
	{
		CPrintHdlcInfo          = true,
		CPrintTdmInfo           = true,
		CPrintProcessIrqInfo    = true,
		CPrintLiuInfo           = true,
	};

    enum
    {
        CTypicalRtpPackSize         = 160,

        CBidirBuffSize              = 512,  
        CBidirBuffOffset            = 32,
    };


    bool CForceTdmLinear = false; // for debug only

    // буфер вмещает типичный RTP пакет
    BOOST_STATIC_ASSERT( (CBidirBuffSize - CBidirBuffOffset) > CTypicalRtpPackSize );

    void InitTdmProfile(TdmMng::TdmManagerProfile &profile, 
        const iCmp::BfInitDataBase &data, int maxChannels, int DmaBlockCapacity)
    {
        profile.BufferingProf = data.RtpRecvProfile;
       
        profile.BidirBuffSize       = CBidirBuffSize;
        profile.BidirBuffCount      = maxChannels * (data.RtpRecvProfile.QueueDepth + 6);
        profile.BidirBuffOffset     = CBidirBuffOffset; 

        // буфер вмещает DMA block
        if (CBidirBuffSize < DmaBlockCapacity)
        {
            ESS_THROW_T(TdmMng::TdmException, TdmMng::terBadDmaBlockCapacity); 
        }
    }

    Ds2155::HdlcProfile MakeHdlcConfig()
    {
        // HDLC
        enum
        {
            CHdlcMemPoolBlockSize      = 256,
            CHdlcBlocksNum             = 16,
            CHdlcBuffOffset            = 0,
            CHdlcMaxPackSize           = 200,	        
        };

        return Ds2155::HdlcProfile(
            CHdlcMemPoolBlockSize, 
            CHdlcBlocksNum, 
            CHdlcBuffOffset, 
            CHdlcMaxPackSize);
    }

    SndMix::PointMode DecodePointMode(bool send, bool recv)
    {
        if (!(send || recv)) 
        {
            ESS_THROW_T(TdmMng::TdmException, TdmMng::terConfPointBadMode); 
        }

        if (send && recv) return SndMix::PmSendRecv;
        return send ? SndMix::PmSend : SndMix::PmRecv;
    }

};

namespace E1App
{

	E1AppBody::E1AppBody(E1BodyParams &params,  
		const iCmp::BfInitDataBase &data,
        bool enableT1Mode) :
        // iCore::MsgObject(params.Thread),
		m_cmpRecv(*this), 
		m_logSession( params.LogCreator->CreateSession("AppBody", true) ),
		m_tagInfo( m_logSession->RegisterRecordKindStr("Info") ),
		m_connection(params.Connection),
		m_pStream(0),
        m_echo(params.Echo),
        m_blockSend(false),
        m_startSendCounter(0)
	{
        ThrowFromInit(data);

		if ( m_logSession->LogActive() )
		{
			*m_logSession << m_tagInfo << "Created." << iLogW::EndRecord;
		}

		// channel set 
		TdmMng::TdmChannelsSet chSet;
		if (!chSet.Parse(data.VoiceChannels))
		{
			ESS_THROW_T(TdmMng::TdmException, TdmMng::terBadVoiceChannelString); 
		}

        // HAL profile + HAL
        {
        	BfTdm::TdmLaw tdmLaw = BfTdm::CNoCompanding;
            if (CForceTdmLinear)
            {
                tdmLaw = data.UseAlaw ? BfTdm::CAlawCompanding : BfTdm::CUlawCompanding;
            }
            BfTdm::TdmProfile sport0(data.Sport0BlocksCount, data.Sport0BlockCapacity, true, tdmLaw);
            BfTdm::TdmProfile sport1(data.Sport1BlocksCount, data.Sport1BlockCapacity, true, tdmLaw);

            if (enableT1Mode)
            {
                BfTdm::TdmProfile &p = (data.VoiceSportNum == 0) ? sport0 : sport1;
                p.T1Mode(true);
            }

            sport0.Name(params.AppConfig.AppName);
            sport1.Name(params.AppConfig.AppName);

            TdmMng::TdmHalProfile profile(params.Thread, sport0, sport1, m_logSession->LogCreator());
            m_hal.reset( new TdmMng::TdmHAL(profile) );
        }

		// TDM profile + Manager
        {
            TdmMng::TdmManagerProfile profile(params.Thread, *m_hal, m_logSession->LogCreator());
            int maxCapacity = std::max(data.Sport0BlockCapacity, data.Sport1BlockCapacity);
            InitTdmProfile(profile, data, chSet.Count(), maxCapacity);
            profile.LocalIp     = Stack::Instance().GetIP();

            if(data.StartRtpPort != 0)
            {
                profile.RtpInfraPar = 
                    iRtp::RtpInfraParams(data.StartRtpPort, data.StartRtpPort + chSet.Count() * 4);                
            }
            else
            {
                profile.RtpInfraPar = 
                    iRtp::RtpInfraParams(params.AppConfig.MinRtpPort, 
                                         params.AppConfig.MaxRtpPort);
            }

            // init
            HeapLog("on enter");

            m_tdm.reset( new TdmMng::TdmManager(profile) );
            HeapLog("TdmManager created");
        }


		// stream
        {
            m_pStream = m_tdm->RegisterStream(data.DeviceName, chSet, 
                data.VoiceSportNum, data.VoiceSportChNum, data.UseAlaw);
            ESS_ASSERT(m_pStream != 0);
            HeapLog("Stream was registered");
        }

        // echo
        {
            // chSet -> dword mask
            Platform::dword chMask = 0;
            for(int i = 0; i < chSet.Capacity(); ++i)
            {
                if (chSet.IsExist(i)) chMask |= (1 << i);
            }

            // init data
            EchoApp::CmdInitData z;
            z.DeviceName = data.DeviceName;
            z.ProtoVersion = EchoApp::CSfxProtoVersion;
            z.T1Mode = enableT1Mode;
            z.UseAlaw = data.UseAlaw;
            z.VoiceChMask = chMask;

            m_echo.CmdInit(z);
        }

	}

    // ----------------------------------

    E1AppBody::~E1AppBody()
    {
        m_echo.CmdShutdown();

        m_connection->UnbindUser();

        if(m_logSession->LogActive())
        {
            *m_logSession << m_tagInfo << "Deleted." << iLogW::EndRecord;
        }
    }

    // ----------------------------------

    void E1AppBody::SendInitResp()
    {
        using std::string;
        iCmp::BfInitInfo info;

        m_pStream->FillRtpPorts(info.RtpPorts);
        info.BuildString =  string(__DATE__) + string(" ") + string(__TIME__);
        info.FreeHeapBytes = Platform::GetHeapState().BytesFree;

        iCmp::BfRespCmdInit::Send(m_connection->Proto(), info);
    }

	// ----------------------------------

	
	void E1AppBody::Process()
	{
		ESS_ASSERT(!m_connection.IsEmpty()); 

        // poll async events from TDM
        m_tdm->SendAllEventsFromQueue( Proto(), *this );

        // poll async events from echo
        m_echo.SendAllEvents( Proto(), *this );
        
        // chip-protection
        if (m_secureChip != 0)
        {
            m_secureChip->Process();
        }

        // call overrided 
        DoProcess();
	}

    // ----------------------------------

    void E1AppBody::SendBfTdmEvent( const TdmMng::TdmAsyncEvent &e )
    {
        e.Send( Proto() );        
    }

	// ----------------------------------

	void E1AppBody::LogState() const
	{        
		if(!m_logSession->LogActive()) return;
	
		// TDM
		if (CPrintTdmInfo)
		{
			ESS_ASSERT(m_pStream != 0);

			*m_logSession << m_tagInfo 
				<< "TDM " << m_pStream->Tdm().PeekStatistic().ToString(false) << iLogW::EndRecord;
		}

		// Process IRQ
		if (CPrintProcessIrqInfo)
		{
			ESS_ASSERT(m_tdm != 0);

			*m_logSession << m_tagInfo 
				<< "TDM.IRQ " 
				<< m_pStream->Stat().ToString(BfDev::SysProperties::Instance().getFrequencySys()) 
				<< iLogW::EndRecord;                
		}

        PrintLogState();
	}

	// ----------------------------------

	void E1AppBody::ProcessCmd(Utils::SafeRef<SBProtoExt::ISbpConnection> src,
		SBProto::SbpRecvPack &data)
	{
		if (m_connection != src)
		{
			ESS_THROW_T(TdmMng::TdmException, 
				TdmMng::TdmErrorInfo(TdmMng::terAlreadyBusy, m_connection->TransportInfo()));
		}


		// redirect packet to IBfSideRecv interface
		if (!m_cmpRecv.Process(data)) 
		{
			ESS_THROW_T(TdmMng::TdmException, TdmMng::terUnknownCommand); 
		}
	}


	// ----------------------------------
	// iCmp::IBfSideRecv

    void E1AppBody::OnPcCmdStartGen(const string &devName, int chNum, bool toLine,
        const iDSP::Gen2xProfile &data)
	{
		TdmMng::ITdmChannel &ch = FindChannel(devName, chNum);

		ch.StartGen(toLine, data);

		SendRespOK();
	}

	// ----------------------------------

	void E1AppBody::OnPcCmdStopGen(const string &devName, int chNum, bool toLine)
	{
		TdmMng::ITdmChannel &ch = FindChannel(devName, chNum);

		ch.StopGen(toLine);

		SendRespOK();
	}

	// ----------------------------------


    /*
	void E1AppBody::OnPcCmdStartEchoSuppress(const string &devName, int chNum, 
		int taps, bool quickMode, bool onExtCpu)
	{
		TdmMng::ITdmChannel &ch = FindChannel(devName, chNum);

		ch.StartEchoSuppress(taps, quickMode, onExtCpu);

		SendRespOK();
	} */

    void E1AppBody::OnPcCmdChEchoSuppressMode(const string &devName, int chNum, int taps, bool useInternal)
    {
        if (useInternal)
        {
            TdmMng::ITdmChannel &ch = FindChannel(devName, chNum);
            ch.EchoSuppressMode(taps);
        }
        else
        {
            m_echo.CmdEcho(chNum, taps);
        }

        SendRespOK();
    }

    // ----------------------------------

    void E1AppBody::OnPcCmdChCaptureMode( const string &devName, int chNum, bool enabled )
    {
        TdmMng::ITdmChannel &ch = FindChannel(devName, chNum);
        ch.DataCaptureModeMode(enabled);

        SendRespOK();
    }

	// ----------------------------------

	void E1AppBody::OnPcCmdGetRtpPort(const string &devName, int chNum)
	{
		TdmMng::ITdmChannel &ch = FindChannel(devName, chNum);
		int port = ch.GetRtpPort();

		iCmp::BfRespGetRtpPort::Send(m_connection->Proto(), port);
	}

	// ----------------------------------

	void E1AppBody::OnPcCmdGetTdmInfo(const string &devName)
	{
		iCmp::BfTdmInfo info;

        if (m_pStream == 0 || m_pStream->Name() != devName)
        {
            ESS_THROW_T(TdmMng::TdmException, TdmMng::terBadDeviceName);
        }

		info.FrequencySys = BfDev::SysProperties::Instance().getFrequencySys();
		info.TdmStat = m_pStream->Tdm().PeekStatistic();
		info.TdmMngStat = m_pStream->Stat();

		iCmp::BfRespGetTdmInfo::Send(m_connection->Proto(), info);
	}

	// ----------------------------------

	void E1AppBody::OnPcCmdGetChannelInfo(const string &devName, int chNum, bool lastCall)
	{
		TdmMng::ITdmChannel &ch = FindChannel(devName, chNum);

		iCmp::BfChannelInfo info;
		if (lastCall) ch.GetLastCallStats(info.Stat);
		else ch.GetAllTimeStats(info.Stat);

		iCmp::BfRespGetChannelInfo::Send(m_connection->Proto(), info);
	}

	// ----------------------------------

	void E1AppBody::OnPcCmdStartRecv(const string &devName, int chNum)
	{
		TdmMng::ITdmChannel &ch = FindChannel(devName, chNum);
		ch.StartRecv();

		SendRespOK();
	}

	// ----------------------------------

	void E1AppBody::OnPcCmdStopRecv(const string &devName, int chNum)
	{
		TdmMng::ITdmChannel &ch = FindChannel(devName, chNum);
		ch.StopRecv();

		SendRespOK();
	}

	// ----------------------------------

	void E1AppBody::OnPcCmdStartSend(const string &devName, int chNum, 
		const std::string &ip, int port)
	{
        if (m_secureChip != 0)
        {
            if (!m_secureChip->Event(m_startSendCounter++))
            {
                m_blockSend = true;
            }
        }

        if (!m_blockSend)
        {
            TdmMng::ITdmChannel &ch = FindChannel(devName, chNum);
            ch.StartSend(ip, port);
        }

		SendRespOK();
	}

	// ----------------------------------

	void E1AppBody::OnPcCmdStopSend(const string &devName, int chNum, 
		const std::string &ip, int port)
	{

        if (!m_blockSend)
        {
            TdmMng::ITdmChannel &ch = FindChannel(devName, chNum);
            ch.StopSend(ip, port);
        }

		SendRespOK();
	}

    // ----------------------------------

    /*
    void E1AppBody::OnPcCmdLogSetup( bool logEnable, const string &udpHostPort )
    {
        // ...

        SendRespOK();
    } */ 

	// ----------------------------------

	TdmMng::ITdmChannel &E1AppBody::FindChannel(const string &devName, int chNum)
	{
        return m_tdm->Channel(devName, chNum);

        /*
		TdmMng::ITdmChannel *pCh = FindStream(devName).Channel(chNum);
		if (pCh == 0)      ESS_THROW_T(TdmMng::TdmException, TdmMng::terBadDeviceChannel); 

		return *pCh; */
	}

	// ----------------------------------

	TdmMng::TdmStream& E1AppBody::FindStream(const string &devName)
	{
        TdmMng::TdmStream *pS = m_tdm->Stream(devName, true);
        ESS_ASSERT(pS != 0);
        return *pS;

        /*
		TdmMng::TdmStream *pStream = m_tdm->Stream(devName);
		if (pStream == 0)  ESS_THROW_T(TdmMng::TdmException, TdmMng::terBadDeviceName); 
		return *pStream; */
	}

	// ----------------------------------


	void E1AppBody::HeapLog(const std::string &desc, const std::string &header, bool addHeapInfo) const
	{
		if (!m_logSession->LogActive()) return;

        *m_logSession << m_tagInfo << header << ": " << desc;

        if (addHeapInfo) 
        {
            *m_logSession << " " << Platform::GetHeapState().BytesFree << " bytes";
        }

        *m_logSession << iLogW::EndRecord;
	}

	// ----------------------------------

	void E1AppBody::SendRespOK()
	{
		iCmp::BfResponse::Send(m_connection->Proto(), TdmMng::terOK, "OK!");
	}

    // ----------------------------------

    void E1AppBody::ThrowUnsupportedCommand()
    {
        ESS_THROW_T(TdmMng::TdmException, TdmMng::terAppBodyUnsupportedCommand); 
    }

    // ----------------------------------

    void E1AppBody::StartProtection()
    {
        if (!CEnableChipProtection) return;

        ESS_ASSERT(m_secureChip == 0);

        string mac = E1App::Stack::Instance().MAC();
        Platform::dword macHash = 0;
        if (!mac.empty()) 
        {
            macHash = Utils::UpdateCRC32(&mac[0], mac.size());
        }

        m_secureChip.reset( new ChipSecure::ChipVerify(macHash) );

		if(m_logSession->LogActive())
		{
	        *m_logSession << m_tagInfo << "MAC-Hash " <<  
    	        Utils::IntToHexString(macHash) << iLogW::EndRecord;
		}

    }

    // ----------------------------------

    void E1AppBody::StopGenToLine(const std::string &devName, int chNum)
    {
        TdmMng::ITdmChannel *pCh = FindStream(devName).Channel(chNum);
        ESS_ASSERT(pCh != 0);

        pCh->StopGen(true);

        // log
        if(m_logSession->LogActive())
        {
            *m_logSession << m_tagInfo << "Stop gen on ch " << chNum << iLogW::EndRecord;
        }

    }

    // ----------------------------------

    void E1AppBody::OnPcCmdSetFreqRecvMode( const string &devName, int chMask, bool useInternal, const string &recvName, iCmp::FreqRecvMode mode, const string &params )
    {

        const int CMaxCh = sizeof(chMask) * 8;

        for(int i = 0; i < CMaxCh; ++i)
        {
            if (chMask & (1 << i))
            {
                if (useInternal)
                {
                    FindChannel(devName, i).SetFreqRecvMode(recvName, params, mode);
                }
                else
                {
                    m_echo.CmdSetFreqRecvMode(i, recvName, params, mode);
                }
            }
        }

        SendRespOK();        
    }

    // ----------------------------------

    /*
    void E1AppBody::OnPcCmdConfCreate(bool autoMode)
    {
        int confHandle = m_tdm->ConferenceMng().CreateConference(autoMode);
        iCmp::BfRespConf::Send(m_connection->Proto(), confHandle);        
    }

    void E1AppBody::OnPcCmdConfDelete(int confHandle)
    {
        m_tdm->ConferenceMng().DeleteConference(confHandle);
        SendRespOK();
    }

    void E1AppBody::OnPcCmdConfMode(int confHandle, const string &mode)
    {
        SndMix::MixMode m(mode);
        m_tdm->ConferenceMng()[confHandle].SetMode(m);
        SendRespOK();
    }

    void E1AppBody::OnPcCmdConfAddTdmPoint(int confHandle, const string &devName, int chNum, 
        bool send, bool recv)
    {
        SndMix::Conference &c = m_tdm->ConferenceMng()[confHandle];
        int pointHandle = c.AddTdmPoint( devName, chNum, DecodePointMode(send, recv) );
        iCmp::BfRespConf::Send(m_connection->Proto(), pointHandle);        
    }

    void E1AppBody::OnPcCmdConfAddRtpPoint(int confHandle, const string &codecName, 
        const string &codecParams, bool send, bool recv)
    {        
        int udpPort;
        SndMix::Conference &c = m_tdm->ConferenceMng()[confHandle];
        int pointHandle = c.AddRtpPoint( codecName, codecParams, DecodePointMode(send, recv), udpPort );
        iCmp::BfRespConf::Send(m_connection->Proto(), pointHandle, udpPort);        
    }

    void E1AppBody::OnPcCmdConfRemovePoint(int confHandle, int pointHandle)
    {
        m_tdm->ConferenceMng()[confHandle].RemovePoint(pointHandle);
        SendRespOK();        
    }

    void E1AppBody::OnPcCmdConfRtpPointSend(int confHandle, int pointHandle, const string &ip, int port)
    {
        m_tdm->ConferenceMng()[confHandle].RtpPointSend(pointHandle, ip, port);
        SendRespOK();
    } */

    // ----------------------------------

    void E1AppBody::OnPcCmdConfCreate(int confHandle, bool autoMode, int blockSize)
    {
        m_tdm->ConferenceMng().CreateConference(confHandle, autoMode, blockSize);
        SendRespOK();
    }

    void E1AppBody::OnPcCmdConfDelete(int confHandle)
    {
        m_tdm->ConferenceMng().DeleteConference(confHandle);
        SendRespOK();
    }

    void E1AppBody::OnPcCmdConfMode(int confHandle, const string &mode)
    {
        SndMix::MixMode m(mode);
        m_tdm->ConferenceMng().ConfSetMode(confHandle, m); 
        SendRespOK();
    }

    void E1AppBody::OnPcCmdConfCreateTdmPoint(int pointHandle, const string &devName, int chNum)
    {
        m_tdm->ConferenceMng().CreateTdmPoint(pointHandle, devName, chNum);
        SendRespOK();
    }

    void E1AppBody::OnPcCmdConfCreateRtpPoint(int pointHandle, const string &codecName, const string &codecParams)
    {
        int udpPort;
        m_tdm->ConferenceMng().CreateRtpPoint(pointHandle, codecName, codecParams, udpPort);
        iCmp::BfRespConf::Send(m_connection->Proto(), udpPort); 
    }

    void E1AppBody::OnPcCmdConfCreateGenPoint(int pointHandle, const iDSP::Gen2xProfile &profile)
    {
        m_tdm->ConferenceMng().CreateGenPoint(pointHandle, profile);
        SendRespOK();
    }


    void E1AppBody::OnPcCmdConfDeletePoint(int pointHandle)
    {
        m_tdm->ConferenceMng().DeletePoint(pointHandle);
        SendRespOK();
    }

    void E1AppBody::OnPcCmdConfRtpPointSend(int pointHandle, const string &ip, int port)
    {
        m_tdm->ConferenceMng().RtpPointSend(pointHandle, ip, port);
        SendRespOK();
    }

    void E1AppBody::OnPcCmdConfAddPointToConf(int pointHandle, int confHandle, bool send, bool recv)
    {
        m_tdm->ConferenceMng().AddPointToConf(pointHandle, confHandle, DecodePointMode(send, recv));
        SendRespOK();
    }

    void E1AppBody::OnPcCmdConfRemovePointFromConf(int pointHandle)
    {
        m_tdm->ConferenceMng().RemovePointFromConf(pointHandle);
        SendRespOK();
    }

    void E1AppBody::OnPcCmdConfGetState( int confHandle )
    {
        std::vector<SndMix::MixPointState> info = 
            m_tdm->ConferenceMng().ConfGetState(confHandle);

        iCmp::BfRespConf::Send(m_connection->Proto(), SndMix::MixPointState::ToString(info) ); 
    }

    // ----------------------------------

    /*
    void E1AppBody::AozEventsHook( const std::string &devName, const DrvAoz::AozEvent &e )
    {
        // hack for stop generator to line on first digit
        if (e.Name == iCmp::BfAbEvent::CLinePulseDialBegin())
        {
            int digitCount;
            ESS_ASSERT( iCmp::ChMngProtoParams::FromParams(e.Params, digitCount) );
            if (digitCount == 0) StopGenToLine(e.ChNumber);
        }

        // route events to channels
        if ( (e.Name == iCmp::BfAbEvent::CBoardState()) || 
             (e.Name == iCmp::BfAbEvent::CLineState())      )
        {
            FindStream(devName).RouteAozEvent(e);  // route to local recievers
            m_echo.RouteAozEvent(e);               // route to remote recievers
        }
        
    }

    void E1AppBody::OnSendItem( const TdmMng::EventsQueueItem &i )
    {
        // is it DTMF?
        {
            // const TdmMng::TdmAsyncEvent *pEv = 
            //    dynamic_cast<>
        }

        // is it AozEvent?
        {
            const DrvAoz::AbAozEvent *pEv = 
                dynamic_cast<const DrvAoz::AbAozEvent*>(&i);

            if (pEv != 0) AozEventsHook( pEv->DevName(), pEv->Event() );
        }
    } */

    void E1AppBody::OnAozLineEvent( const std::string &devName, int chNum, TdmMng::AozLineEvent e )
    {
        if (e == TdmMng::aleDialBegin)
        {
            StopGenToLine(devName, chNum);
            return;
        }

        // route to channels
        FindStream(devName).RouteAozEvent(chNum, e);  // route to local recievers
        m_echo.RouteAozEvent(chNum, e);               // route to remote recievers
    }

    void E1AppBody::ThrowFromInit( const iCmp::BfInitDataBase &data )
    {
        if (!data.ThrowFromInit) return;

        std::string msg = std::string("Init exception for ") + typeid(*this).name();        
        ESS_THROW_T( TdmMng::TdmException, TdmMng::TdmErrorInfo(TdmMng::terTestException, msg) );
    }

}  // namespace E1App

// ------------------------------------------------------------

namespace E1App
{

    E1AppBodyE1::E1AppBodyE1(E1BodyParams &params, 
		const iCmp::BfInitDataE1 &data) :
        E1AppBody( params, data, data.ProfileLIU.IsT1Mode() ),
        m_e1Activated(false)
    {
        ThrowFromInit(data);

        // verify LIU profile 
        if (!data.ProfileLIU.Verify())
        {
            ESS_THROW_T(TdmMng::TdmException, TdmMng::terBadLiuProfile); 
        }

        // LIU
        Ds2155::HdlcProfile hdlcConfig = MakeHdlcConfig();
        m_board.reset( 
            new Ds2155::BoardDs2155(params.AppConfig.AppName, Ds2155::CBaseAddress, data.ProfileLIU, hdlcConfig));
        HeapLog("BoardDs2155 created");

        // alloc hdlc
        if (data.HdlcCh > 0)
        {
            m_hdlc.reset( m_board->AllocHdlc(data.HdlcCh) );
            HeapLog("HDLC created");
        }

        // start protection
        StartProtection();
    }

    // ---------------------------------------------

    void E1AppBodyE1::DoProcess() 
    {
        ESS_ASSERT(m_board != 0);

        // E1 line state
        {
            Ds2155::PriStatus s = m_board->LIU().PeekStatus();
            bool active = !s.rxLOF;

            if (active != m_e1Activated)
            {
                const char *pEvent = active ? iCmp::BfTdmEvent::CE1Activated() : 
                                              iCmp::BfTdmEvent::CE1Deactivated();

                TdmMng::TdmAsyncEvent e("E1AppBodyE1", pEvent);
                SendBfTdmEvent(e);

                m_e1Activated = active;
            }
        }

        // don't poll HDLC
        if (!m_e1Activated) return;

        // hdlc recv
        m_board->HdlcPolling();

        while(true)
        {
            boost::scoped_ptr<Utils::BidirBuffer> buff(PeekHdlcPack());
            if (buff == 0) break;

            iCmp::BfHdlcRecv::Send(Proto(), buff->Front(), buff->Size());
        }
    }

    // ---------------------------------------------

    void E1AppBodyE1::PrintLogState() const
    {
        /*
        // HDLC
        if (CPrintHdlcInfo && m_hdlc != 0)
        {
            Ds2155::HdlcStatistic stat = m_hdlc->PeekStatistic();

            *m_session << m_tagInfo 
                << "HDLC " << stat.ToString(false) << iLogW::EndRecord;
        }

        // LIU
        if (CPrintLiuInfo)
        {
            ESS_ASSERT(m_board.get() != 0);

            *m_session << m_tagInfo 
                << "LIU " << m_board->LIU().PeekStatus().ToString(false) << iLogW::EndRecord;
        } */
    }

    // ---------------------------------------------

    void E1AppBodyE1::OnPcHdlcSend( /* const string &devName, */ std::vector<Platform::byte> &pack )
    {
        if (m_hdlc == 0) ESS_THROW_T(TdmMng::TdmException, TdmMng::terNoHDLC); 

        ESS_ASSERT(pack.size() != 0);
        m_hdlc->SendPack(&pack[0], pack.size());
    }

    // ---------------------------------------------

    void E1AppBodyE1::OnPcCmdGetLiuInfo( const string &devName )
    {
        FindStream(devName); // verify devName

        iCmp::BfLiuInfo info;

        // LIU
        info.LiuStatus = m_board->LIU().PeekStatus();

        // HDLC
        if (m_hdlc.get() != 0)
        {
            info.HdlcStats = m_hdlc->PeekStatistic();
        }

        iCmp::BfRespGetLiuInfo::Send(Proto(), info);
    }

    // ---------------------------------------------

    Utils::BidirBuffer *E1AppBodyE1::PeekHdlcPack()
    {
        return  (m_hdlc == 0) ? 0 : m_hdlc->GetPack();
    }

    // ---------------------------------------------

    bool E1AppBodyE1::GetDeviceState()
    {
        Ds2155::PriStatus s = m_board->LIU().PeekStatus();
        bool active = !s.rxLOF;
        return active;
    }

}  // namespace E1App

// ---------------------------------------------------------------------

namespace E1App
{

    E1AppBodyAoz::E1AppBodyAoz(E1BodyParams &params, 
        const iCmp::BfInitDataAoz &data) :
        E1AppBody(params, data, false),
		m_log( params.LogCreator->CreateSession(std::string("AOZ_") + data.DeviceName, true) )
    {
        ThrowFromInit(data);

        m_boards.Add( new DrvAoz::AozBoard(params.Thread, m_log->LogCreator(), data, HAL()));

        HAL().RegisterEventsHandler(this);
    }

    // ---------------------------------------------------------

    E1AppBodyAoz::~E1AppBodyAoz()
    {
        HAL().UnregisterEventsHandler(this);
    }

    // ---------------------------------------------------------

    void E1AppBodyAoz::OnTdmHalIrq(int sportNum, TdmMng::ITdmBlock &block)
    {
        for(int i = 0; i < m_boards.Size(); ++i)
        {
            m_boards[i]->BlockProcess(block);
            m_boards[i]->SendAllEventsFromQueue( Proto(), *this );
        }        
    }

    // ---------------------------------------------------------

    void E1AppBodyAoz::OnTdmHalBlockCollision()
    {
        for(int i = 0; i < m_boards.Size(); ++i)
        {
            m_boards[i]->BlockCollision();
        }
    }

    // ---------------------------------------------------------

    void E1AppBodyAoz::OnPcCmdAbSetCofidecSetup( const string &devName, int chMask, const iCmp::AbCofidecParams &params )
    {
        Board(devName).SetCofidecSetup(chMask, params);
        SendRespOK();
    }

    // ---------------------------------------------------------

    void E1AppBodyAoz::OnPcCmdAbSetLineParams( const string &devName, int chMask, const iCmp::AbLineParams &params )
    {
        Board(devName).SetLineParams(chMask, params);
        SendRespOK();
    }

    // ---------------------------------------------------------

    void E1AppBodyAoz::OnPcCmdAbCommand( const string &devName, int chMask, const string &cmdName, const string &cmdParams )
    {
        Board(devName).Command(chMask, cmdName, cmdParams);
        SendRespOK();        
    }

    // ---------------------------------------------------------

    DrvAoz::AozBoard& E1AppBodyAoz::Board( const string &name )
    {
        for(int i = 0; i < m_boards.Size(); ++i)
        {
            if (m_boards[i]->Name() == name) return *m_boards[i];
        }

        ESS_THROW_T(TdmMng::TdmException, TdmMng::terBadDeviceName); 
        return *m_boards[0];  // dummi
    }

    // ---------------------------------------------------------

    bool E1AppBodyAoz::GetDeviceState()
    {
        bool active = true;

        for(int i = 0; i < m_boards.Size(); ++i)
        {
            if (!m_boards[i]->Avaible())
            {
                active = false;
                break;
            }
        }

        return active;
    }

}  // namespace E1App

// ----------------------------------------------------------------

namespace E1App
{

    class E1AppBodyPult::Profiler
    {
        E1AppBodyPult &m_owner; 
        std::string m_id;

        Platform::dword m_time;

    public:

        Profiler(E1AppBodyPult &owner, const std::string &id) : 
          m_owner(owner),
          m_id(id),
          m_time( Platform::GetSystemTickCount() )
        {
        }

        ~Profiler()
        {
            std::ostringstream oss;
            
            oss << "Time " << m_id << " = " << (Platform::GetSystemTickCount() - m_time);
            if ( std::uncaught_exception() ) oss << " (in exception)";
            
            m_owner.HeapLog(oss.str(), "", false);
        }


    };

    E1AppBodyPult::E1AppBodyPult( E1BodyParams &params, const iCmp::BfInitDataPult &data ) :
        E1App::E1AppBody(params, data, false),
        m_devName(data.DeviceName),
        m_cofidecBugCapture(data.CofidecBugCapture),
        m_log( params.LogCreator->CreateSession(std::string("Pult_") + data.DeviceName, true) )
    {
        ThrowFromInit(data);

        if (!data.NoCofidec)
        {
            PsbCofidec::IPsbDebug *pDebug = 0;
            if (data.TraceCofidec) pDebug = this;

            try
            {
                Profiler p(*this, "init");
                m_drv.reset( new PsbCofidec::PsbCofidecDrv(pDebug) );    
            }
            catch(/* const */ std::exception &e)
            {
                if (m_cofidecBugCapture) ProcessCofidecError(e);
                else throw;
            }
        }

        /*
        if (data.DebugMode)
        {
            PutMsg(this, &E1AppBodyPult::StartDebugMode);
        } */
    }

    // -------------------------------------------------

    E1AppBodyPult::~E1AppBodyPult()
    {
    }

    // -------------------------------------------------

    bool E1AppBodyPult::GetDeviceState()
    {
        return true;
    }

    // -------------------------------------------------

    void E1AppBodyPult::OnPcCmdPultCofidec( const string &devName, const iPult::CofidecState &state )
    {
        if (m_cofidecBugCapture && m_lastCofidecError.size())
        {
            std::string msg = "CofidecError " + m_lastCofidecError;
            TdmMng::TdmErrorInfo err(TdmMng::terBadDeviceName, msg);
            ESS_THROW_T(TdmMng::TdmException, err);
        }

        if ((devName != m_devName) || (m_drv == 0))
        {
            std::string msg = "get " + devName + " stored " + m_devName;
            if (m_drv == 0) msg = "no cofidec!";
            TdmMng::TdmErrorInfo err(TdmMng::terBadDeviceName, msg);
            ESS_THROW_T(TdmMng::TdmException, err);
        }

        try
        {
            Profiler p(*this, "state");
            m_drv->SetState(state);
        }
        catch(/* const */ std::exception &e)
        {
            if (m_cofidecBugCapture) ProcessCofidecError(e);
            else throw;
        }

        SendRespOK();
    }

    // -------------------------------------------------

    void E1AppBodyPult::StartDebugMode()
    {
        std::string drvState = "no cofidec";

        if (m_drv) 
        {
            m_drv->SetDebugMode(true);
            drvState = m_drv->StateInfo();
        }

        iCmp::IBfSideRecv *p = this;
        p->OnPcCmdStartGen(m_devName, 0, true, iDSP::Gen2xProfile::Ready);

        HeapLog("Pult debug enabled; " + drvState);
    }

    // -------------------------------------------------

    void E1AppBodyPult::ProcessCofidecError( const std::exception &e )
    {
        m_lastCofidecError = e.what();
        HeapLog(m_lastCofidecError, "CofidecError", false);
    }

}  // namespace E1App
