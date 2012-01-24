#ifndef __ECHOSFXCLIENT__
#define __ECHOSFXCLIENT__

#include "Utils/TimerTicks.h"

#include "iCore/MsgObject.h"
#include "iCore/MsgThread.h"
#include "iCore/MsgTimer.h"

#include "SafeBiProto/SbpRecvPack.h"
#include "SafeBiProtoExt/SbpUartTransport.h"
#include "SafeBiProtoExt/SbpUartTransportFactory.h"
#include "SafeBiProtoExt/SbpConnProfile.h"
#include "SafeBiProtoExt/SbpConnection.h"

#include "iCmp/ChMngProto.h"
#include "TdmMng/TdmEvents.h"
#include "FreqRecv/TdmFreqRecvPool.h"
#include "FreqRecv/FreqRecvHelper.h"

#include "SfxFreqRecvControl.h"

#include "IEchoClient.h"

namespace E1App
{
    using boost::scoped_ptr;
    using boost::shared_ptr;
    using SBProtoExt::ISbpConnection;
    using SBProto::SbpRecvPack;
    using Utils::SafeRef;
    using iCmp::BfTdmEvent;

    // ---------------------------------------------------

    // client for Sfx protocol, used thru IEchoClient
    class EchoSfxClient : 
        public iCore::MsgObject, 
        public SBProtoExt::ISbpConnectionEvents,
        public EchoApp::IMainRecvSide,
        public SBProtoExt::ISpbRawDataCallback,
        public IEchoClient,
        public ISfxFreqRecvProto,
        boost::noncopyable
    {
        enum
        {
            CComPort  = 1,

            CInitRespTimeout        = 1000,

            CUpdateStateInterval    = 5000,
            CStateRespTimeout       = 1000,

            CMaxSeqFailsForBlock    = 3,
            CMaxRawErrors           = 5,
            CMaxInitRetry           = 3,
        };

        enum InitState
        {
            StInitial,
            StWaitResp,     // send MainCmdInit(), wait response with timeout
            StInitDone,     // after get response for MainCmdInit()
        };

        class StatsUpdater
        {
            const int m_updateInterval;
            ISbpConnection &m_conn;

            EchoApp::EchoAppStats m_data;

            Utils::TimerTicks m_timer;
            bool m_waitStatsResp;

            bool m_overloaded;

            // stats
            int m_reqCount, m_respCount, m_seqFails;            

        public:

            StatsUpdater(ISbpConnection &conn, int updateInterval) :
              m_updateInterval(updateInterval),
              m_conn(conn),
              m_overloaded(false), 
              m_reqCount(0), 
              m_respCount(0),
              m_seqFails(0)
            {
                m_data.IsEmpty = true;

                if (updateInterval <= 0)  // disabled
                {
                    return;
                }

                m_timer.Set(m_updateInterval);
                m_waitStatsResp = false;
            }

            // return false on response timeout
            bool Process()
            {
                // object is disabled
                if (m_updateInterval <= 0) return true;

                if (!m_timer.TimeOut()) return true;

                // response timeout
                if (m_waitStatsResp)
                {
                    m_timer.Set(m_updateInterval);
                    m_waitStatsResp = false;
                    ++m_seqFails;

                    return false;
                }

                // request stats
                ++m_reqCount;
                EchoApp::MainCmdRequestState::Send( m_conn.Proto() );

                m_timer.Set(CStateRespTimeout);
                m_waitStatsResp = true;

                return true;
            }

            // return false on unexpected response
            bool SetData(const EchoApp::EchoAppStats &data)
            {
                if (!m_waitStatsResp) return false;

                // overloaded
                if (!data.IsEmpty && !m_data.IsEmpty)
                {
                    int diff = data.BlockCollisions - m_data.BlockCollisions;
                    if ((data.BlockCollisions > 8) && (diff > 0)) m_overloaded = true;
                }

                // set data
                m_data = data;
                m_data.RecvCounter = ++m_respCount;                
                m_seqFails = 0;

                // set timer
                m_timer.Set(m_updateInterval);
                m_waitStatsResp = false;

                return true;
            }

            bool Overloaded()
            {
                bool retValue = m_overloaded;

                m_overloaded = false;

                return retValue;
            }

            int SeqFails() const { return m_seqFails; }
 
            const EchoApp::EchoAppStats& Data() const { return m_data; }
        };

        class UartErrorCollector
        {
            enum
            {
                CTotalCollectTime = 10 * 1000,   // длительно полного цикла сбора данных
                CSilenceInterval  = 200,         // min минимальное значение для распознавания "тишины" в передаче
                CMaxDataSize      = 512,
            };

            enum State
            {
                StInactive,
                StWaitOpenSilence,
                StWaitFirstData,
                StCollect,
                StCollected,
                StWaitCycleEnd,
            };

            State m_state;
            std::string m_data;

            Utils::TimerTicks m_globalTimer;   // время начала цикла
            Utils::TimerTicks m_lastDataRecv;

            void AppendData(const void *pData, size_t size)
            {
                ESS_ASSERT(pData != 0);
                ESS_ASSERT(size > 0);
                m_data.append(static_cast<const char*>(pData), size);
            }

        public:

            UartErrorCollector()
            {
                m_state = StInactive;
            }

            bool BeginCollect()
            {
                // ignore it
                if (m_state != StInactive) return false;

                // begin collect
                m_globalTimer.Set(CTotalCollectTime, false);
                m_lastDataRecv.Reset();
                m_state = StWaitOpenSilence;
                m_data.clear();

                return true;
            }

            std::string Process()
            {
                // process
                ProcessData(0, 0);

                // peek error
                std::string result;

                if (m_state == StCollected)
                {
                    result = m_data;
                    m_data.clear();
                    m_state = StWaitCycleEnd;
                }

                return result;
            }

            void ProcessData(const void *pData, size_t size)
            {
                // StInactive
                if (m_state == StInactive)
                {
                    return;
                }

                // full cycle timeout
                if (m_globalTimer.TimeOut())
                {
                    m_state = StInactive;
                    m_data.clear();
                    return;
                }

                // data 
                bool data = (pData != 0);
                if (data) 
                {
                    ESS_ASSERT(size > 0);
                    m_lastDataRecv.Reset();
                }

                // StWaitOpenSilence
                if (m_state == StWaitOpenSilence)
                {
                    if (m_lastDataRecv.Get() > CSilenceInterval) m_state = StWaitFirstData;
                    return;
                }

                // StWaitFirstData
                if (m_state == StWaitFirstData)
                {
                    if (data) 
                    {
                        m_data = "";
                        AppendData(pData, size);
                        m_state = StCollect;
                    }

                    return;
                }

                // StCollect
                if (m_state == StCollect)
                {
                    if (data) 
                    {
                        AppendData(pData, size);
                        if (m_data.size() > CMaxDataSize)
                        {
                            m_data.clear();
                            m_state = StWaitOpenSilence;
                            return;
                        }
                    }

                    if (m_lastDataRecv.Get() > CSilenceInterval)
                    {
                        m_state = StCollected;
                    }

                    return;
                }

                // else -> nothing
            }

        };

        scoped_ptr<SfxFreqRecvControl> m_freqRecvs;
        scoped_ptr<iLogW::LogSession> m_logSession;
    
        InitState m_state;
        TdmMng::EventsQueue m_events;
        Utils::TimerTicks m_respTimeout;
        EchoApp::CmdInitData m_initData; // for init retry

        scoped_ptr<ISbpConnection> m_uart;

        scoped_ptr<StatsUpdater> m_statsUpdater;


        UartErrorCollector m_errorCollector;

        bool m_blocked;  // blocked 'couse too much errors
        int m_rawErrorsQuota;
        int m_initSendCounter;

        void InitUART(Utils::SafeRef<iLogW::ILogSessionCreator> log)
        {
            // create SBP/UART transport
            SBProtoExt::SbpUartProfile prof;
            prof.BoudRate = EchoApp::CSfxProtoUartSpeed;

            std::string err;
            boost::shared_ptr<SBProto::ISbpTransport> transport =
                SBProtoExt::SbpUartTransportFactory::CreateTransport(CComPort, prof, err, true);            

            ESS_ASSERT(transport != 0);

            // create SPB connection 
            {
                SBProtoExt::SbpConnProfile prof;
                prof.m_logCreator = log;   
                prof.SetTransport( transport );
                prof.m_user = this;
                prof.m_rawDataCallback = this;

                m_uart.reset(new SBProtoExt::SbpConnection(getMsgThread(), prof));
                m_uart->ActivateConnection();
            }
        }

        void PushEvent(const std::string &e, const std::string &params = "")
        {
            PushEvent( TdmMng::TdmAsyncEvent(BfTdmEvent::SrcEcho(), e, params) );
        }

        void PushEvent(const TdmMng::TdmAsyncEvent &e)
        {
            m_events.Push(e);

            // trace
            *m_logSession << "EVENT: " << e.Event() << iLogW::EndRecord;
        }

        void ProcessStats()
        {
            if (!m_statsUpdater->Process())
            {
                if (m_statsUpdater->SeqFails() >= CMaxSeqFailsForBlock)
                {
                    m_blocked = true;
                    PushEvent( BfTdmEvent::CEchoBlocked() );
                }
                else
                {
                    PushEvent( BfTdmEvent::CEchoStatsRespTimeout() );
                }

            }

            if (m_statsUpdater->Overloaded())
            {
                PushEvent( BfTdmEvent::CEchoOverloaded() );
            }
        }

        void SendInit()
        {
            ESS_ASSERT(m_state == StWaitResp);

            m_respTimeout.Set(CInitRespTimeout, false);
            ++m_initSendCounter;

            EchoApp::MainCmdInit::Send(m_uart->Proto(), m_initData);
        }

    // ISfxFreqRecvProto impl
    private:

        void CmdSetFreqRecvMode(int chNum, const std::string &recvName, 
            const std::string &params, bool enable)
        {
            if (m_blocked) return;
            if (m_state != StInitial) return;

            EchoApp::MainCmdSetFreqRecvMode::Send(m_uart->Proto(), chNum,
                recvName, params, enable);
        }


    // SBProtoExt::ISpbRawDataCallback impl
    private:

        void OnSbpRawDataRecv(const void *pData, size_t size) 
        {
            m_errorCollector.ProcessData(pData, size);
        }

    // IMainRecvSide impl
    private:

        void OnEchoResponse(const EchoApp::EchoResp &resp)
        {
            if ((m_state == StWaitResp) && (resp.getCode() == EchoApp::EchoResp::ER_Ok))
            {
                m_state = StInitDone;
                return;
            }

            PushEvent(BfTdmEvent::CEchoBadResp(), resp.ToString());
        }

        void OnEchoStateResp( const EchoApp::EchoAppStats &stats)
        {
            if (!m_statsUpdater->SetData(stats))
            {
                PushEvent( BfTdmEvent::CEchoUnexpectedStateResp() );
            }
        }

        void OnEchoAsyncEvent(const std::string &eventName, 
            int chNum, const std::string &params)
        {
            if (m_state != StInitDone) return;
            if (m_initData.DeviceName.empty()) return;
            if (m_freqRecvs == 0) return;

            int pos = eventName.find(EchoApp::EchoAsyncEvent::CSep());
            if (pos < 0) return;

            std::string name = eventName.substr(0, pos);
            std::string recvName = eventName.substr(pos + 1);

            if (name == EchoApp::EchoAsyncEvent::CFreqRecvEvent())
            {
                m_freqRecvs->PushFreqRecvEvent(m_initData.DeviceName, chNum, recvName, params);

                // using iCmp::BfTdmEvent;
                // PushEvent( BfTdmEvent::MakeFreqRecvEvent(m_initData.DeviceName, chNum, recvName, params) );
            }
            
        }


    // SBProtoExt::ISbpConnectionEvents impl
    private:

        void CommandReceived(SafeRef<ISbpConnection> src,  shared_ptr<SbpRecvPack> data)
        {
            PushEvent( BfTdmEvent::CEchoUnexpectedPacket() );
        }

        void ResponseReceived(SafeRef<ISbpConnection> src, shared_ptr<SbpRecvPack> data)
        {
            PushEvent( BfTdmEvent::CEchoUnexpectedPacket() );
        }

        void InfoReceived(SafeRef<ISbpConnection> src, shared_ptr<SbpRecvPack> data)
        {
            using namespace EchoApp;

            ESS_ASSERT(data != 0);

            try
            {
                if (EchoResponse::ProcessPacket(*data, *this))   return;
                if (EchoStateResp::ProcessPacket(*data, *this))  return;
                if (EchoAsyncEvent::ProcessPacket(*data, *this)) return;
            }
            catch (SBProto::SpbException &e)
            {
                PushEvent( BfTdmEvent::CEchoSbpException(), e.what() );
                return;
            }

            PushEvent( BfTdmEvent::CEchoUnexpectedPacket() );
        }

        void ProtocolError(SafeRef<ISbpConnection> src, shared_ptr<SBProto::SbpError> err)
        {
            if (m_rawErrorsQuota == 0) return;

            if (m_errorCollector.BeginCollect())
            {
                PushEvent( BfTdmEvent::CEchoProtoError(), err->ToString() );
            }
        }

        void ConnectionActivated(SafeRef<ISbpConnection>) 
        { 
            // nothing
        }        

        void ConnectionDeactivated(SafeRef<ISbpConnection> src,  const std::string &errInfo)
        {
            PushEvent( BfTdmEvent::CEchoProtoDisc(), errInfo );
        }

    // IEchoClient impl
    private:

        void Process()
        {
            // recv packets
            m_uart->Process();

            // init timeout
            if (m_state == StWaitResp)
            {
                if (m_respTimeout.TimeOut())
                {
                    if (m_initSendCounter < CMaxInitRetry) 
                    {
                        SendInit();
                    }
                    else 
                    {
                        PushEvent( BfTdmEvent::CEchoInitRespTimeout() );
                        // + block echo - ?
                    }
                }
            }

            // error collector
            {
                std::string rawErrorText = m_errorCollector.Process();

                if (!rawErrorText.empty() && (m_rawErrorsQuota > 0))
                {
                    --m_rawErrorsQuota;
                    PushEvent(BfTdmEvent::CEchoRawErrorText(), rawErrorText);
                }
            }

            // stats updater
            if (!m_blocked)
            {
                ProcessStats();
            }

        }

        void CmdInit(const EchoApp::CmdInitData &data)
        {
            if (m_blocked) return;

            ESS_ASSERT(m_state == StInitial);

            m_events.Clear();
            m_state = StWaitResp;
            m_initSendCounter = 0;
            m_initData = data;

            m_freqRecvs.reset( new SfxFreqRecvControl(*this, data.VoiceChMask, m_events) );

            SendInit();
        }

        void CmdEcho(int chNum, int taps)
        {
            if (m_blocked) return;

            ESS_ASSERT(m_state != StInitial);

            EchoApp::MainCmdEcho::Send(m_uart->Proto(), chNum, taps);
        }

        void CmdHalt()
        {
            if (m_blocked) return;

            EchoApp::MainCmdRequestHalt::Send(m_uart->Proto());
        }

        void CmdShutdown()
        {
            if (m_blocked) return;

            ESS_ASSERT(m_state != StInitial);

            m_events.Clear();
            EchoApp::MainCmdShutdown::Send(m_uart->Proto());

            m_state = StInitial;
        }

        void CmdSetFreqRecvMode(int chNum, const std::string &recvName, 
            const std::string &params, iCmp::FreqRecvMode mode)
        {
            if (m_blocked) return;

            ESS_ASSERT(m_state != StInitial);
            ESS_ASSERT(m_freqRecvs != 0);
    
            // exec
            m_freqRecvs->CmdSetFreqRecvMode(chNum, recvName, params, mode);
        }


        EchoApp::EchoAppStats GetStats()
        {
            EchoApp::EchoAppStats s = m_statsUpdater->Data();
            s.Blocked = m_blocked;
            return s;
        }

        void SendAllEvents(SBProto::ISafeBiProtoForSendPack &send,
                           TdmMng::IIEventsQueueItemDispatch &hook)
        {
            m_events.SendAll(send, &hook);
        }

        void RouteAozEvent(int chNum, TdmMng::AozLineEvent e)
        {
            if (m_freqRecvs) m_freqRecvs->RouteAozEvent(chNum, e);            
        }

    public:
        
        EchoSfxClient(iCore::MsgThread &thread, 
            Utils::SafeRef<iLogW::ILogSessionCreator> log) : 
            iCore::MsgObject(thread),
            m_logSession(log->CreateSession("Echo", true)),
            m_state(StInitial),
            m_events(BfTdmEvent::SrcQueueEcho()),
            m_blocked(false),
            m_rawErrorsQuota(CMaxRawErrors),
            m_initSendCounter(0)
        {
            InitUART(log);

            m_statsUpdater.reset( new StatsUpdater(*m_uart, CUpdateStateInterval) );
        }
                        
    };
    
}  // namespace E1App

#endif
