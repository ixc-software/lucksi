#ifndef SECTORINFO_H
#define SECTORINFO_H

#include "Platform/PlatformTypes.h"

namespace BfBootSrv
{
    using Platform::dword;

    // Информация о секторе
    class SectorInfo
    {     
        //int m_number;
        dword m_beginOffset;        
        dword m_size;              

        friend class SectorMap;
        SectorInfo(dword beginOffset, dword size)
            : m_beginOffset(beginOffset),
            m_size(size)               
        {}

    public:
        dword BeginOffset() const
        {
            return m_beginOffset;
        }
        dword Size() const
        {
            return m_size;
        }
        dword LastIndex() const // todo rename LastOffset
        {
            return m_size + m_beginOffset - 1;
        }

        bool operator ==(const SectorInfo& rhs) const
        {
            return 
                m_beginOffset == rhs.m_beginOffset 
             && m_size == rhs.m_size;
        }

        bool operator !=(const SectorInfo& rhs) const
        {
            return ! operator==(rhs);
        }
    };
} // namespace BfBootSrv

#endif
