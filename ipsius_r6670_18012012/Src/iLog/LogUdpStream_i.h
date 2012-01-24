#ifndef __LOGUDPSTREAM__
#define __LOGUDPSTREAM__

// DON'T INCLUDE THIS FILE DIRECTLY!
// For use in LogWrapperLibrary.h only

#include "Utils/QtHelpers.h"
#include "Utils/HostInf.h"

namespace iLogW
{
    // Class udp-stream for log listing output
    class LogUdpStream : public LogBasicStream
    {
        QUdpSocket m_udp;
        QHostAddress m_ip;
        int m_port;
    public:
        ESS_TYPEDEF(UdpError);

        LogUdpStream(const Utils::HostInf &host,
            boost::shared_ptr<ILogRecordFormater> formater = boost::shared_ptr<ILogRecordFormater>());
    private:
        void Write(const iLogW::LogString &data);  // override;
    };
};  // namespace iLog

#endif

