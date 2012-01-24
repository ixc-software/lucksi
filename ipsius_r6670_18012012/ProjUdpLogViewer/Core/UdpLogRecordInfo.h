#ifndef UDPLOGRECORDINFO_H
#define UDPLOGRECORDINFO_H

#include "UdpLogRecordSrc.h"

namespace Ulv
{
    // Parsed UDP packet + packet source info
    struct UdpLogRecordInfo // : boost::noncopyable
    {
        // const UdpLogRecordSrc &Source;
        Utils::SafeRef<UdpLogRecordSrc> Source;

        QDateTime RecvrTimestamp;
        QDateTime SenderTimestamp;
        QString SenderTimestampStr; // save as str if failed to parse
        QString SessionName;
        QString SessionTag;
        QString Data;

        bool IsSuccessfullyParsed;

    public:
        UdpLogRecordInfo(Utils::SafeRef<UdpLogRecordSrc> source, const QDateTime &recvrTimestamp) :
            Source(source), RecvrTimestamp(recvrTimestamp), IsSuccessfullyParsed(false)
        {
        }

        /*UdpLogRecordInfo(const UdpLogRecordInfo &other) :
            Source(other.Source),
            RecvrTimestamp(other.RecvrTimestamp),
            SenderTimestamp(other.SenderTimestamp),
            SenderTimestampStr(other.SenderTimestampStr),
            SessionName(other.SessionName),
            SessionTag(other.SessionTag),
            Data(other.Data)
        {
        }*/

    };


} // namespace Ulv

#endif // UDPLOGRECORDINFO_H
