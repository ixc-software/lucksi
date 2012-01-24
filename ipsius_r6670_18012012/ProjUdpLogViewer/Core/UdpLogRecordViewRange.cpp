
#include "stdafx.h"
#include "UdpLogRecordViewRange.h"

namespace Ulv
{
    // An element that won't be removed from the list
    // until list destruction(i.e. source, data, tag ranges).
    // They should be put one after another from the begin of the list
    void UdpLogRecordViewRangesList::AddStaticElem(int startIndex, int count, UdpLogRecordViewRange::Type type)
    {
        ESS_ASSERT(m_staticElemCount == Size());
        Add(startIndex, count, type);
        ++m_staticElemCount;
    }

    // -------------------------------------------------------------------

    // Removable elements. They should be put after static elements
    void UdpLogRecordViewRangesList::Add(int startIndex, int count, UdpLogRecordViewRange::Type type)
    {
        ESS_ASSERT(Size() >= StaticElementsCount());
        m_list.Add(new UdpLogRecordViewRange(startIndex, count, type));
    }

    // -------------------------------------------------------------------

    void UdpLogRecordViewRangesList::Add(const UdpLogRecordViewRangesList &other)
    {
        ESS_ASSERT(!other.StaticElementsCount());
        for (int i = 0; i < other.Size(); ++i)
        {
            Add(other[i]);
        }
    }

    // -------------------------------------------------------------------

    void UdpLogRecordViewRangesList::Add(const UdpLogRecordViewRange &other)
    {
        ESS_ASSERT(Size() >= StaticElementsCount());
        m_list.Add(new UdpLogRecordViewRange(other));
    }

    // -------------------------------------------------------------------

    // All elements after staticCount
    void UdpLogRecordViewRangesList::DropNonStaticElements()
    {
        for (size_t i = (Size() - 1); i >= m_staticElemCount; --i)
        {
            m_list.Delete(i);
        }

        ESS_ASSERT(StaticElementsCount() == Size());
    }

    // -------------------------------------------------------------------

    void UdpLogRecordViewRangesList::DropAllFilterRanges()
    {
        int i = m_list.Size() - 1;
        while(i >= 0)
        {
            if (m_list[i]->RangeType() == UdpLogRecordViewRange::RTFilter) { m_list.Delete(i); }
            --i;
        }
    }

    // -------------------------------------------------------------------

    const UdpLogRecordViewRange& UdpLogRecordViewRangesList::operator[](size_t index) const
    {
        ESS_ASSERT(index < m_list.Size());
        return *(m_list[index]);
    }

    // -------------------------------------------------------------------

} // namespace Ulv

