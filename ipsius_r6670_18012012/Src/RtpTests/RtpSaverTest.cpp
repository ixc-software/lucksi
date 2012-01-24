#include "stdafx.h"

#include "iCore/ThreadRunner.h"
#include "iCore/MsgObject.h"
#include "iCore/MsgTimer.h"

#include "Utils/AtomicTypes.h"
//#include "iLog/LogWrapper.h"
#include "Utils/MemoryPool.h"

#include "Rtp/RtpSession.h"
#include "Rtp/RtpPacket.h"
#include "Rtp/RtpTypes.h"
#include "Rtp/RtpEvent.h"
#include "Rtp/RtpManager.h"
#include "Rtp/IRtpSessionEvents.h"


namespace
{
    using boost::shared_ptr;
    using boost::scoped_ptr;
    using Platform::dword;
    using iCore::IThreadRunner;
    using Rtp::RtpParams;
    using Rtp::RtpLogParams;
    using Rtp::PayloadType;
    using Rtp::RtpEvent;

    class TestParams
    {
        static const dword CTimeInterval = 100;
        static const dword CFirstTimeIntervel = 5000;

        QHostAddress m_localIp;
        PayloadType m_payload;
        

    public:
        TestParams(const QHostAddress &localIp, PayloadType payload)
            : m_localIp(localIp), m_payload(payload)
        {
        
        }

        const QHostAddress & getLocalIp() const { return m_localIp; }
        PayloadType getPayload() const { return m_payload; }

        static dword getTimeInterval() { return CTimeInterval; }
        static dword getFirstTimeInterval() { return CFirstTimeIntervel; }

    };

    // -----------------------------------------------

    class RtpSaver :
        public iCore::MsgObject,
        public Rtp::IRtpSessionEvents,
        private boost::noncopyable
    {        
        
        class RtpData
        {
            shared_ptr<QByteArray> m_pData;
            bool m_marker;
            dword m_timestamp;
        public:
            RtpData(const QByteArray data, bool marker, dword timestamp)
                : m_pData(new QByteArray(data)), m_marker(marker), m_timestamp(timestamp)
            {
            }

            shared_ptr<QByteArray> getData() { return m_pData; }
            bool getMarker() const { return m_marker; }
            dword getTimestamp() const { return m_timestamp; }
        };

        typedef RtpSaver T;                

        Rtp::RtpSession m_session;    
        std::list< shared_ptr<RtpData> > m_list;
        iCore::MsgTimer m_timer;
        bool m_flag;  // end of data thread
        Utils::AtomicBool &m_done;


    // Rtp::IRtpSessionEvents impl
    private:    

        void OnReceiveData( Rtp::SharedInRtpPack rtpPack )
        {
            // todo
            /*if (m_list.empty())
            {
                m_timer.Stop();
                m_timer.Start(TestParams::getTimeInterval(), true);
            }

            shared_ptr<RtpData> pRtpData(new RtpData(data, marker, timestamp));
            m_list.push_back(pRtpData);
            
            m_flag = true;*/
        }

        void OnReceiveEvent(boost::shared_ptr<RtpEvent> pEvent, 
            dword timestamp)
        {
        }

        void OnReceiveNewPayload(PayloadType payload, dword timestamp)
        {
            // Activating RTP silence payload 
            if (payload == Rtp::CN) 
                m_session.ChangeSilencePayload(payload);
        }

        void OnRtpSocketError(boost::shared_ptr<iNet::SocketError> pError)   
        {
            std::string msg = pError->getErrorString().toStdString();
            ESS_HALT(msg);
        }

    private:

        void OnWaitClientSend(iCore::MsgTimer *pT)
        {
            if (m_flag == false)
                AsyncSetReadyForDeleteFlag(m_done);

            m_flag = false;
        }

    public:

        RtpSaver(IThreadRunner &runner, const RtpParams &params, 
            const RtpLogParams &logParams, Rtp::RtpManager &manager)
            : iCore::MsgObject(runner.getThread()), 
            m_session(runner.getThread(), *this, params, logParams, manager),
            m_timer(this, &T::OnWaitClientSend),
            m_done(runner.getCompletedFlag()),
            m_flag(false)

        {
            iNet::PortNumber port = m_session.getLocalPort();
            m_timer.Start(TestParams::getFirstTimeInterval());
        }


    };

    // -----------------------------------------------

    class RtpSaverTestClass : boost::noncopyable
    {
        // настройки аллокации
        // перенести в параметры теста?
        enum
        {
            CRtpPacketPoolBlockSize = 40,
            CPackBlockCount = 1, // preallocated

            CBidirBuffPoolBlockSize = 4 * 1024, //4KB
            CBuffBlockCount = 1 // preallocated
        };

    public:
        
        RtpSaverTestClass(IThreadRunner &runner, TestParams &params)
            : m_shellPool(CRtpPacketPoolBlockSize, CPackBlockCount),
            m_buffPool(CBidirBuffPoolBlockSize, CBuffBlockCount),
            m_rtpParams(params.getLocalIp(), false, params.getPayload(), m_buffPool, m_shellPool, false)
        {
            RtpLogParams logParams(true, true, true, m_logStore, "");
            //RtpParams rtpParams(params.getLocalIp(), false, params.getPayload(), false);
            m_rtpParams.setAutoRemote(true);

            m_pSaver.reset(new RtpSaver(runner, m_rtpParams, logParams, m_rtpManager));
        }

    private:

        Utils::FixedMemBlockPool<> m_shellPool;
        Utils::FixedMemBlockPool<> m_buffPool;
        RtpParams m_rtpParams;

        iLogW::LogStore m_logStore;
        Rtp::RtpManager m_rtpManager;
        scoped_ptr<RtpSaver> m_pSaver;
    };


} // namespace

namespace RtpTests
{
    void RtpSaverTest()
    {
        TestParams params(QHostAddress::LocalHost, Rtp::PCMA); 
        iCore::ThreadRunner test(QThread::LowPriority);
        test.Run<RtpSaverTestClass>(params);
        std::cout << "RtpSaverTest is finished!" << std::endl;
    }

} // namespace RtpTests
