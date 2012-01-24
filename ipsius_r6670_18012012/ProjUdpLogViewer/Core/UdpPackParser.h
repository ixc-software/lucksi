
#ifndef __UDPPACKPARSER__
#define __UDPPACKPARSER__


#include "IUdpPackParser.h"
#include "UdpLogRecord.h"

#include "Utils/ErrorsSubsystem.h"

namespace Ulv
{
    // Default received UDP packet parser
    class UdpPackParser :
        public IUdpPackParser,
        boost::noncopyable
    {
        QString m_dtFormat;
        
        // doesn't check format
        static bool CheckTimestampString(const QString &ts);

    // IUdpPackParser impl
    private:
        void Parse(const QString &inPacket, UdpLogRecordInfo &outData); // can throw
        const QString& UsedDateTimeFormat() const { return m_dtFormat; }

    public:
        UdpPackParser(const QString &dateTimeFormat = "dd.MM.yyyy hh:mm:ss.zzz") :
            m_dtFormat(dateTimeFormat)
        {
            ESS_ASSERT(!dateTimeFormat.isEmpty());
        }
    };
    
} // namespace Ulv

#endif
