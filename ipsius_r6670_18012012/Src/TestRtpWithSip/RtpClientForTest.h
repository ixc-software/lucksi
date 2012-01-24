#ifndef RTPCLIENTFORTEST_H
#define RTPCLIENTFORTEST_H

#include "iCore/MsgThread.h"
#include "iCore/MsgObject.h"
#include "iCore/MsgTimer.h"

//#include "CallTestRtpWithSip.h"
#include "TestRtpWithSip.h"
#include "RtpPacketStoreQueue.h"

#include "iRtp/RtpPcSession.h"
#include "iRtp/IRtpPCToUser.h"

//#include "iLog/DefaultLogFormating.h"
//#include "iLog/LogOutSettings.h"
#include "ilog/LogSessionSettings.h"
#include "iLog/LogSessionProfileSetup.h"
#include "Sip/ISipToRtp.h"

#include "Utils/ExeName.h"
//#include "TestRtpWithSip.h"
#include "TestParams.h"

#include "Utils/StatisticElement.h"
#include "Utils/QtHelpers.h"

//-----

#include "TestProfile.h"
#include "CallScenaries.h"

namespace TestRtpWithSip
{
    using Platform::dword;
    using Platform::byte;

    typedef TestRtpWithSip::RtpPacketStoreQueue<false> PackQueue;

    
    short ulaw2linear(unsigned char u_val);

    //-----------------------------------------------------------------------------------
    // QueueSender

    class IQueueSenderToOwner
    {
    public:
        virtual void SendComplete() = 0; 
    };

    class DynamycSizeSender
        : public iCore::MsgObject,
        boost::noncopyable
    {
        static const iRtp::RtpPayload CPayload = PCMU;
        static const bool CMarker = false;
        static const Platform::dword CSugPeriodMsec = 20;

        iRtp::RtpPcSession& m_rtp;
        IQueueSenderToOwner& m_owner;  
        QByteArray m_dataBuff;
        iCore::MsgTimer m_timer;  

        Platform::ddword m_prevTick;
        Utils::StatElementForInt m_stat;
        

        //Platform::ddword get

        void DoSend(iCore::MsgTimer*)
        {   
            if (m_dataBuff.size() == 0)
            {
                m_timer.Stop();                
                m_owner.SendComplete();
                std::cout << m_stat.getAsString();
                return;
            }

            //Platform::ddword currTime = Platform::GetSystemTickCount() * 1000;
            Platform::ddword currTime = Platform::GetSystemMicrosecTickCount();

            if (m_prevTick == 0)
            {
                m_prevTick = currTime;
                return;
            }

            Platform::ddword diffTime = currTime - m_prevTick;
            //ESS_ASSERT(diffTime > 0);
            if (diffTime < 0)
                diffTime = ~Platform::ddword(0) - diffTime;

            if (diffTime == 0)
                return;
            
            m_prevTick = currTime;

            int size = getByteRate() * diffTime / 1000000;

            // size correction at and
            if (size > m_dataBuff.size())
                size = m_dataBuff.size();            

            QByteArray packData = m_dataBuff.left(size);
            m_dataBuff.remove(0, size);
            
            if (packData.size() != size)
                std::cout << size << std::endl;
            
            m_rtp.TxData( CMarker, 0/*size*/, packData );  
            m_stat.AddData(diffTime);
            
        }

        int getByteRate() // byte/Sec
        {
            if (CPayload == PCMU)
                return 8000;
            
            ESS_ASSERT(0);
            return 0;
        }

    public:

        DynamycSizeSender(iCore::MsgThread& thread, iRtp::RtpPcSession& rtp, const std::string& fileName, IQueueSenderToOwner& owner)
            : iCore::MsgObject(thread),
            m_rtp(rtp),
            m_owner(owner),
            m_timer(this, &DynamycSizeSender::DoSend),
            m_prevTick(0)
        {
            QFile file(fileName.c_str());
            file.open(QIODevice::ReadOnly);
            m_dataBuff = file.readAll();
            rtp.setPayload(CPayload);
            m_timer.Start(CSugPeriodMsec, true);
        }
    };

    // отправляет пакеты из очереди в rtp, с заданным интервалом, проставляя timestamp
    class QueueSender// todo rename, use QFile
        : public iCore::MsgObject,
        boost::noncopyable
    {
        enum{ CPacketSize = 160 };

        iRtp::RtpPayload m_payload;
        bool m_marker;
        bool m_payloadAndMarkerIsSet;

        IQueueSenderToOwner& m_owner;
        iRtp::RtpPcSession& m_rtp;        
        iCore::MsgTimer m_timer;        
        std::vector<char> m_buff;
        int m_readPos;

        dword m_timestamp;

        Utils::StatElementForDDWord m_timeStat;
        Platform::ddword m_prevTick;

        
        void WriteStat()
        {
            //учесть переход через currTime 0

            Platform::ddword currTime = Platform::GetSystemMicrosecTickCount();

            if (m_prevTick == 0)
            {
                m_prevTick = currTime;
                return;
            }
            
            Platform::ddword diffTime = currTime - m_prevTick;
            ESS_ASSERT(diffTime > 0);
            m_timeStat.AddData(diffTime);
            m_prevTick = currTime;            
        }

        void DoSend(iCore::MsgTimer*)
        {         
            int packSize;
            int buffSize = m_buff.size();
            if (m_readPos + CPacketSize > buffSize)            
                packSize = buffSize - m_readPos;            
            else
                packSize = CPacketSize;

            QByteArray data = QByteArray::fromRawData( &m_buff.at(m_readPos), packSize );
            m_readPos += packSize;
            m_timestamp += CPacketSize;
            
            WriteStat();
            m_rtp.TxData( m_marker, m_timestamp, data );

            if (m_readPos >= buffSize) // send complete
            {
                m_timer.Stop();
                std::cout << m_timeStat.getAsString();
                m_owner.SendComplete();
            }
            
        }

        void Save(const PackQueue::Packet& pack)
        {
            if (!m_payloadAndMarkerIsSet)
            {
                m_marker = pack.Marker;
                m_payload = pack.Payload;
                m_payloadAndMarkerIsSet;
            }
            else
            {   // marker & payload must be const
                ESS_ASSERT(pack.Marker == m_marker);
                ESS_ASSERT(pack.Payload == m_payload);
            }
            
            for (int i = 0; i < pack.Buff->size(); ++i)
            {
                m_buff.push_back( pack.Buff->At(i) );
            }            
        }

        // ret byte/msec
        int getSpeed()const
        {
            switch (m_payload)
            {
            case(PCMU):
                return 8;
            case(G723):
                return 8;
            default:
                ESS_ASSERT(0);
                return 0;
            }
        }

    public:

        QueueSender(iCore::MsgThread& thread, iRtp::RtpPcSession& rtp, const std::string& fileName, IQueueSenderToOwner& owner)
            : iCore::MsgObject(thread),
            m_payloadAndMarkerIsSet(false),
            m_owner(owner),
            m_rtp(rtp),            
            m_timer(this, &QueueSender::DoSend),
            m_readPos(0),
            m_payload(/*payload*/PCMU/*G723*/),
            m_timestamp(0),
            m_prevTick(0)
        {        
            /*PackQueue::Packet pack;
            while( packetQueue.Pop(pack) )
                Save(pack);*/
            m_marker = false;

            std::ifstream fileStream( fileName.c_str() );
            
            while (!fileStream.fail())
            {
                char c;
                fileStream >> c;
                m_buff.push_back(c);
            }
            //std::copy(fileSream.begin());

            m_rtp.setPayload(m_payload);

            int period = 20;//CPacketSize / getSpeed();
            m_timer.Start(period, true);
        }
    };

    // QueueSender
    //-----------------------------------------------------------------------------------

    class RtpSessionForTest 
        : public iCore::MsgObject,
        boost::noncopyable,
        public Sip::ISipToRtp,
        public iRtp::IRtpPcToUser,
        IQueueSenderToOwner
    {
        enum 
        { 
            CReserveStore = 1000, // reserved storage for rx packet
            CRcvTime = 20 * 1000  // rcv time
        };

        struct LogDetail 
        {

            iLogW::LogStore m_store;
            iLogW::LogSessionSettings m_settings;
            
            boost::scoped_ptr<iLogW::LogSessionProfileSetup> m_logPrepearer;

            LogDetail();
        };

        enum State
        {
            st_null,
            st_inited,
            st_recive,
            st_write,
            st_waitCanSend,
            st_send,
            st_complete
        };

        void AssertState(State st)
        {
            TUT_ASSERT(m_state == st);
        }

        void OnTimer(iCore::MsgTimer*)
        {
            AssertState(st_recive);            
            
            m_state = st_write;
            WriteQueueToFile();
        }

    // IQueueSenderToOwner
    private:

        void SendComplete()
        {
            AssertState(st_send);            
            m_state = st_complete;
            DoLog("SendComplete");
        }

    public:

        RtpSessionForTest(iCore::MsgThread& thread, const Utils::HostInf &localAddr, const RtpClientParams& params) 
            : iCore::MsgObject(thread),
            m_packetQueue(CReserveStore),  
            m_timer(this, &RtpSessionForTest::OnTimer),
            m_localAddr(localAddr),
            m_state(st_null),
            m_canRcv(false),
            m_canSend(false),
            m_mode(params.Mode),
            m_fileName(Utils::ExeName::GetExeDir() + params.FileName)
        {
            InitInfra();            
        }

        // ISipToRtp        
    private:
        Utils::HostInf getLocalRtp()
        {
            return m_localAddr;
        }

        void SetActiveDestination(const Utils::HostInf& rtpDst, const Utils::HostInf& rtcpDst)
        {
            DoLog("SetActiveDestination");
            InitSession(rtpDst);
        };

        void SetActiveDestination(const Utils::HostInf& rtpDst)
        {
            DoLog("SetActiveDestination");
            InitSession(rtpDst);
        };

        void StartRtpSend(const std::list<resip::SdpContents::Session::Codec> &)
        {
            ESS_ASSERT(m_rtpSession.get() != 0);            
            DoLog("StartRtpSend");
            m_canSend = true;
            if (m_state == st_waitCanSend)
            {
                m_state = st_send;
                DoSend();
            }
        };

        void StartRtpReceive(const std::list<resip::SdpContents::Session::Codec> &)
        {
            ESS_ASSERT(m_rtpSession.get() != 0);            
            DoLog("StartReceive");
            m_canRcv = true;
            m_state = st_recive;
            if (m_mode == WriteToFile)
            {
                m_timer.Start(CRcvTime);
                return;
            }
            if (m_mode == ReadFromFile)
            {
                if (m_canSend)
                {
                    m_state = st_send;
                    DoSend();
                    return;
                }

                m_state = st_waitCanSend;

                return;
            }
            ESS_ASSERT(0 && "Not implemented mode");
        };

        void StopRtpSend()
        {
            DoLog("StopRtpSend");
            // assert state??
            m_canSend = false;
        };

        // iRtp::IRtpPcToUser impl:
    private:

        void RxData(iRtp::SharedBdirBuff buff, iRtp::RtpHeaderForUser header)
        {            
            ESS_ASSERT(header.Payload == PCMU); //todo 
            if (m_state == st_recive)
            {
                m_packetQueue.Add(buff, header);                
            }
            //else ignore
            
        };
        void RxEvent(iRtp::RtpEvent ev, dword timestamp)
        {
            ESS_ASSERT(0);
        };        
        void RtpErrorInd(iRtp::RtpBaseError er) 
        {
            ESS_ASSERT(0);
        };

    private:

        void InitSession(const Utils::HostInf &dstAddr)
        {
            ESS_ASSERT(m_infra.get() != 0);
            ESS_ASSERT(m_rtpSession.get() == 0);
            AssertState(st_null);
            m_state = st_inited;
            iRtp::RtpParams rtpProf;
            rtpProf.m_traceOn = true;

            m_rtpSession.reset(new iRtp::RtpPcSession(getMsgThread(), rtpProf, *m_infra.get(), *this, dstAddr) );                        
        }

        void InitInfra()
        {
            iRtp::RtpInfraParams infraProf( m_logObj.m_logPrepearer->getLogSessionProfile() );
            infraProf.m_minPort = m_localAddr.getPort();
            infraProf.m_maxPort = infraProf.m_minPort;            
            m_infra.reset(new iRtp::RtpPcInfra( getMsgThread(), infraProf, m_logObj.m_store.getSessionInterface() ));
        }

        void DoLog(const std::string& msg)
        {
            std::cout << msg << std::endl;
        }

        void DoSend()
        {
            DoLog("DoSend");
            AssertState(st_send);

            //ESS_ASSERT(m_sender.get() == 0);
            ESS_ASSERT(m_rtpSession.get() != 0);
            
            m_sender.reset(new DynamycSizeSender(getMsgThread(), *m_rtpSession, m_fileName, *this) );                      
        }

        void WriteQueueToFile()
        {
            DoLog("Begin WriteQueueToFile....");
            AssertState(st_write);
            //std::string fileName = Utils::ExeName::GetExeDir() + "RtpData.bin";
            std::ofstream fileOut(m_fileName.c_str());

            PackQueue::Packet pack;
            while(m_packetQueue.Pop(pack))
            {
                // todo select by payload      
                ESS_ASSERT (pack.Payload == iRtp::PCMU);
                
                int size = pack.Buff->size();


                //// save as Linear
                //for (int i = 0; i < size; ++i)
                //{      
                //    word strobe = ulaw2linear(pack.Buff->At(i));
                //    char* pStrobe = reinterpret_cast<char*>(&strobe);
                //    fileOut.write(pStrobe, sizeof(word));
                //}

                // no conversion
                fileOut.write(static_cast<char*>(pack.Buff->getRaw()), size);

            }
            DoLog("WriteQueueToFile complete");
        }
        

        LogDetail m_logObj;
        PackQueue m_packetQueue;

        iCore::MsgTimer m_timer;
        Utils::HostInf m_localAddr;         
        State m_state;        
        bool m_canSend;
        bool m_canRcv;

        boost::scoped_ptr<iRtp::RtpPcInfra> m_infra;
        boost::scoped_ptr<iRtp::RtpPcSession> m_rtpSession;
        boost::scoped_ptr<DynamycSizeSender> m_sender;
        //iRtp::RtpPayload m_currentPayload;
        RtpClientMode m_mode;
        const std::string m_fileName;
    };

    //--------------------------------------------------------

    using boost::shared_ptr;
    using boost::scoped_ptr;
   
    class RtpClientForTest :
        public iCore::MsgObject,
        boost::noncopyable,
        public Sip::ISipToRtp,
        public iRtp::IRtpPcToUser
    {
    // internal types
    private:
        struct LogDetail 
        {
            iLogW::LogStore m_store;
            iLogW::LogSessionSettings m_settings;
            scoped_ptr<iLogW::LogSessionProfileSetup> m_logPrepearer;
            LogDetail();
        };

    // fields:
    private:
       
        LogDetail m_logObj;
        iLogW::LogSession m_logSession;

        const TestProfile& m_prof;
        shared_ptr<BaseScenaries> m_scn;

        Utils::HostInf m_localAddr;            
        scoped_ptr<iRtp::RtpPcInfra> m_infra;
        scoped_ptr<iRtp::RtpPcSession> m_rtpSession;

    // own methods:
    private:
        void InitInfra();
        void InitSession(const Utils::HostInf &dstAddr);

    // ISipToRtp impl:
    private:

        Utils::HostInf getLocalRtp();
        void SetActiveDestination(const Utils::HostInf &rtp, const Utils::HostInf &rtcp);
        void SetActiveDestination(const Utils::HostInf &rtp);
        void StartRtpSend(const std::list<resip::SdpContents::Session::Codec> &);
        void StartRtpReceive(const std::list<resip::SdpContents::Session::Codec> &);
        void StopRtpSend();

    // IRtpPcToUser impl:
    private:

        void RxData(SharedBdirBuff buff, RtpHeaderForUser header);
        void RxEvent(RtpEvent ev, dword timestamp)  {ESS_ASSERT(0 && "NotImplemented");}
        void RtpErrorInd(RtpBaseError er)           {ESS_ASSERT(0 && "NotImplemented");}

    public:

        RtpClientForTest(MsgThread& thread, const TestProfile& prof, shared_ptr<BaseScenaries> scn)
            : iCore::MsgObject(thread), //???
            m_logSession(m_logObj.m_store.getSessionInterface(), L"RtpClient", m_logObj.m_logPrepearer->getLogSessionProfile() ),
            m_prof(prof),
            m_scn(scn),
            m_localAddr(QHostAddress::LocalHost, prof.getRtpLocalPort())
        {
            InitInfra();
        }
    };
} // namespace TestRtpWithSip

#endif
