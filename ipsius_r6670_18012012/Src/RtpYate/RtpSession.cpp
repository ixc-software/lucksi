#include "stdafx.h"

#include "Utils/AtomicTypes.h"

#include "Yate/yrtp/yatertp.h";

#include "RtpSession.h"

namespace Rtp
{
    // Inplementation of RtpSession
    class RtpSession::Impl : private TelEngine::RTPSession
    {
	public:
        Impl(IRtpUserReceiver *pOwner, const QHostAddress &localIp, bool rtcp);
        ~Impl();
            
        bool StartRtp(const RtpParams& params, int msleep, 
            TelEngine::Thread::Priority threadPriority);
        void SendData(const QByteArray &data, bool marker, unsigned long tStamp);
        bool SendDtmf(char dtmf, int duration = 0);
        void AddDirection(TelEngine::RTPSession::Direction dir);

        iNet::PortNumber getLocalPort() const { return m_localPort; }
        unsigned int getBufSize() const  { return m_bufSize; }

    private:
        void SetupRtp(const QHostAddress &localIp, bool rtcp);
        bool setRemote(const QHostAddress &remoteAddress, iNet::PortNumber rport);

        // override functions TelEngine::RTPSession
        bool rtpRecvData(bool marker, unsigned int timestamp, const void* data, int len);
        bool rtpRecvEvent(int event, char key, int duration, int volume, unsigned int timestamp);
        void rtpNewPayload(int payload, unsigned int timestamp);
        void rtpNewSSRC(u_int32_t newSsrc);
        
        enum 
		{
			CMinPort = 16384,
			CMaxPort = 32769,
			CBufSize = 240,
			CBufPref = 160
		};

        IRtpUserReceiver *m_pOwner;

        bool m_started;           // is rtp started?
        iNet::PortNumber m_localPort;
        unsigned int m_bufSize;

        bool m_resync;            // after restart RTP, need change ssrc
        bool m_anyssrc;           // can ssrc change when a packet with an unexpected SSRC is received

        bool m_splitable;         // as send data one block or some ones
        //Utils::AtomicBool m_busy; // when removing this to wait for rtpRecvData() to finish      
        //TelEngine::Mutex m_mutex; // use for protection of receive when it will destroy
    };


    // ---------------------------------------------------------------

    RtpSession::Impl::Impl(IRtpUserReceiver *pOwner, const QHostAddress &localIp, bool rtcp)
        : m_pOwner(pOwner), m_started(false)
        , m_localPort(0), m_bufSize(CBufSize), m_splitable(false)
    {
        initTransport();  // create m_transport and assigned RTPTransport::m_processor=this
        SetupRtp(localIp, rtcp);
    }

    // ---------------------------------------------------------------

    RtpSession::Impl::~Impl()
    {
        // disconnect thread and transport before our virtual methods become invalid
        // this will also lock the group preventing rtpRecvData from being called
        group(0);
        transport(0);

        //m_mutex.lock();
        //m_pOwner = 0;
        //m_mutex.unlock();

        //// we have just to wait for any rtpRecvData() to finish
        //while (m_busy.Get())
        //    TelEngine::Thread::yield();

    }

    // ---------------------------------------------------------------

    void RtpSession::Impl::SetupRtp(const QHostAddress &localIp, bool rtcp)
    {
        int minport = CMinPort;
        int maxport = CMaxPort;
        int attempt = 10;
        if (minport > maxport) {
            int tmp = maxport;
            maxport = minport;
            minport = tmp;
        }
        else if (minport == maxport) {
            maxport++;
            attempt = 1;
        }

        TelEngine::SocketAddr addr(AF_INET);
        if (!addr.host(localIp.toString().toStdString().c_str())) 
            return;

        for (; attempt; attempt--) 
        {
            int lport = (minport + (::random() % (maxport - minport))) & 0xfffe;
            addr.port(lport);
            if (localAddr(addr, rtcp)) 
            {
                m_localPort = lport;
                return;
            }
        }
    }

	// ---------------------------------------------------------------
    // when msleep - the idle time in milliseconds for cycle of data receive to sleep 
    bool RtpSession::Impl::StartRtp(const RtpParams &params, int msleep, 
        TelEngine::Thread::Priority threadPriority)
    {
        if (m_started) {
            // attempted to restart RTP
            setRemote(params.getRemoteIp(), params.getRemotePort()); 
            m_resync = true;    
            return true;
        }

        if ((params.getPayload() < 0) || (params.getPayload() >= 127)) 
            return false;

        //int minJitter = msg.getIntValue("minjitter",s_minjitter);
        //int maxJitter = msg.getIntValue("maxjitter",s_maxjitter);

        if (!(getLocalPort() && setRemote(params.getRemoteIp(), params.getRemotePort())))
            return false;

        m_anyssrc = params.getAnySSRC();    

        if (!params.getFormat().empty()) {
            m_splitable = (params.getFormat() == "alaw") || 
                          (params.getFormat() == "mulaw");
        }

        if ( !(initGroup(msleep, threadPriority) && direction(params.getDirection())) )
            return false;

        dataPayload(params.getPayload());
        eventPayload(params.getEvPayload());
        setTOS(params.getTos());        // set RTPTransport::m_rtpSock.setTOS(tos)
        
        if (params.getDrillhole())
            drillHole();                // Drill a hole for the RTP and RTCP sockets

        m_started = true;

        return true;        
    }

	// ---------------------------------------------------------------

    bool RtpSession::Impl::setRemote(const QHostAddress &remoteAddress, iNet::PortNumber remotePort)
    {
        TelEngine::SocketAddr addr(AF_INET);
        // second param of remoteAddr(..) define:
        // Automatically adjust the address from the first incoming packet
        return addr.host(remoteAddress.toString().toStdString().c_str()) && 
                addr.port(remotePort) && remoteAddr(addr, false);
    }

    // ---------------------------------------------------------------

    void RtpSession::Impl::SendData(const QByteArray &data, bool marker, unsigned long tStamp)
    {
        if (!m_started)
            return;

        const char* ptr = data.data();
        unsigned int len = data.length();

        while (len) 
        {
            unsigned int sz = len;
            if (m_splitable  && (sz > m_bufSize)) {
                // divide evenly a buffer that is multiple of preferred size
                if ((m_bufSize > CBufPref) && ((len % CBufPref) == 0))
                    sz = CBufPref;
                else
                    sz = m_bufSize;
            }
            rtpSendData(marker, tStamp, ptr, sz);
            // if timestamp increment is not provided we have to guess...
            tStamp += sz;
            len -= sz;
            ptr += sz;
        }            
    }

    // ---------------------------------------------------------------

    bool RtpSession::Impl::SendDtmf(char dtmf, int duration)
    {
        return m_started ? rtpSendKey(dtmf,duration) : false;        
    }

    // ---------------------------------------------------------------
    // add other (opposite) direction
    void RtpSession::Impl::AddDirection(TelEngine::RTPSession::Direction dir)
    {
        if (m_started)
            addDirection(dir);
    }

	// ---------------------------------------------------------------

    bool RtpSession::Impl::rtpRecvData(bool marker, unsigned int timestamp, 
		const void *data, int len)                                  // override 
	{
		//m_mutex.lock();
        //IRtpUserReceiver *pOwner = m_pOwner ? m_pOwner : 0;
        //if (m_pOwner)
        //{
        //    m_busy.Set(true);
        //}
        //m_mutex.unlock();
        
        //if (!pOwner)
        //    return false;

        //// the source will not be destroyed until we reset the busy flag
		//QByteArray buf((const char *)data, len);
        //pOwner->ReceiveData(buf, timestamp);

        //m_busy.Set(false);

        if (!m_pOwner)
            return false;

		QByteArray buf((const char *)data, len);
        m_pOwner->ReceiveData(buf, marker, timestamp);

        return true;
	}

    // ---------------------------------------------------------------

    bool RtpSession::Impl::rtpRecvEvent(int event, char key, int duration, 
        int volume, unsigned int timestamp)                         // override
    {
        //if (!(m_pOwner && key))
        //    return false;

        //m_mutex.lock();
        //IRtpUserReceiver *pOwner = m_pOwner ? m_pOwner : 0;
        //if (m_pOwner)
        //{
        //    m_busy.Set(true);
        //}
        //m_mutex.unlock();
        //
        //if (!pOwner)
        //    return false;

        //pOwner->ReceiveDtmf(key);
        //m_busy.Set(false);

        if (!m_pOwner)
            return false;

        m_pOwner->ReceiveEvent(event, key, duration, volume, timestamp);
        //m_pOwner->ReceiveDtmf(key);
        return true;
    }

    // ---------------------------------------------------------------
    // Activating RTP silence payload 
    void RtpSession::Impl::rtpNewPayload(int payload, unsigned int timestamp) // override 
	{
		if (payload == 13) {
			silencePayload(payload);
		}
	}

    // ---------------------------------------------------------------
    // Changing SSRC when a packet with an unexpected SSRC is received
    void RtpSession::Impl::rtpNewSSRC(u_int32_t newSsrc)            // override 
	{
		if ((m_anyssrc || m_resync) && receiver()) {
			m_resync = false;
			receiver()->ssrc(newSsrc);
		}
	}

    // ---------------------------------------------------------------

    RtpSession::RtpSession(IRtpUserReceiver *pOwner, 
        const QHostAddress &localIp, bool rtcp)
        : m_pImpl(new Impl(pOwner, localIp, rtcp))
    {
    }

    bool RtpSession::StartRtp(const RtpParams &params, int msleep, 
            TelEngine::Thread::Priority threadPriority)
    {
        return m_pImpl->StartRtp(params, msleep, threadPriority);
    }

    void RtpSession::SendData(const QByteArray &data, bool marker, 
        unsigned long tStamp)
    {
        m_pImpl->SendData(data, marker, tStamp);    
    }

    bool RtpSession::SendDtmf(char dtmf, int duration)
    {
        return m_pImpl->SendDtmf(dtmf, duration);
    }

    void RtpSession::AddDirection(RtpParams::Direction direction)
    {
        m_pImpl->AddDirection(direction);
    }

    iNet::PortNumber RtpSession::getLocalPort() const
    {
        return m_pImpl->getLocalPort();
    }


} // namespace RTP
