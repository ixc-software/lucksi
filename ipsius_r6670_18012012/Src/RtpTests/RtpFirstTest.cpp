#include "stdafx.h"

#include "iCore/MsgTimer.h"
#include "iCore/MsgObject.h"
#include "iCore/ThreadRunner.h"

#include "Utils/AtomicTypes.h"

#include "RtpYate/RtpSession.h"
#include "RtpTests.h"

namespace 
{
    using iCore::MsgObject;
    using iCore::IThreadRunner;

    typedef Rtp::RtpParams::Direction Direction;

    const QHostAddress CLocalIp(QHostAddress::LocalHost); 

    // ---------------------------------------------------------------
    
    class TestParams : boost::noncopyable
    {
        enum 
        { 
            CTimeInterval = 50, 
            CRepeatCount  = 100,
            CTimeOut = CRepeatCount * CTimeInterval * 2
        };

    public:
        static int getTimeInterval() { return CTimeInterval; }
        static int getRepeatCount() { return CRepeatCount; }
        static int getTimeOut() { return CTimeOut; }
    };

    // ---------------------------------------------------------------

    class OwnerRTPSender 
        : public MsgObject
        , public Rtp::IRtpUserReceiver
    {
        typedef OwnerRTPSender T;

        boost::shared_ptr<Rtp::RtpSession> m_pRtpSession;
        unsigned int m_sendCount;
        Utils::AtomicInt m_recvCount;
        Utils::AtomicBool &m_done;
        iCore::MsgTimer m_timer; 
 
    // Implementation Rtp::IRtpUserReceiver        
    private:
        void ReceiveData(const QByteArray &data, bool marker, 
            unsigned long tStamp)                                       // override
        {
            TUT_ASSERT( data == getDataBlock() ); 
            m_recvCount.Inc();
        }

        void ReceiveEvent(int event, char key, int duration, int volume, 
            unsigned int timestamp)                                      //override
        {
        }

    // End of implementation Rtp::IRtpUserReceiver
    private:

        void OnWaitClientSend(iCore::MsgTimer *pT)
        {
            TUT_ASSERT( m_recvCount == m_sendCount );
            
            if (m_sendCount < TestParams::getRepeatCount())
            {
                m_pRtpSession->SendData(getDataBlock(), 0);
                m_sendCount++;
            }
            else
            {
                AsyncSetReadyForDeleteFlag(m_done);
            }
        }

        QByteArray& getDataBlock() const
        {
            static QByteArray data("123456789");
            return data;
        }

     public:
        OwnerRTPSender(IThreadRunner &runner)
            : MsgObject(runner.getThread()) 
            , m_pRtpSession(new Rtp::RtpSession(this, CLocalIp, true))
            , m_sendCount(0), m_done(runner.getCompletedFlag())
            , m_timer(this, &T::OnWaitClientSend)
        {
        }

        ~OwnerRTPSender()
        {
            // First we must delete RtpSession 
            m_pRtpSession.reset();
        }

        void Start(const QHostAddress &remoteIp, iNet::PortNumber remotePort, 
            Direction direction)
        {
            Rtp::RtpParams rtpParams;
            rtpParams.setRemoteIp(remoteIp);
            rtpParams.setRemotePort(remotePort);
            rtpParams.setDirection(direction);

            TUT_ASSERT( m_pRtpSession->StartRtp(rtpParams) );

            m_timer.Start(TestParams::getTimeInterval(), true);
        }

        iNet::PortNumber getLocalPort() const 
        {
            return m_pRtpSession->getLocalPort();
        }

     };

    // ---------------------------------------------------------------

    class OwnerRTPLoopback 
        : public Rtp::IRtpUserReceiver
    {
        boost::shared_ptr<Rtp::RtpSession> m_pRtpSession;
        unsigned int m_repeatCount;

    // Implementation Rtp::IRtpUserReceiver        
    private:
        void ReceiveData(const QByteArray &data, bool marker, 
            unsigned long tStamp)                                       // override
        {
            m_pRtpSession->SendData(data, 0);
            m_repeatCount++;
        }

        void ReceiveEvent(int event, char key, int duration, int volume, 
            unsigned int timestamp)                                      //override
        {
        }

    // End of implementation Rtp::IRtpUserReceiver
    public:
        OwnerRTPLoopback()
            : m_pRtpSession(new Rtp::RtpSession(this, CLocalIp, true))
            , m_repeatCount(0)
        {
        }

        ~OwnerRTPLoopback()
        {
            // First we must to delete RtpSession 
            m_pRtpSession.reset();
        }

        void Start(const QHostAddress &remoteIp, iNet::PortNumber remotePort, Direction direction)
        {
            Rtp::RtpParams rtpParams;
            rtpParams.setRemoteIp(remoteIp);
            rtpParams.setRemotePort(remotePort);
            rtpParams.setDirection(direction);

            TUT_ASSERT( m_pRtpSession->StartRtp(rtpParams) );
        }

        iNet::PortNumber getLocalPort() const 
        {
            return m_pRtpSession->getLocalPort();
        }
    };

    // ---------------------------------------------------------------


    class FirstTestRTP
    {
    public:
        FirstTestRTP(IThreadRunner &runner, TestParams &params)
            : m_loopback(), m_sender(runner) 
        {
            m_loopback.Start(CLocalIp, m_sender.getLocalPort(), 
                TelEngine::RTPSession::SendRecv);
            m_sender.Start(CLocalIp, m_loopback.getLocalPort(), 
                TelEngine::RTPSession::SendRecv);
        }
        
    private:
        OwnerRTPLoopback m_loopback;
        OwnerRTPSender m_sender;
    };

} // namespace

namespace RtpTests
{
    void RtpFirstTest()
    {
        TestParams params;
        iCore::ThreadRunner test(QThread::LowPriority, TestParams::getTimeOut());

        if (test.Run<FirstTestRTP>(params))
        {
            TUT_ASSERT( "Test is succesful!" );
        }
        else
        {
            TUT_ASSERT(0 && "Test Time out");
        }
    }
}
