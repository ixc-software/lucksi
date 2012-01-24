#ifndef __CTRLTOMODELPARAMS__
#define __CTRLTOMODELPARAMS__

#include "UlvTypes.h"
#include "UdpLogRecordSrc.h"

namespace Ulv
{
    class CtrlToModelParams
    {
        int m_scrollPos;
        bool m_scrollWasAtEnd;

        RecordViewSett m_recSett;

        UdpLogRecordSrcInfoList m_srcList;

    public:
        CtrlToModelParams() : m_scrollPos(0), m_scrollWasAtEnd(true)
        {}

        virtual ~CtrlToModelParams()
        {}

        void Scroll(int pos, bool wasAtEnd)
        {
            m_scrollPos = pos;
            m_scrollWasAtEnd = wasAtEnd;
        }

        int ScrollPos() const { return m_scrollPos; }
        int ScrollWasAtEnd() const { return m_scrollWasAtEnd; }

        void RecordViewWithSrc(bool withSrc) { m_recSett.UseRecordSrcInView(withSrc); }
        void RecordViewTimestamp(TimestampView ts) { m_recSett.UsedTimestamp(ts); }
        void Filter(const FilterExp &filter) { m_recSett.Filter(filter); }

        const RecordViewSett& RecordView() const { return m_recSett; }

        void SrcList(const UdpLogRecordSrcInfoList &list) { m_srcList = list; }
        const UdpLogRecordSrcInfoList& SrcList() const { return m_srcList; }
    };

} // namespace Ulv

#endif
