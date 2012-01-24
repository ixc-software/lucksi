
#ifndef __UDPLOGRECORD__
#define __UDPLOGRECORD__

// UdpLogRecord.h

#include "IUdpPackParser.h"
#include "UdpLogRecordSrc.h"
#include "UlvTypes.h"
#include "UdpLogRecordInfo.h"
#include "UdpLogRecordViewRange.h"

#include "Utils/HostInf.h"
#include "iNet/SocketData.h"


namespace Ulv
{
    // View for received data
    // Note: isn't using timestamp in filter
    class UdpLogRecordView :
        // public IUdpLogRecordView,
        boost::noncopyable
    {
        // data
        const UdpLogRecordInfo &m_rec;

        // view
        QString m_view;
        UdpLogRecordViewRangesList m_ranges;

        // timestamp and source aren't using in filter
        int m_startFilterPos;
        std::vector<int> m_lines;

        void MakeView(const RecordViewSett &sett);
        void SplitViewToLines(); // cut LF, save ranges
        void AddStaticElemToView(const QString &add, UdpLogRecordViewRange::Type type,
                                 bool addSeparator);

    public:
        UdpLogRecordView(const UdpLogRecordInfo &rec, const RecordViewSett &sett);

        const QString& Get() const { return m_view; }

        // const UdpLogRecordSrc& Source() const { return m_rec.Source; }
        QString SrcName() const { return m_rec.Source->UniqueName(); }
        bool SrcEnabled() const { return m_rec.Source->Enabled(); }

        const UdpLogRecordViewRangesList& PartsRanges() const { return m_ranges; }
        const std::vector<int>& LinesSizes() const { return m_lines; }

        bool Filter(const FilterExp &filter);
    };

    // -----------------------------------------------------------------

    // Wrapper for received UDP packet
    class UdpLogRecord // copyable
    {
        UdpLogRecordInfo m_data;
        boost::shared_ptr<UdpLogRecordView> m_view;

        // to check version and update view if need
        int m_lastSettingsVer;

    public:
        UdpLogRecord(const QString &data, Utils::SafeRef<UdpLogRecordSrc> src,
                     IUdpPackParser &parser); // can throw

        UdpLogRecord(const UdpLogRecord &other);

        void operator= (const UdpLogRecord &other);

        // const UdpLogRecordSrc& Source() const { return m_data.Source; }
        // const QString& Source() const { return m_data.Source.; }

        const UdpLogRecordView& View(const RecordViewSett &sett, int settVersion);
        bool IsSuccessfullyParsed() const { return m_data.IsSuccessfullyParsed; }

        // Returns true if record contains such a data
        bool Filter(const RecordViewSett &sett);
    };

} // namespace Ulv


#endif
