#ifndef __RECORDVIEWRANGE__
#define __RECORDVIEWRANGE__

#include "Utils/ManagedList.h"
#include "Utils/ErrorsSubsystem.h"

namespace Ulv
{
    // Record view part's parameters
    class UdpLogRecordViewRange
    {
        Q_ENUMS(RangeType);

    public:
        enum Type
        {
            RTSrc = 0,
            RTSenderTimestamp = 1,
            RTRecvrTimestamp = 2,
            RTSessionName = 3,
            RTSessionTag = 4,
            RTData = 5,
            RTFilter = 6,
            // RTOther = 7, // i.e. separators
        };

    public:
        UdpLogRecordViewRange(int startIndex, int count, Type type) :
            m_startIndex(startIndex), m_count(count), m_type(type)
        {

        }

        bool Enter(int index) const { return (m_startIndex == index); }
        bool Leave(int index) const { return (index == (m_startIndex + m_count)); }
        // RangeType Type() const  { return m_type; }

        Type RangeType() const { return m_type; }
        void RangeType(Type type) { m_type = type; }

    private:
        int m_startIndex;
        int m_count;
        Type m_type;
    };

    // --------------------------------------------------------

    // List of record view parts' parameters
    class UdpLogRecordViewRangesList :
        boost::noncopyable
    {
        Utils::ManagedList<UdpLogRecordViewRange> m_list;
        size_t m_staticElemCount; // elements that won't be removed from the list

    public:
        UdpLogRecordViewRangesList() : m_list(true), m_staticElemCount(0)
        {}

        // An element that won't be removed from the list
        // until list destruction(i.e. source, data, tag ranges).
        // They should be put one after another from the begin of the list
        void AddStaticElem(int startIndex, int count, UdpLogRecordViewRange::Type type);

        // Removable elements. They should be put after static elements
        void Add(int startIndex, int count, UdpLogRecordViewRange::Type type);
        // other must contain only non-static elements
        void Add(const UdpLogRecordViewRangesList &other);
        void Add(const UdpLogRecordViewRange &other);

        // All elements after staticCount
        void DropNonStaticElements();
        void DropAllFilterRanges();

        size_t Size() const { return m_list.Size(); }
        bool IsEmpty() const { return m_list.IsEmpty(); }
        size_t StaticElementsCount() const { return m_staticElemCount; }
        const UdpLogRecordViewRange& operator[](size_t index) const;
    };

} // namespace Ulv

#endif
