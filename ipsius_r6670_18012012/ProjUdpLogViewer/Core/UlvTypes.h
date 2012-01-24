#ifndef __ULVTYPES__
#define __ULVTYPES__

#include "stdafx.h"
#include "Platform/PlatformTypes.h"
#include "Utils/ErrorsSubsystem.h"

namespace Ulv
{
    using Platform::word;
    using Platform::dword;
    using Platform::ddword;

    // -------------------------------------------------
    
    // Packet data type. Defined for each opened port
    enum UdpPortDataType
    {
        PTString = 0,
        PTWstring = 1,
    };

    // -------------------------------------------------

    // Using inside UdpLogRecordView
    class TimestampView
    {
    public:
        enum TspType
        {
            None = 0,
            SenderSide = 1,
            ReceiverSide = 2,
        };

    public:
        TimestampView() : m_type(None)
        {
        }

        TimestampView(TspType type, const QString &format) :
            m_type(type), m_format(format)
        {
        }

        TspType Type() const { return m_type; }
        const QString& Format() const { return m_format; }

        bool operator==(const TimestampView& other) const
        {
            return ((m_type == other.Type()) && (m_format == other.Format()));
        }

        bool operator!=(const TimestampView& other) const
        {
            return !operator==(other);
        }

    private:

        TspType m_type;
        QString m_format;
    };

    // -------------------------------------------------

    // Wrapper for filter expressions
    class FilterExp
    {
        QString m_exp;
        bool m_caseSens;

    public:
        // expList = "exp1 || exp2 && exp3", "||" --> or, "&&" --> and (has higher priority than "||")
        FilterExp(const QString &exp = "", bool caseSensitive = false) :
            m_exp(exp), m_caseSens(caseSensitive)
        {
        }

        const QString& Get() const { return m_exp; }

        bool CaseSensitive() const { return m_caseSens; }
        Qt::CaseSensitivity CaseSensitiveQt() const { return (m_caseSens)? Qt::CaseSensitive : Qt::CaseInsensitive; }

        bool IsEmpty() const { return m_exp.isEmpty(); }

        bool operator ==(const FilterExp &other) const
        {
            return ((m_exp == other.m_exp) && (m_caseSens == other.m_caseSens));
        }

        bool operator !=(const FilterExp &other) const { return !operator ==(other); }
    };

    // -------------------------------------------------

    // Using for making view of UdpLogRecord. The same for all records in DB.
    class RecordViewSett
    {
        bool m_useSrcInView;
        TimestampView m_timestamp;
        FilterExp m_filter;

    public:
        RecordViewSett() : m_useSrcInView(false)
        {}

        void UsedTimestamp(TimestampView ts) { m_timestamp = ts; }
        const TimestampView& UsedTimestamp() const { return m_timestamp; }

        void UseRecordSrcInView(bool state) { m_useSrcInView = state; }
        bool UseRecordSrcInView() const { return m_useSrcInView; }

        void Filter(const FilterExp &filter) { m_filter = filter; }
        const FilterExp& Filter() const { return m_filter; }

        bool operator ==(const RecordViewSett &other) const
        {
            return ((m_useSrcInView == other.m_useSrcInView)
                    && (m_timestamp == other.m_timestamp)
                    && (m_filter == other.m_filter));
        }

        bool operator !=(const RecordViewSett &other) const { return !operator ==(other); }
    };


} // namespace Ulv

#endif
