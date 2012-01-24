#ifndef IEID_H
#define IEID_H

#include "Utils/ComparableT.h"
#include "Platform/PlatformTypes.h"

namespace ISDN
{
    using Platform::byte;

    // Идентификатор информационных элементов пакета
    class IeId : public Utils::ComparableT<IeId>
    {
        bool m_empty;
        byte m_id;
        byte m_codeSet;

    public:

        IeId(byte id, byte codeSet);
        IeId();

        void Set(byte id, byte codeSet);
        void Clear();
        int Compare(const IeId &other) const;        
        bool Empty() const { return m_empty; }
        byte Id() const;
        byte Codset() const;
    };
} // namespace ISDN

#endif
