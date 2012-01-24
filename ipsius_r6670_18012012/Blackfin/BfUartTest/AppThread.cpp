#include "stdafx.h"

#include "Platform/Platform.h"
#include "iCore/MsgObject.h"
#include "iCore/ThreadRunner.h"
#include "iCore/MsgTimer.h"
#include "SafeBiProto/BfSbpServer.h"
#include "iLog/LogManager.h"
#include "iLog/LogManagerOutput_i.h"
#include "iCmp/ChMngProto.h"
#include "TdmMng/TdmManager.h"
#include "TdmMng/TdmException.h"
#include "Ds2155/boardds2155.h"
#include "Utils/TimerTicks.h"
#include "Utils/IntToString.h"
#include "Utils/MsDateTime.h"
#include "iVDK/CriticalRegion.h"
#include "BfDev/E1LedsAssign.h"

#include "MngLwip.h"
#include "AppThread.h"
#include "AppConfig.h"
#include "AppCpuUsage.h"

// -----------------------------------

namespace
{
    using boost::scoped_ptr;
    using std::string;
    using Utils::BidirBuffer;

    using iCore::MsgObject;
    using iCore::IThreadRunner;
    using iCore::MsgTimer;

    using iLogW::LogSettings;
    using iLogW::LogManager;
    using iLogW::LogSession;
    using iLogW::LogRecordTag;
    using iLogW::EndRecord;
    using iLogW::ILogSessionCreator;

    using TdmMng::TdmException;
    using TdmMng::TdmErrorCode;
    using TdmMng::TdmManagerProfile;
    using TdmMng::TdmManager;

    using iCmp::BfResponse;
    using iCmp::BfHdlcRecv;
    using iCmp::BfRespInit;

    using Ds2155::LiuProfile;
    using Ds2155::Hdlc;
    using Ds2155::HdlcProfile;
    using Ds2155::BoardDs2155;
    
    using namespace SBProto;

    // ------------------------------------------------------

    enum
    {
        // Info
        CPrintInfoInterval      = 30 * 1000,
        CUseCpuLoad             = true,
        CPrintHdlcInfo          = true,
        CPrintTdmInfo           = true,
        CPrintProcessIrqInfo    = true,
        CPrintLiuInfo           = true,


        // SBP related
        CPollTimeMs             = 25,
        CListenPort             = 44044,

        // Stream params
        CDevSportNum               = 0,
        CDevSportCh                = 0,

        // debug
        CCreateBodyOnStartup       = false,

    };


    // ------------------------------------------------------

    struct AppParams
    {
        AppCpuUsage &CpuThread;

        AppParams(AppCpuUsage &cpuThread) : CpuThread(cpuThread)
        {            
        }
    };

    // ------------------------------------------------------

    class AppBody : boost::noncopyable 
    {
        ILogSessionCreator &m_log;
        scoped_ptr<LogSession> m_session;
        LogRecordTag m_tagInfo;

        scoped_ptr<TdmManager>  m_tdm;     // TDM/RTP
        scoped_ptr<BoardDs2155> m_board;   // LIU/HDLC
        scoped_ptr<Hdlc>        m_hdlc;

        TdmMng::TdmStream*      m_pStream;

        TdmMng::ITdmChannel& FindChannel(const string &devName, int chNum)
        {
            TdmMng::TdmStream *pStream = m_tdm->Stream(devName);
            if (pStream == 0)  ESS_THROW_T(TdmException, TdmMng::terBadDeviceName); 

            TdmMng::ITdmChannel *pCh = pStream->Channel(chNum);
            if (pCh == 0)      ESS_THROW_T(TdmException, TdmMng::terBadDeviceChannel); 

            return *pCh;
        }

        void HeapLog(const string &desc)
        {
            *m_session << m_tagInfo << "Heap " << desc << " " 
                << heap_space_unused(0) << EndRecord;
        }


    public:

        AppBody(ILogSessionCreator &log, const TdmManagerProfile &profile, 
                const string &deviceName, int hdlcCh, 
                const TdmMng::TdmChannelsSet &chSet, bool useAlaw) :
            m_log(log), m_pStream(0)
        {
            // log
            m_session.reset( m_log.CreateLogSesion("AppBody", true) );

            m_tagInfo = m_session->RegisterRecordKind("Info");

            // init
            HeapLog("on enter");

            m_tdm.reset( new TdmManager(profile) );
            HeapLog("TdmManager created");

            m_board.reset
            ( 
                new BoardDs2155(Ds2155::CBaseAddress, LiuProfile::getDefault(), AppConfig::CreateHdlcProfile() )                 
            );
            HeapLog("BoardDs2155 created");


            // stream
            m_pStream = m_tdm->RegisterStream(deviceName, chSet, CDevSportNum, CDevSportCh, useAlaw);
            ESS_ASSERT(m_pStream != 0);
            HeapLog("Stream registred created");

            // alloc hdlc
            m_hdlc.reset( m_board->AllocHdlc(hdlcCh) );
            HeapLog("HDLC created");
        }

        void Process()
        {
            ESS_ASSERT(m_board != 0);

            // hdlc recv
            m_board->HdlcPolling();
        }

        BidirBuffer* PeekHdlcPack()
        {
            ESS_ASSERT(m_hdlc != 0);

            return m_hdlc->GetPack();
        }

        void LogInfo()
        {
            // HDLC
            if (CPrintHdlcInfo)
            {
                ESS_ASSERT(m_hdlc.get() != 0);

                Ds2155::Statistic stat = m_hdlc->PeekStatistic();

                *m_session << m_tagInfo 
                    << "HDLC " << stat.ToString(false) << EndRecord;
            }

            // LIU
            if (CPrintLiuInfo)
            {
                ESS_ASSERT(m_board.get() != 0);

                *m_session << m_tagInfo 
                    << "LIU " << m_board->LIU().PeekStatus().ToString(false) << EndRecord;
            }

            // TDM
            if (CPrintTdmInfo)
            {
                ESS_ASSERT(m_pStream != 0);

                *m_session << m_tagInfo 
                    << "TDM " << m_pStream->Tdm().PeekStatistic().ToString(false) << EndRecord;
            }

            // Process IRQ
            if (CPrintProcessIrqInfo)
            {
                ESS_ASSERT(m_tdm.get() != 0);

                *m_session << m_tagInfo 
                    << "TDM.IRQ " << m_tdm->getStat().getAsString() << EndRecord;                
            }

        }

        void HdlcSend(std::vector<byte> &pack)
        {
            ESS_ASSERT(m_hdlc != 0);
            ESS_ASSERT(pack.size() != 0);

        	m_hdlc->SendPack(&pack[0], pack.size());
        }

        void StartGen(const string &devName, int chNum,
            bool toLine, int freq0, int freq0Lev, int freq1, int freq1Lev,
            int genInterval, int silenceInterval, int duration)
        {
            TdmMng::ITdmChannel &ch = FindChannel(devName, chNum);

            ch.StartGen(toLine, freq0, freq0Lev, freq1, freq1Lev,
                        genInterval, silenceInterval, duration);
        }

        void StopGen(const string &devName, int chNum, bool toLine)
        {
            TdmMng::ITdmChannel &ch = FindChannel(devName, chNum);

            ch.StopGen(toLine);
        }


        int GetRtpPort(const string &devName, int chNum)
        {
            TdmMng::ITdmChannel &ch = FindChannel(devName, chNum);
            return ch.GetRtpPort();
        }


        void StartRecv(const string &devName, int chNum)
        {
            TdmMng::ITdmChannel &ch = FindChannel(devName, chNum);
            ch.StartRecv();
        }

        void StopRecv(const string &devName, int chNum)
        {
            TdmMng::ITdmChannel &ch = FindChannel(devName, chNum);
            ch.StopRecv();
        }

        void StartSend(const string &devName, int chNum, 
            const std::string &ip, int port, bool useTdmIn, bool useTdmOut)
        {
            TdmMng::ITdmChannel &ch = FindChannel(devName, chNum);
            ch.StartSend(ip, port, useTdmIn, useTdmOut);
        }

        void StopSend(const string &devName, int chNum, const std::string &ip, int port)
        {
            TdmMng::ITdmChannel &ch = FindChannel(devName, chNum);
            ch.StopSend(ip, port);
        }

    };

    // -----------------------------------------------------

    class Application : 
        public MsgObject,
        public ISafeBiProtoEvents,
        public ISendPackMonitor,
        public SBProto::ILog,
        public iCmp::IBfSideRecv,
        boost::noncopyable 
    { 
        AppParams m_param;
        int m_timerCycle;

        LogManager m_log;
        scoped_ptr<LogSession> m_session;
        LogRecordTag m_tagError;
        LogRecordTag m_tagInfo;
        LogRecordTag m_tagSBP;
        LogRecordTag m_tagRecvPack;
        LogRecordTag m_tagSendPack;

        iCmp::BfRecv m_cmpRecv;
        BfSbpServer m_sbp;  // TCP socket + SBP
        MsgTimer m_timer;
        bool m_established;  // Ethernet ph level
        int  m_heapOnCreateBody;
        int  m_prevFreeHeapValue;

        Utils::TimerTicks m_cpuUsagePrintTimer;
        Utils::TimerTicks m_keepAliveTimer;
        bool m_keepAliveState;

        // create after SBP Init command, destroy on connection drop
        boost::scoped_ptr<AppBody> m_body;

        void Log(const std::string &s, LogRecordTag tag)
        {
            if (!m_session->LogActive()) return; 
            m_session->Add(s, tag);
        }

        // optimisation -- no string creation if log don't active
        /*
        void Log(const char *pMsg, LogRecordTag tag)
        {
            if (!m_session->LogActive()) return; 
            m_session->Add(pMsg, tag);
        } */

        void OnPhConnect()
        {
            Log("PhConnected", m_tagInfo);
            m_sbp.OnPhConnect();
        }

        void OnPhDisconnect()
        {
            Log("PhDisconnect", m_tagInfo);
            m_sbp.OnPhDisconnect();
        }

        void DestroyBody()
        {
            if (m_body.get() == 0) return;

            m_body.reset(0);

            if (m_session->LogActive())
            {
                *m_session << m_tagInfo << "Body destroyed, heap " 
                    << (int)(heap_space_unused(0) - m_heapOnCreateBody) << EndRecord;
            }

        }

        // log and disconnect (msg not ref for iCore msg call)
        void ProtoError(std::string msg)
        {
            Log(msg, m_tagError);

            // close socket, disable SBP
            m_sbp.OnPhDisconnect();  

            // destroy AppBody
            DestroyBody();
        }

        void MsgProtoError(const std::string &msg)
        {
            PutMsg(this, &Application::ProtoError, msg);
        }
   

        void ProcessHdlcRecv()
        {
            m_body->Process();

            while(true)
            {
                scoped_ptr<BidirBuffer> buff( m_body->PeekHdlcPack() );
                if (buff.get() == 0) break;

                BfHdlcRecv::Send(m_sbp.Proto(), buff->Front(), buff->Size());
            }
        }

        void ProcessInfo()
        {
            if (!m_session->LogActive()) return;            // log inactive            
            if (CPrintInfoInterval <= 0) return;            // interval == 0
            if (!m_cpuUsagePrintTimer.TimeOut()) return;    // interval not completed

            // std::cout << "CPU " << data.CPUUsagePercent() << "%" << endl;

            int curr = heap_space_unused(0);
            int diff = m_prevFreeHeapValue - curr;
            m_prevFreeHeapValue = curr;
                                    
            *m_session << m_tagInfo << "CPU " 
                << m_param.CpuThread.CpuLoad() << "%" 
                << " Free heap " << heap_space_unused(0) 
                << " (" << Utils::IntToStringSign(diff) << ")" << EndRecord;

            if (m_body.get() != 0)
            {
                m_body->LogInfo();
            }

        }

        void LogStartup()
        {
            *m_session << m_tagInfo << "Started, build " 
                << __DATE__ << " " << __TIME__ << EndRecord;
        }


        void OnTimer(MsgTimer *pT)
        {
            m_timerCycle++;

            // keep alive
            if (m_keepAliveTimer.TimeOut())
            {                
                BfDev::BfLed::Set(CLed_MsgAlive, m_keepAliveState);
                m_keepAliveState = !m_keepAliveState;
            }

            // network poll
            {
                bool curr = LwIP::Stack::Instance().IsEstablished();

                if (curr != m_established)
                {
                    m_established = curr;

                    if (curr) OnPhConnect();
                         else OnPhDisconnect();
                }
            }

            // SBP poll
            if (m_established) m_sbp.Process();

            if (!m_sbp.Connected())
            {
                if (m_body.get() != 0) DestroyBody();
            }

            // Body poll
            if (m_body.get() != 0)
            {
                ProcessHdlcRecv();
            }

            // DataTime capture poll
            {
                Utils::MsDateTime dt;
                dt.Capture();
            }

            // CPU usage
            ProcessInfo();
        }

        void DebugCreateBody()
        {
            Log("Debug create body", m_tagInfo);

            // OnPcCmdInit(iCmp::CCmpProtoVersion, "TDM", 16, "1..15, 17..31");

            TdmMng::TdmChannelsSet chSet;
            ESS_ASSERT( chSet.Parse("1..15, 17..31") );

            DoInit("TDM", 16, chSet, true);
        }

        void CheckInitDone()
        {
            if (m_body.get() != 0) return;

            ESS_THROW_T(TdmException, TdmMng::terCommandBeforeInit); 
        }

        void SendRespOK()
        {
            iCmp::BfResponse::Send(m_sbp.Proto(), TdmMng::terOK, "OK!");
        }

        /*
        TdmManagerProfile CreateProfile()
        {
            BfTdm::TdmProfile tdmProfile(CDmaBlocksCount, CDmaBlockCapacity, false);

            TdmManagerProfile profile(tdmProfile, m_log, getMsgThread());

            {
                profile.LocalIp             = LwIP::Stack::Instance().GetIP();

                profile.RtpStartPort        = CRtpStartPort;
                profile.RtpPortCount        = CRtpPortCount;

                profile.BidirBuffSize       = CBidirBuffSize;
                profile.BidirBuffCount      = CBidirBuffCount;
                profile.BidirBuffOffset     = CBidirBuffOffset;

                // buffering
                TdmMng::RtpRecvBufferProfile buff;
                buff.MinBufferingDepth      = CRtpMinBufferingDepth;
                buff.MaxBufferingDepth      = CRtpMaxBufferingDepth;
                buff.UpScalePercent         = CRtpUpScalePercent;
                buff.QueueDepth             = CRtpQueueDepth;

                profile.BufferingProf = buff;
            }

            ESS_ASSERT( profile.IsCorrect() );

            return profile;
        } */
        
        LogSession& getLogSession()
        {
        	return *(m_session.get());
        }

        void DoInit(const string &deviceName, int hdlcCh, 
                    const TdmMng::TdmChannelsSet &chSet, bool useAlaw)
        {
            int bodySize = 0;

            try
            {
                TdmManagerProfile profile( AppConfig::CreateTdmManagerProfile( m_log, getMsgThread() ) );

                m_heapOnCreateBody = heap_space_unused(0);

                m_body.reset
                ( 
                    new AppBody(m_log, profile, deviceName, hdlcCh, chSet, useAlaw) 
                );

                bodySize = m_heapOnCreateBody - heap_space_unused(0);

            }
            catch(/*const*/ std::exception &e)
            {
                string msg = "Create AppBody error: " + string(e.what());
                ProtoError(msg);
                return;
            }

            // Log("Init done!", m_tagInfo);

            if (m_session->LogActive())
            {
                *m_session << m_tagInfo << "Init done, body size " << bodySize << EndRecord;
            }
        }

        void LogBinary(iLogW::LoggerTextStream &wr, const SBProto::Frame &frame, int maxBytes)
        {
            int len = frame.Length();
            wr << " [bin " << len << "] ";

            if (maxBytes <= 0) return;

            bool dataCuted = false;
            if (len > maxBytes)
            {
                len = maxBytes;
                dataCuted = true;
            }

            std::string dump;
            Utils::DumpToHexString(frame.DataPtr(), len, dump);

            wr << dump;

            if (dataCuted) wr << " (...)";
        }

        void OnInfoReceived(boost::shared_ptr<SbpRecvPack> data)
        {
            try
            {
                // redirect packet to IBfSideRecv interface
                if (!m_cmpRecv.Process(data)) 
                {
                    ESS_THROW_T(TdmException, TdmMng::terUnknownCommand); 
                }
            }
            catch(/*const*/ TdmException &e)
            {
                const TdmMng::TdmErrorInfo &info =  e.getProperty();
                BfResponse::Send(m_sbp.Proto(), info.Code(), info.Desc());
            }
            catch(/*const*/ std::exception &e)
            {
                string msg;
                msg += "Unexpected exception during InfoReceived: ";
                msg += e.what();
                ProtoError(msg);
            }
        }

        // ISendPackMonitor* AsPackMonitor() { return this; }        

    // ISendPackMonitor impl
    private:

        void SentPacketInfo(SbpPackInfo &data, bool isSentPack)
        {
            const int CBinaryMaxBytes = 8;

            if (!m_session->LogActive()) return;

            iLogW::LoggerTextStream wr(getLogSession(), isSentPack ? m_tagSendPack : m_tagRecvPack);

            // print cmd
            wr << data[0].AsString();

            for(int i = 1; i < data.Count(); ++i)
            {
                SBProto::TypeInfoDesc typeInfo = data[i].Type().TypeID;

                if (typeInfo == SBProto::TypeBinary) 
                {
                    LogBinary(wr, data[i], CBinaryMaxBytes);
                    continue;
                }

                if (typeInfo == SBProto::TypeString) 
                {
                    wr << " \"" << data[i].Value() << "\""; 
                    continue;
                }

                wr << " " << data[i].Value();  
            }

        }

    // IBfSideRecv impl
    private:

        void OnPcCmdInit(int protoVersion, const string &deviceName, 
            int hdlcCh, const string &voiceChannels, bool useAlaw)
        {
            if (m_body.get() != 0)
            {
                ProtoError("Re-init!");
                return;
            }

            if (protoVersion != iCmp::CCmpProtoVersion)
            {
                ProtoError("Bad CMP version!");
                return;
            }

            TdmMng::TdmChannelsSet chSet;
            if (!chSet.Parse(voiceChannels))
            {
                ESS_THROW_T(TdmException, TdmMng::terBadVoiceChannelString); 
                return;
            }

            DoInit(deviceName, hdlcCh, chSet, useAlaw);

            BfRespInit::Send(m_sbp.Proto());
        }

        void OnPcHdlcSend(std::vector<byte> &pack)
        {
            CheckInitDone();

            m_body->HdlcSend(pack);
        }


        void OnPcCmdStartGen(const string &devName, int chNum,
            bool toLine, int freq0, int freq0Lev, int freq1, int freq1Lev,
            int genInterval, int silenceInterval, int duration)
        {
            CheckInitDone();

            m_body->StartGen(devName, chNum, toLine, freq0, freq0Lev, freq1, freq1Lev,
                             genInterval, silenceInterval, duration);

            SendRespOK();
        }

        void OnPcCmdStopGen(const string &devName, int chNum, bool toLine)
        {
            CheckInitDone();

            m_body->StopGen(devName, chNum, toLine);

            SendRespOK();
        }

        void OnPcCmdGetRtpPort(const string &devName, int chNum)
        {
            CheckInitDone();

            int port = m_body->GetRtpPort(devName, chNum);
            iCmp::BfRespGetRtpPort::Send(m_sbp.Proto(), port);
        }


        void OnPcCmdStartRecv(const string &devName, int chNum)
        {
            CheckInitDone();

            m_body->StartRecv(devName, chNum);
            SendRespOK();
        }

        void OnPcCmdStopRecv(const string &devName, int chNum)
        {
            CheckInitDone();

            m_body->StopRecv(devName, chNum);
            SendRespOK();
        }

        void OnPcCmdStartSend(const string &devName, int chNum, 
            const std::string &ip, int port, bool useTdmIn, bool useTdmOut)
        {
            CheckInitDone();

            m_body->StartSend(devName, chNum, ip, port, useTdmIn, useTdmOut);
            SendRespOK();
        }

        void OnPcCmdStopSend(const string &devName, int chNum, 
            const std::string &ip, int port)
        {
            CheckInitDone();

            m_body->StopSend(devName, chNum, ip, port);
            SendRespOK();
        }

    // SBProto::ILog impl
    private:

        void LogSBP(const std::string &s)
        {
            Log(s, m_tagSBP);
        }


    // ISafeBiProtoEvents impl
    private:

        void CommandReceived(boost::shared_ptr<SbpRecvPack> data)
        {
            MsgProtoError("CommandReceived");
        }

        void ResponceReceived(boost::shared_ptr<SbpRecvPack> data)
        {
            MsgProtoError("CommandReceived");
        }

        void InfoReceived(boost::shared_ptr<SbpRecvPack> data)
        {
            PutMsg(this, &Application::OnInfoReceived, data);
        }

        void ProtocolError(boost::shared_ptr<SbpError> err)
        {
            MsgProtoError( err->ToString() );
        }

    public:

        Application(IThreadRunner &runner, AppParams &param) : 
          MsgObject(runner.getThread()),
          m_param(param),
          m_log( AppConfig::GetLogSettings() ),
          m_cmpRecv(*this),
          m_sbp(*this, this, *this, CListenPort),
          m_timer(this, &Application::OnTimer)
        {
            m_established = false;
            m_timerCycle = 0;

            // log init
            m_session.reset( m_log.CreateLogSesion("App", true) );

            m_tagError      = m_session->RegisterRecordKind("Error");
            m_tagInfo       = m_session->RegisterRecordKind("Info");
            m_tagSBP        = m_session->RegisterRecordKind("SBP");
            m_tagRecvPack   = m_session->RegisterRecordKind("SBP.Recv");
            m_tagSendPack   = m_session->RegisterRecordKind("SBP.Send");
            
            // begin polling
            m_timer.Start(CPollTimeMs, true);

            // cpu usage timer            
            if (CPrintInfoInterval > 0)
            {
                m_cpuUsagePrintTimer.Set(CPrintInfoInterval, true);
            }

            m_keepAliveTimer.Set(500, true);
            m_keepAliveState = true;
               
            // debug startup
            if (CCreateBodyOnStartup)
            {
                PutMsg(this, &Application::DebugCreateBody);
            }
            
            // log
            if (m_session->LogActive()) LogStartup();            

            m_prevFreeHeapValue = heap_space_unused(0);
        }
    };

	
}  // namespace

// -----------------------------------

void RunApplicationThread()
{
    AppCpuUsage cpuUsage(CUseCpuLoad);
    AppParams params(cpuUsage);

    // run
    iCore::ThreadRunner test(Platform::Thread::NormalPriority, 0);
    bool res = test.Run<Application>( params, 0, AppConfig::GetThreadRunnerSleepInterval() );
}
