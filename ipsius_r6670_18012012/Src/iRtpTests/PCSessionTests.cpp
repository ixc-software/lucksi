#include "stdafx.h"

#include "Domain/DomainTestRunner.h"

#include "iRtp/RtpPcInfra.h"
#include "iRtp/IRtpPcToUser.h"
#include "iRtp/RtpPcSession.h"

#include "iCore/MsgTimer.h"

#include "PcSessionTests.h"
#include "iLog/LogManager.h"

namespace iRtpTests
{
    struct LoopBackParam
    {
        iLogW::LogStore& Store;
        const iLogW::LogSettings& LogSettings;

        LoopBackParam(iLogW::LogStore& store, const iLogW::LogSettings& logSettings)
            : Store(store),
            LogSettings(logSettings) //!!! no compile error
        {}        
    };

    namespace
    {
        using namespace iRtp;
        using boost::scoped_ptr;
        using Domain::DomainClass;
        
        // todo : проверка фильтрации по номерам в соответствии с RtpParams::SeqCounterSettings

        class PcClientRtp : public IRtpPcToUser, public iCore::MsgObject
        {
            typedef PcClientRtp TMy;
            
            // настройки
            enum{CFixedTS = 1000, CRepeatTest = 10, CRepeatInterval = 50};
            static const RtpEvent CFixedEvent = BusyTone;
            static const bool CFixedMarker = false;
            static const RtpPayload CFixedPayload = PCMA;


            // состояния
            enum State
            {
                stNull,
                stWaitRxData,
                stWaitRxEvent,
                stComplete
            };

            
            DomainClass& m_domain;                 
            iLogW::LogManager m_logManager;

            State m_state;
            QByteArray m_lastData;
            int m_repeatCount;
            iCore::MsgTimer m_tTransmit;
            int m_transmitCount;


            // Rtp system
            RtpParams m_params;
            RtpInfraParams m_infraParams;
            RtpPcInfra m_infra;
            scoped_ptr<iRtp::RtpPcSession> m_rtp;

            QByteArray CreateData()
            {
                char data[] = {1,2,3,4,5,6,7,8,9};
                QByteArray rez(data, sizeof (data));
                m_lastData = rez;
                return rez;
            }

            void FinishCycle()
            {
                if (++m_repeatCount < CRepeatTest)
                {
                    m_state = stNull;
                    m_transmitCount = 0;
                    m_tTransmit.Start(CRepeatInterval, true);
                }
                else
                {
                    iRtp::RtpStatistic stat = m_rtp->getAllTimeStats();
                    std::cout << "\nRtpStat:\n" << stat.getAsString() << std::endl;
                    std::cout << "\nRtpLastStat:\n" << m_rtp->getLastSsrcStats().getAsString() << std::endl;
                    m_state = stComplete;
                    m_domain.Stop();
                }
            }

            // step 1
            void DoTransmitt(iCore::MsgTimer* pTimer)
            {
                ESS_ASSERT(m_state == stNull);
                QByteArray data = CreateData();

                if (m_repeatCount != 0 || ++m_transmitCount == m_params.SeqConterSettings.MinSequential)                
                {
                    m_state = stWaitRxData;
                    pTimer->Stop();
                }

                m_rtp->TxData(CFixedMarker, CFixedTS, data);            
            }

        // IRtpPcToUser
        private:

            // step 2
            void RxData(SharedBdirBuff buff, RtpHeaderForUser header)
            {                
                TUT_ASSERT(m_state == stWaitRxData);                
                ESS_ASSERT(buff.get() != 0);

                QByteArray data( reinterpret_cast<const char*>(buff->Front()), buff->Size() );
                TUT_ASSERT(data == m_lastData);
                TUT_ASSERT(
                    header.Marker == CFixedMarker && 
                    header.Timestamp == CFixedTS && 
                    header.Payload == CFixedPayload
                    );    

                /*do step 3  ...*/

                FinishCycle();                
            }

            // step 3
            void RxEvent(RtpEvent ev, dword timestamp)
            {
                TUT_ASSERT(m_state == stWaitRxEvent);
                TUT_ASSERT(ev == CFixedEvent && timestamp == CFixedTS);
                FinishCycle();

            }

            //void RxNewPayload(RtpPayload payload) {ESS_ASSERT(0);}
            void RtpErrorInd(RtpBaseError er) {ESS_ASSERT(0);}

            void NewSsrcRegistred(Platform::dword) {}

        public:

            PcClientRtp(DomainClass &domain, const LoopBackParam& param)
                : iCore::MsgObject(domain.getMsgThread()),
                m_domain(domain), 
                m_logManager(param.LogSettings),                
                m_transmitCount(0),
                m_infraParams(2, 1001),
                m_tTransmit(this, &TMy::DoTransmitt),                
                m_state(stNull),
                m_repeatCount(0),                
                m_infra(domain.getMsgThread(),  m_infraParams, m_logManager)
            {  
                Start();
            }


            void Start()
            {
                ESS_ASSERT(m_state == stNull && m_repeatCount == 0);
                HostInf local;
                local.Set(QHostAddress::LocalHost, m_infraParams.MinPort()); // todo get ...
                
                m_rtp.reset( new iRtp::RtpPcSession(m_domain.getMsgThread(), m_params, m_infra.Infra(), *this, local) );    
                m_rtp->setPayload(CFixedPayload); // из параметров теста?
                m_tTransmit.Start(CRepeatInterval, true);
            }

            /*bool IsComplete()
            {
                return m_state == stComplete;
            }*/


        };

    }
    
    // Simple test. Set socket to local host and validate send/receive procedures.
	void PcSessionLoopbackTest()
	{   
        iRtp::RtpStatistic stat;
        std::cout << stat.getAsString() << std::endl;
        //iCore::MsgThread thread(QThread::IdlePriority, true);

        iLogW::LogStore store;
        iLogW::LogSettings logSettings;

        logSettings.out().CoutOn();// setOutputToCout(true); ????
        logSettings.setTimestampInd(true);

        LoopBackParam param(store, logSettings);
        //param.m_file = "LoopBack.log";
        //param.m_writeToConsole ...

        Domain::DomainTestRunner test(15*1000); 

        bool rez = test.Run<PcClientRtp>(param);
        TUT_ASSERT( rez );  
        std::cout << "\nPCSessionLoopbackTest complete.";

    }

} // namespace iRtpTests
