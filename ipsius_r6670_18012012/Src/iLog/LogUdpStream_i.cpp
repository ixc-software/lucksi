#include "stdafx.h"

#include "LogWrapperLibrary.h"
#include "LogUdpStream_i.h"

namespace iLogW
{
    LogUdpStream::LogUdpStream(const Utils::HostInf &host, 
        boost::shared_ptr<ILogRecordFormater> formater) : 
        LogBasicStream(formater),
        m_ip(host.Address().c_str()),
        m_port(host.Port())
    {
        if(m_port && !m_udp.bind()) ESS_THROW(UdpError);
    }

    void LogUdpStream::Write( const iLogW::LogString &data )
    {
        if(m_port) m_udp.writeDatagram(QString::fromStdWString(data).toAscii(), m_ip, m_port);
    }
};

