#include "stdafx.h"
#include "RangeIOBase.h"

namespace BfBootSrv
{    
    shared_ptr<FlashIStream> RangeIOBase::InStreamRange( NamedRangeIndex index )
    {
        AssertRegion(index);
        return getIn(m_map.at(index));
    }

    // ------------------------------------------------------------------------------------

    shared_ptr<FlashOStream> RangeIOBase::OutStreamRange( NamedRangeIndex index )
    {
        AssertRegion(index);
        return getOut(m_map.at(index));
    }

    // ------------------------------------------------------------------------------------

    shared_ptr<FlashIStream> RangeIOBase::InStreamAll()
    {
        return getIn(m_totalRange);
    }

    // ------------------------------------------------------------------------------------

    shared_ptr<FlashOStream> RangeIOBase::OutStreamAll()
    {
        return getOut(m_totalRange);
    }

    // ------------------------------------------------------------------------------------

    int RangeIOBase::getFullSize() const
    {
        return m_flash.getMap().FullSize();
    }

    // ------------------------------------------------------------------------------------

    void RangeIOBase::EraseRange( NamedRangeIndex regionIndex )
    {
        AssertRegion(regionIndex);


        for (int i = 0; i < m_map.at(regionIndex).Map().SectorCount(); ++i)
        {
            m_flash.EraseSector(m_map.at(regionIndex).Map().Sector(i));
        }
    }

    // ------------------------------------------------------------------------------------

    RangeIOBase::RangeIOBase( SectorStorageDeviceBase& flash ) : m_lastAdded(min),
        m_flash(flash)
    {
        for (int i = min + 1; i < max; ++i)
        {
            m_map.push_back(StoreRange());
        }
        m_totalRange.Init(m_flash.getMap(), 0, m_flash.getMap().SectorCount());
    }

    // ------------------------------------------------------------------------------------

    void RangeIOBase::BindNextRegion( NamedRangeIndex region, int sectorCount )
    {
        int last = m_lastAdded;
        ESS_ASSERT(last + 1 == region);

        BindRegion(region, m_map.at(last).LastIndex() + 1, sectorCount);
    }

    // ------------------------------------------------------------------------------------

    void RangeIOBase::BindRegion( NamedRangeIndex region, int startSector, int sectorCount )
    {
        AssertRegion(region);
        // assert no intersection
        for (int i = 0; i < max; ++i)
        {
            if (i != region)ESS_ASSERT( !m_map.at(i).IsIntersected(startSector, sectorCount) );
        }

        m_map.at(region).Init(m_flash.getMap(), startSector, sectorCount);
        m_lastAdded = region;
    }

    // ------------------------------------------------------------------------------------

    shared_ptr<FlashOStream> RangeIOBase::getOut( const StoreRange& range )
    {
        return shared_ptr<FlashOStream>(
            new FlashOStream(&m_flash, range)
            );
    }

    // ------------------------------------------------------------------------------------

    shared_ptr<FlashIStream> RangeIOBase::getIn( const StoreRange& range ) const
    {
        return shared_ptr<FlashIStream>(
            new FlashIStream(&m_flash, range)
            );
    }

    // ------------------------------------------------------------------------------------

    void RangeIOBase::AssertRegion( NamedRangeIndex region ) const
    {
        ESS_ASSERT(region > min &&  region < max);
    }

    // ------------------------------------------------------------------------------------

    const StoreRange& RangeIOBase::getRange( int index )
    {
        ESS_ASSERT(index > min &&  index < max); 
        return m_map.at(index);
    }
} // namespace BfBootSrv

