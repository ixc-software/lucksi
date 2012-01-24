#ifndef SECTORMAP_H
#define SECTORMAP_H

#include "stdafx.h"
//cpp
#include "SectorInfo.h"
#include "Utils/ErrorsSubsystem.h"

namespace BfBootSrv
{
    class SectorMap : boost::noncopyable
    {
        std::vector<SectorInfo> m_map;
        int m_size;

    public:

        SectorMap() : m_size(0)
        {}

        // Create copy of segment assigned beginIndex and count
        SectorMap(const SectorMap& srcMap, int beginIndex, int count) : m_size(0)
        {
            ESS_ASSERT(beginIndex >= 0 && count >= 0);
            ESS_ASSERT(m_map.empty()); // or erase ?
            int end = beginIndex + count;
            ESS_ASSERT(end <= srcMap.SectorCount());
            for (int i = beginIndex; i < end; ++i) // todo subfunction
            {
                m_map.push_back(srcMap.Sector(i));
                m_size += srcMap.Sector(i).Size();
            }
        }


        int SectorCount() const
        {
            return m_map.size();
        }

        int FullSize() const
        {
            return m_size;
        }

        const SectorInfo& Sector(int index) const // toto operator[]
        {
            ESS_ASSERT(index < SectorCount());
            return m_map.at(index);
        }

        const SectorInfo& BackSector() const
        {
            ESS_ASSERT(!m_map.empty());
            return m_map.back();
        }
        const SectorInfo& FrontSector()const
        {
            ESS_ASSERT(!m_map.empty());
            return m_map.front();
        }

        void AddSector(int sectorSize)
        {
            dword offs =  0; 
            if (!m_map.empty())
            {
                SectorInfo last = m_map.back();
                offs = last.BeginOffset() + last.Size();
            }
                        
            m_map.push_back( SectorInfo(offs, sectorSize) );
            m_size += sectorSize;
        }       

        bool IsValidateRange(dword off, dword size) const
        {
            //return (off + size) <= m_size;
            return (off + size) <= m_map.back().LastIndex() + 1;
        }

        enum{ CNotFound = -1};
        int FindSector(const SectorInfo& sector) const 
        {
            for (int i = 0; i < m_map.size(); ++i)
            {
                if (sector == m_map.at(i)) return i;
            }
            return CNotFound;
        }
        int FindSectorByOffs(dword offs) const
        {
            for (int i = 0; i < m_map.size(); ++i)
            {
                if (m_map.at(i).BeginOffset() <= offs &&  offs < m_map.at(i).LastIndex()) return i;
            }
            return CNotFound;
        }
    };
} // namespace BfBootSrv

#endif

