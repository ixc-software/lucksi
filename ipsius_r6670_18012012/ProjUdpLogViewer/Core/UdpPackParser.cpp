
#include "stdafx.h"
#include "UdpPackParser.h"

namespace
{
    const QString CBeginBreak = "[";
    const QString CEndBreak = "]";

} // namespace

// -------------------------------------------------------

namespace Ulv
{
    // static
    bool UdpPackParser::CheckTimestampString(const QString &timestamp)
    {
        // timestamp should contain just numbers, '.', ':' and ' '

        for(int i = 0; i < timestamp.size(); i++)
        {
            QChar ch = timestamp.at(i);
            bool isDigit = ((ch >= '0') && (ch <= '9'));
            if ( !(isDigit || (ch == '.') || (ch == ':') || (ch == ' ')) ) return false;
        }

        return true;
    }

    // -------------------------------------------------------

    void UdpPackParser::Parse(const QString &inPacket, UdpLogRecordInfo &outData) // can throw
    {
        // (timestamp) [sessionName][sessionTag] data
        // or
        // (timestamp) [sessionNameAndTag] data
        if ((inPacket.size() == 0)
             || (inPacket.count(CBeginBreak) != inPacket.count(CEndBreak)))
        {
            ESS_THROW(LogRecordParsingFailed);
        }

        int beginBreakIndex = inPacket.indexOf(CBeginBreak);
        int endBreakIndex = inPacket.lastIndexOf(CEndBreak);

        if ((beginBreakIndex < 0) || (endBreakIndex < 0)
            || (endBreakIndex <= beginBreakIndex))
        {
            ESS_THROW(LogRecordParsingFailed);
        }

        QString timestamp = inPacket.left(beginBreakIndex).trimmed();
        // try convert
        QDateTime dt = QDateTime::fromString(timestamp, m_dtFormat);
        if (dt.isValid())
        {
            outData.SenderTimestamp = dt;
        }
        else
        {
            if (!CheckTimestampString(timestamp)) ESS_THROW(LogRecordParsingFailed);
            outData.SenderTimestampStr = timestamp;
        }

        /* without breaks
        QString nameAndTag = inPacket.mid(beginBreakIndex + 1,
                                          endBreakIndex - beginBreakIndex - 1).trimmed();

        outData.SessionName = nameAndTag.left(nameAndTag.indexOf(CEndBreak)).trimmed();
        outData.SessionTag = nameAndTag.mid(nameAndTag.indexOf(CBeginBreak) + 1).trimmed();
        */

        QString nameAndTag = inPacket.mid(beginBreakIndex,
                                          endBreakIndex - beginBreakIndex + 1).trimmed();

        outData.SessionName = nameAndTag.left(nameAndTag.indexOf(CEndBreak) + 1).trimmed();
        outData.SessionTag = nameAndTag.mid(nameAndTag.indexOf(CBeginBreak, 1)).trimmed();

        outData.Data = inPacket.mid(endBreakIndex + 1);

        outData.IsSuccessfullyParsed = true;
    }


} // namespace Ulv
