
#include "stdafx.h"

#include "UdpLogRecord.h"
#include "UdpLogFilter.h"
#include "UdpLogDB.h"

namespace
{
    const QString CSeparator = " ";

    // ---------------------------------------------------------------------------

    // \n & \r\n
    void LFPos(const QString &line, int startPos, int &lfPos, int &lfSize)
    {
        lfPos = line.indexOf("\n", startPos);
        lfSize = 1;
        if (lfPos < 0) return;

        int lfPosTmp = line.indexOf("\r", startPos);
        if ((lfPosTmp >= 0) && (lfPosTmp == lfPos - 1))
        {
            ++lfSize;
            lfPos = lfPosTmp;
        }
    }

}

// --------------------------------------------------------------------------

namespace Ulv
{
    // ---------------------------------------------------------------------------
    // UdpLogRecordView impl

    UdpLogRecordView::UdpLogRecordView(const UdpLogRecordInfo &rec,
                                       const RecordViewSett &sett) :
    m_rec(rec), m_startFilterPos(0)
    {
        MakeView(sett);
        SplitViewToLines();
    }

    // ---------------------------------------------------------------------------

    void UdpLogRecordView::AddStaticElemToView(const QString &add,
                                               UdpLogRecordViewRange::Type type,
                                               bool addSeparator)
    {
        int startIndex = m_view.size();
        m_view += add;
        if ((!add.isEmpty()) && (addSeparator)) m_view += CSeparator;
        int count = m_view.size() - startIndex;
        m_ranges.AddStaticElem(startIndex, count, type);
    }

    // ---------------------------------------------------------------------------

    void UdpLogRecordView::MakeView(const RecordViewSett &sett)
    {
        bool addSep = true;
        m_view.clear();

        // make view
        // ([source])[timestamp(serder or receiver)][sessionName][sessionTag][data]
        if (sett.UseRecordSrcInView())
        {
            AddStaticElemToView(SrcName(), UdpLogRecordViewRange::RTSrc, addSep);
        }

        /*
            TODO: convert timestamp once and if only format was changed
        */
        switch(sett.UsedTimestamp().Type())
        {
        case TimestampView::None:
            break;

        case TimestampView::ReceiverSide:
            AddStaticElemToView(m_rec.RecvrTimestamp.toString(sett.UsedTimestamp().Format()),
                                UdpLogRecordViewRange::RTRecvrTimestamp, addSep);
            break;

        case TimestampView::SenderSide:
            {
                QString ts = (m_rec.SenderTimestamp.isValid())?
                             m_rec.SenderTimestamp.toString(sett.UsedTimestamp().Format())
                              : m_rec.SenderTimestampStr;
                AddStaticElemToView(ts, UdpLogRecordViewRange::RTSenderTimestamp, addSep);
            }
            break;

        default: ESS_HALT("Invalid timestamp type");
        }

        m_startFilterPos = m_view.size();

        AddStaticElemToView(m_rec.SessionName, UdpLogRecordViewRange::RTSessionName, addSep);
        AddStaticElemToView(m_rec.SessionTag, UdpLogRecordViewRange::RTSessionTag, addSep);
        AddStaticElemToView(m_rec.Data, UdpLogRecordViewRange::RTData, !addSep);
    }

    // ---------------------------------------------------------------------------

    void UdpLogRecordView::SplitViewToLines() // cut LF, save ranges
    {
        int checkSumm = 0; // to check
        int currLineStart = 0;
        while (currLineStart < m_view.size())
        {
            int currLineEnd = -1;
            int lfSize = 0;
            LFPos(m_view, currLineStart, currLineEnd, lfSize);
            if (currLineEnd < 0) currLineEnd = m_view.size();

            // save size
            int size = currLineEnd - currLineStart;
            checkSumm += size;
            m_lines.push_back(size);
            currLineStart = currLineEnd;

            // remove LF
            m_view.remove(currLineStart, lfSize);            
        }

        ESS_ASSERT(m_view.size() == checkSumm);
        ESS_ASSERT(!m_view.contains("\n"));
    }

    // ---------------------------------------------------------------------------

    // Returns true if record contains such data.
    // 'exp' can be changed from QString to RegExp
    bool UdpLogRecordView::Filter(const FilterExp &filter)
    {
        // drop old filter parts
        m_ranges.DropNonStaticElements();

        bool res = UdpLogFilter::Match(filter,
                                       m_view, m_startFilterPos, SrcEnabled(),
                                       m_ranges);
        return res;
    }

    // ---------------------------------------------------------------------------
    // UdpLogRecord impl

    UdpLogRecord::UdpLogRecord(const QString &data, Utils::SafeRef<UdpLogRecordSrc> src,
                               IUdpPackParser &parser) :
    m_data(src, QDateTime::currentDateTime()),
    m_lastSettingsVer(-1)
    {
        QString pack(data); // .simplified()); // <<---- ignore inner LF

        try
        {
            parser.Parse(pack, m_data);
        }
        catch(LogRecordParsingFailed &e)
        {
            m_data.Data =  pack; // "<packet cannot be parsed>";
        }

        // create view on request only
    }

    // ---------------------------------------------------------------------------

    UdpLogRecord::UdpLogRecord(const UdpLogRecord &other) :
        m_data(other.m_data), m_lastSettingsVer(other.m_lastSettingsVer)
    {
        /* don't copy view */
    }

    // ---------------------------------------------------------------------------

    void UdpLogRecord::operator= (const UdpLogRecord &other)
    {
        m_data = other.m_data;
        m_lastSettingsVer = other.m_lastSettingsVer;
        m_view.reset();
    }

    // ---------------------------------------------------------------------------

    const UdpLogRecordView& UdpLogRecord::View(const RecordViewSett &sett, int settVersion)
    {
        if ((m_view.get() == 0) || 
            (m_lastSettingsVer != settVersion) ||
            (settVersion < 0) )
        {
            m_view.reset(new UdpLogRecordView(m_data, sett)); // recreate
            m_view->Filter(sett.Filter());
            m_lastSettingsVer = settVersion;
        }

        ESS_ASSERT(m_view != 0);
        return *m_view;
    }

    // ---------------------------------------------------------------------------

    // Returns true if record contains such data
    bool UdpLogRecord::Filter(const RecordViewSett &sett)
    {
        // Even if filter empty, we can filter records by source selection!
        // if (sett.Filter().IsEmpty()) return true;

        if (m_view.get() == 0) m_view.reset(new UdpLogRecordView(m_data, sett));

        return m_view->Filter(sett.Filter());
    }

    // ---------------------------------------------------------------------------

} // namespace Ulv

