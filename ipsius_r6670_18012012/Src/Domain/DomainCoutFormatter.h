
#ifndef __DOMAINCOUTFORMATTER__
#define __DOMAINCOUTFORMATTER__

// DomainCoutFormatter.h


namespace DRI
{
    class CommandDRI;

} // namespace DRI

// ------------------------------------------

namespace Domain
{
    class DomainCoutFormatter
    {
        QString m_tag;
        bool m_useTimestamp;
        QString m_tsFormat;
        
    public:
        DomainCoutFormatter(const QString &tag = "", 
                            bool useTimestamp = false, 
                            const QString &timestampFormat = "hh:mm:ss") : 
            m_tag(tag), 
            m_useTimestamp(useTimestamp), 
            m_tsFormat(timestampFormat)
        {
            if (useTimestamp) ESS_ASSERT(!timestampFormat.isEmpty());
        }

        QString Format(const QString &data)
        {
            QString res;
            if (m_useTimestamp)
            {
                res += QString("%1 ").arg(QDateTime::currentDateTime().toString(m_tsFormat));
            }
            
            if (!m_tag.isEmpty())
            {
                res += QString("[%1] ").arg(m_tag);
            }

            res += data;
            
            return res;
        }

        QString FormatIncompleteLineBegin(const QString &data)
        {
            return Format(data);
        }

        QString FormatIncompleteLineEnd(const QString &data)
        {
            return data;
        }
    };
    
} // namespace Domain

#endif
