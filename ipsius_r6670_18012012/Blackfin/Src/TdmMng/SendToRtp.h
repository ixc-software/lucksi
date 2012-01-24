#ifndef SENDTORTP_H
#define SENDTORTP_H

#include "ISendDirection.h"
#include "TdmMng/ITdmManager.h"
#include "TdmMng/DataPacket.h"
#include "iRtp/RtpCoreSession.h"

namespace TdmMng
{

    class DirectionID : boost::noncopyable
    {
        std::string m_ip; 
        int m_port; 

    public:

        bool Equal(const std::string &ip, int port) const
        {
            return (m_ip == ip) && (m_port == port);
        }

        DirectionID(const std::string &ip, int port)
            : m_ip(ip), m_port(port)
        {
        }
    };
    

    //-------------------------------------------------------------------------------------
    
    class SendToRtp : 
        public ISendDirection,
        boost::noncopyable
    {
        DirectionID m_id;
        ITdmManager& m_mng;
        iRtp::RtpCoreSession &m_rtp;
        int m_sampleCounter; // used as timestamp                
        Utils::BidirBuffer* m_pBuff;

        void SendBuffered(DataPacket &pack, bool canDetach);
        void SendDirect(DataPacket &pack, bool canDetach);

        void PackToRtp(Utils::BidirBuffer *p);
        
    // ISendDirection impl:
    private:        
        
        void Send(DataPacket &pack, bool canDetach);

        bool Equal(const std::string &ip, int port) const;

        bool Equal(const Utils::SafeRef<ISendSimple> &ref) const
        {
            return false;
        }

    public:

        static int RtpPackSize();

        SendToRtp(ITdmManager& mng, const std::string &ip, int port, bool useAlaw, iRtp::RtpCoreSession &rtp);

        ~SendToRtp();

    };
} // namespace TdmMng

#endif
