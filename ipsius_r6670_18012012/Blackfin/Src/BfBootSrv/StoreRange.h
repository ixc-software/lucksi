#ifndef STORERANGE_H
#define STORERANGE_H

#include "SectorMap.h"

namespace BfBootSrv
{
    

    class StoreRange
    {
        boost::shared_ptr<SectorMap> m_sectors;
        int m_beginIndex;

        friend class RangeIOBase;
        StoreRange(){}
        void Init(const SectorMap& srcMap, int beginIndex, int count)
        {
            ESS_ASSERT(!m_sectors && "Range already inited");
            m_sectors.reset(new SectorMap(srcMap, beginIndex, count));            
            m_beginIndex = beginIndex;
        }

        bool IsIntersected(int beginIndex, int count) // (StoreRange&) ?
        {
            if (!m_sectors) return false;
            int end = beginIndex + count;
            return m_beginIndex <= beginIndex && end <= LastIndex();
        }

        int LastIndex()
        {
            ESS_ASSERT(m_sectors && "Range not inited");
            return m_beginIndex + m_sectors->SectorCount() - 1;
        }

    public:
        const SectorMap& Map() const 
        {
            ESS_ASSERT(m_sectors && "Range not inited");
            return *m_sectors;
        } // как вариант

       
        // return false if skip out of range
        bool JumpSector(SectorInfo& curSector, dword minSkeepSize) const
        {         
            ESS_ASSERT(m_sectors && "Range not inited");

            int currIndex = m_sectors->FindSector(curSector);
            ESS_ASSERT(currIndex != SectorMap::CNotFound);             

            dword endSkip = curSector.BeginOffset() + minSkeepSize;// + 1;
            
            for (int i = currIndex + 1; i < m_sectors->SectorCount(); ++i)
            {
                if (m_sectors->Sector(i).BeginOffset() >= endSkip) 
                {
                    curSector = m_sectors->Sector(i);
                    return true;
                }                
            }
            return false;
        }

        bool NextSector(SectorInfo& curSector) const
        {
            ESS_ASSERT(m_sectors && "Range not inited");

            int currIndex = m_sectors->FindSector(curSector);
            ESS_ASSERT(currIndex != SectorMap::CNotFound);        
            if (m_sectors->SectorCount() <= currIndex + 1) return false;
            curSector = m_sectors->Sector(currIndex + 1);
            return true;
        }

        int SizeBeyondOffs(dword offs) const // размер после указанной позиции с учетом выравнивания
        {
            ESS_ASSERT(m_sectors && "Range not inited");

            int currIndex = m_sectors->FindSectorByOffs(offs);
            if (currIndex == SectorMap::CNotFound) return 0; // or assert
            
            if (m_sectors->Sector(currIndex).BeginOffset() != offs) ++currIndex;

            int result = 0;
            for (int i = currIndex; i < m_sectors->SectorCount(); ++i)
            {
                result += m_sectors->Sector(i).Size();
            }
            return result;            
        }

      
    };
} // namespace BfBootSrv

#endif
