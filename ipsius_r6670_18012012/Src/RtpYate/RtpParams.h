#ifndef __RTPPARAMS__
#define __RTPPARAMS__ 

#ifdef ICORE_YATE
#include "YateRtpSession.h";
#else
#include "Yate/yrtp/yatertp.h";
#endif

#include "iNet/PortNumber.h"

namespace Rtp
{
    // Some parameters for adjusting rtp session
    class RtpParams
    {
    public:
# ifdef  ICORE_YATE
        typedef TelEngine::YateRtpSession::Direction Direction; 
        //typedef TelEngine::Thread::Priority ThreadPriority;
# else
        typedef TelEngine::RTPSession::Direction Direction; 
        //typedef TelEngine::Thread::Priority ThreadPriority;
# endif

        RtpParams()
            : m_direction(Direction::FullStop)
            , m_remoteport(0), m_format("mulaw")
            , m_payload(0), m_evpayload(101), m_tos(0), m_anyssrc(false), m_drillhole(false)
        {}

        Direction getDirection() const { return m_direction; }
        void setDirection(Direction direction) { m_direction = direction; }

        iNet::PortNumber getRemotePort() const { return m_remoteport; }
        void setRemotePort(iNet::PortNumber remoteport) { m_remoteport = remoteport; } 

        QHostAddress getRemoteIp() const { return m_remoteip; }
        void setRemoteIp(QHostAddress remoteip) { m_remoteip = remoteip; }

        const std::string& getFormat() const { return m_format; }
        void setFormat(std::string format) { m_format = format; }

        int getPayload() const { return m_payload; }
        void setPayload(int payload) { m_payload = payload; } 

        int getEvPayload() const { return m_evpayload; }
        void setEvPayload(int evpayload) { m_evpayload = evpayload; } 

        int getTos() const { return m_tos; }
        void setTos(int tos) { m_tos = tos; } 

        bool getAnySSRC() const { return m_anyssrc; }
        void setAnySSRC(bool anyssrc) { m_anyssrc = anyssrc; }

        bool getDrillhole() const { return m_drillhole; }
        void setDrillhole(bool drillhole) { m_drillhole = drillhole; }

    private:
        Direction m_direction;  // data transfer direction in rtp session 

        QHostAddress m_remoteip;     
        iNet::PortNumber m_remoteport;  

        std::string m_format;  // used for definition as send data one block or some ones

        int m_payload;
        int m_evpayload;
        int m_tos;            // Type Of Service for the RTP socket

        bool m_anyssrc;       //can ssrc change when a packet with an unexpected SSRC is received

        bool m_drillhole;     // Drill a hole for the RTP and RTCP sockets
    };

}

#endif
