#include "stdafx.h"

#include "iCore/MsgTimer.h"
#include "iCore/MsgObject.h"
#include "iCore/ThreadRunner.h"
#include "iNet/SocketError.h"
#include "iNet/SocketError.h"

#include "Utils/AtomicTypes.h"
#include "Utils/MemoryPool.h"

#include "iLog/LogWrapper.h"

#include "Rtp/RtpSession.h"
#include "Rtp/RtpTypes.h"
#include "Rtp/RtpEvent.h"
#include "Rtp/RtpManager.h"
#include "Rtp/IRtpSessionEvents.h"
#include "LogPrint.h"
#include "RtpTests.h"

namespace 
{
    using boost::shared_ptr;
    using Platform::dword;
    using Platform::byte;
    using iCore::MsgObject;
    using iCore::IThreadRunner;
    using Rtp::PayloadType;
    using Rtp::RtpEvent;

    const QHostAddress CLocalIp(QHostAddress::LocalHost);  

    // ----------------------------------------

    class TestParams : boost::noncopyable
    {
        enum 
        { 
            CTimeInterval = 50, 
            CRepeatCount  = 50,
            CNumCycleWhenResetParams = 5,

            CTimeOut = CRepeatCount * CTimeInterval * 5,
            CDataBlockSize = 120,
        };

        static const PayloadType CDataPayload  = Rtp::PCMU;
        static const PayloadType CDataPayload2 = Rtp::PCMA;
        static const PayloadType CEventPayload = PayloadType(96);

        static const iNet::PortNumber CLocalTestPort = 17000;

    public:
        static int getTimeInterval() { return CTimeInterval; }
        static int getRepeatCount() { return CRepeatCount; }
        static int getTimeOut() { return CTimeOut; }

        static PayloadType getDataPayload()  { return CDataPayload; }
        static PayloadType getEventPayload() { return CEventPayload; }

        static int getNumCycleWhenResetParams() { return CNumCycleWhenResetParams; }
        static PayloadType getDataPayload2() { return CDataPayload2; }

        static iNet::PortNumber getLocalTestPort() { return CLocalTestPort; }
        static int getDataBlockSize() {return CDataBlockSize;}
    };

   // ---------------------------------------------------------------

    class RtpClientSender : 
        public MsgObject,
        public Rtp::IRtpSessionEvents,
        private boost::noncopyable
    {
        typedef RtpClientSender T;
        
        Rtp::RtpSession m_session;
        int m_sendCount;
        int m_recvCount;
        Utils::AtomicBool &m_done;
        iCore::MsgTimer m_timer;

    // Rtp::IRtpSessionEvents impl
    private:    

        void OnReceiveData( Rtp::SharedInRtpPack rtpPack )
        {             
            Rtp::SharedBdirBuff buff = rtpPack->getUserData();            
            std::vector<byte> data(&buff->At(0), &buff->At(0) + buff->size());
            TUT_ASSERT( data == getDataBlock() );

            m_recvCount++;
            
            if (m_recvCount == TestParams::getNumCycleWhenResetParams())
            {
                m_session.StopReceive();
                m_session.StartReceive();
                m_session.ChangeDataPayload(TestParams::getDataPayload2());
            }
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
            //TUT_ASSERT( m_recvCount == m_sendCount );
            if (m_recvCount != m_sendCount)
                return;

            if (m_sendCount < TestParams::getRepeatCount())
            {
                m_sendCount++;
                dword timestamp = m_sendCount * TestParams::getTimeInterval();
                // Почему маркер = false
                m_session.SendData(false, timestamp, getDataBlock());
            }
            else
            {
                AsyncSetReadyForDeleteFlag(m_done);
                return;            
            }
        }

        static std::vector<byte>& getDataBlock()
        {            
            static std::vector<byte> data(TestParams::getDataBlockSize(), 'X');            
            return data;
        }

    public:

        RtpClientSender(IThreadRunner &runner, const Rtp::RtpParams &params, 
            const Rtp::RtpLogParams &logParams, Rtp::RtpManager &manager)
            : MsgObject(runner.getThread()),
            m_session(runner.getThread(), *this, params, logParams, manager),
            m_sendCount(0), m_recvCount(0),
            m_done(runner.getCompletedFlag()),
            m_timer(this, &T::OnWaitClientSend)
        {
        }

        ~RtpClientSender()
        {
        }

        void Start(const QHostAddress &remoteIp, iNet::PortNumber remotePort)
        {
            m_session.StopReceive();
            m_session.StartReceive();

            m_session.setRemoteAddress(remoteIp, remotePort);            

            m_timer.Start(TestParams::getTimeInterval(), true);
        }

        iNet::PortNumber getLocalPort() const
        {
            return m_session.getLocalPort();
        }
    };

    // ---------------------------------------------------------------

    class RtpLoopback :
        public Rtp::IRtpSessionEvents,
        private boost::noncopyable
    {
        Rtp::RtpParams m_params;
        Rtp::RtpSession m_session;
        int m_repeatCount;

    // Rtp::IRtpSessionEvents impl
    private:    
        
        void OnReceiveData( Rtp::SharedInRtpPack rtpPack ) 
        {
            m_repeatCount++;
            // заворот
            Utils::IFixedMemBlockPool& shellPool = m_params.getShellPool();
            Rtp::SharedBdirBuff buff = rtpPack->getUserData();
            
            std::vector<byte> data(&buff->At(0), &buff->At(0) + buff->size());
            m_session.SendData(false, rtpPack->getTs(), data);

            if (m_repeatCount == TestParams::getNumCycleWhenResetParams())
            {
                m_session.ChangeDataPayload(TestParams::getDataPayload2());
            }
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

    public:

        RtpLoopback(IThreadRunner &runner, const Rtp::RtpParams &params, 
            const Rtp::RtpLogParams &logParams, Rtp::RtpManager &manager) 
            : m_params(params), 
            m_session(runner.getThread(), *this, params, logParams, manager),
            m_repeatCount(0)
        {
            
        }

        ~RtpLoopback()
        {
        }

        void Start(const QHostAddress &remoteIp, iNet::PortNumber remotePort)
        {
            m_session.getLogParams().setLogActive(true);
            m_session.setRemoteAddress(remoteIp, remotePort);
        }

        iNet::PortNumber getLocalPort() const
        {
            return m_session.getLocalPort();
        }

    };


    // ----------------------------------------------------------------

    class RtpTestClass : boost::noncopyable
    {
        // настройки аллокации
        // перенести в параметры теста?
        enum
        {
            CRtpPacketPoolBlockSize = 80,
            CPackBlockCount = 1, // preallocated

            CBidirBuffPoolBlockSize = 4 * 1024, //4KB
            CBuffBlockCount = 1 // preallocated
        };

    public:

        RtpTestClass(IThreadRunner &runner, TestParams &params)
            : m_shellPool(CRtpPacketPoolBlockSize, CPackBlockCount),
            m_buffPool(CBidirBuffPoolBlockSize, CBuffBlockCount),
            m_rtpParams(CLocalIp, false, TestParams::getDataPayload(), m_buffPool, m_shellPool)
        {
            RtpTests::LogPrint logSessionPrint(m_logStore, "LogTestLoopback.txt", false);
            m_pLogSessionHandler = RtpTests::LogPrint::CreateLogHandler(logSessionPrint, true);
            
            Rtp::RtpLogParams logSender(true, true, true, m_logStore, "Sender", 
                true, true, m_pLogSessionHandler.get());
            Rtp::RtpLogParams logLoopback(true, true, true, m_logStore, "Loopback", 
                true, true, m_pLogSessionHandler.get());
            
            
            m_rtpParams.setDrillhole(true);
            m_rtpParams.setEventPayload(TestParams::getEventPayload());
            m_rtpParams.setDejitterDelay(20, 50);

            
            //m_rtpParams.setLocalIp(CLocalIp);
            

            m_pLoopback.reset(new RtpLoopback(runner, m_rtpParams, logLoopback, m_rtpManager));
            
            m_rtpParams.setLocalPort(TestParams::getLocalTestPort());
            m_pSender.reset(new RtpClientSender(runner, m_rtpParams, logSender, m_rtpManager));

            m_pLoopback->Start(CLocalIp, m_pSender->getLocalPort());
            m_pSender->Start(CLocalIp, m_pLoopback->getLocalPort());
        }
        
    private:

        Utils::FixedMemBlockPool<> m_shellPool;
        Utils::FixedMemBlockPool<> m_buffPool;
        Rtp::RtpParams m_rtpParams;
        
        boost::shared_ptr<iLogW::ILogSessionHandler> m_pLogSessionHandler;
        iLogW::LogStore m_logStore;
        Rtp::RtpManager m_rtpManager;
        boost::scoped_ptr<RtpLoopback> m_pLoopback;
        boost::scoped_ptr<RtpClientSender> m_pSender;
    };

} // namespace

#include "Platform/PlatformTypes.h"
namespace RtpTests
{
    using Platform::word;
    using Platform::byte;

    void RtpLoopbackTest()
    {
        using namespace Platform;
        /*QByteArray arr;
        {
            QDataStream out(&arr,QIODevice::WriteOnly);
            word i = 0xFF18;
            out << i;
            arr += (byte)(i>>8);
            arr += (byte)(i&0xFF);        
        }     */   
        TestParams params;
        iCore::ThreadRunner test(QThread::LowPriority, TestParams::getTimeOut());

        if (test.Run<RtpTestClass>(params))
        {
            TUT_ASSERT( "Test is succesful!" );
            std::cout << "RtpLoopbackTest: Ok!" << std::endl;
        }
        else
        {
            TUT_ASSERT(0 && "Test Time out");
        }
    }
}
