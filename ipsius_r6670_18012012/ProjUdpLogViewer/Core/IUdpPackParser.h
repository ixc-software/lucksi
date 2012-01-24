
#ifndef __IUDPPACKPARSER__
#define __IUDPPACKPARSER__


#include "Utils/IBasicInterface.h"
#include "UdpLogExceptions.h"
#include "UdpLogRecordInfo.h"

namespace Ulv
{
    // Base interface for all UDP packet parsers
    class IUdpPackParser : 
        public Utils::IBasicInterface
    {
    public:
        // throw LogRecordParsingFailed exception if failed
        virtual void Parse(const QString &inPacket, UdpLogRecordInfo &outData) = 0;
        virtual const QString& UsedDateTimeFormat() const = 0;
    };

    
} // namespace Ulv

#endif
