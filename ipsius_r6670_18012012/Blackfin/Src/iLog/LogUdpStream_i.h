#ifndef __LOGUDPSTREAM__
#define __LOGUDPSTREAM__

#include "Lw/UdpSocket.h"
#include "Utils/HostInf.h"

//#include "DevIpTdm/BfLed.h"

namespace iLogW
{	
    // Streaming to UDP socket
    class LogUdpStream : public LogBasicStream
    {
    	Lw::UdpSocket m_sock;

    public:

        LogUdpStream(const Utils::HostInf &host, boost::shared_ptr<ILogRecordFormater> formater) : 
        	LogBasicStream(formater),
        	m_sock(host.Port())
        {
        	m_sock.SetDestination(host);
        }

        void Write(const iLogW::LogString &data)  // override
        {            
            int len = data.size();
            
            if (len <= 0) return;
            	
            if (len > Lw::UdpSocket::MaxUpdPackSize()) len = Lw::UdpSocket::MaxUpdPackSize();
            
            
            
            m_sock.Send((void*)&data[0], len);
        }

    };

		
}  // namespace iLogW


#endif
