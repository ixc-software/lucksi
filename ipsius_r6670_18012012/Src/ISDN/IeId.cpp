#include "stdafx.h"
#include "IeId.h"
#include "Utils/ErrorsSubsystem.h"

namespace ISDN
{

    IeId::IeId( byte id, byte codeSet )
    {
        Set(id, codeSet);
    }

    IeId::IeId()
    {
        Clear();
    }

    void IeId::Set( byte id, byte codeSet )
    {
        m_empty = false; 
        m_id = id; 
        m_codeSet = codeSet;
    }

    void IeId::Clear()
    {
        m_empty = true;
    }

    int IeId::Compare( const IeId &other ) const
    {
        ESS_ASSERT(!m_empty);

        int cmp = Utils::CompareToInt(m_id, other.m_id);
        if (cmp != 0) return cmp;
        return Utils::CompareToInt(m_codeSet, other.m_codeSet);
    }

    byte IeId::Id() const
    {
        ESS_ASSERT(!m_empty);
        return m_id;
    }

    byte IeId::Codset() const
    {
        ESS_ASSERT(!m_empty);
        return m_codeSet;
    }
} // namespace ISDN
