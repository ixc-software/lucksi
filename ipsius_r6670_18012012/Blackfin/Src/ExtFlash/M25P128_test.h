#ifndef __M25P128_TEST__
#define __M25P128_TEST__

#include "M25P128.h"
#include "FlashTest.h"

namespace ExtFlash
{

    class M25P128Test : public IFlashTestIntf
    {
        M25P128	&m_flash;
        FlashMap  m_map;

    // IFlashTestIntf impl
    public:

        const ExtFlash::FlashMap& Map() const
        {
            return m_map;
        }

        bool SectorErase(int sectorNumber)
        {
            if (!m_map.CorrectSector(sectorNumber)) return false;
            m_flash.SectorErase(sectorNumber);        	
            return true;
        }

        bool PageRead(int page, void *p, int size)
        {
            if (m_map.PageSize != size) return false;            
            if (page >= m_map.Pages) return false;

            m_flash.Read(page * m_map.PageSize, p, size);            
            return true;               
        }

        bool PageWrite(int page, const void *p, int size)
        {
            if (m_map.PageSize != size) return false;            
            if (page >= m_map.Pages) return false;

            bool ok = m_flash.Write(page * m_map.PageSize, p, size, true);

            return ok;        	
        }

    public:

        M25P128Test(M25P128	&flash) : 
            m_flash(flash),
        	m_map(m_flash.GetMap())
        {
        }

    };
    
}  // namespace ExtFlash

#endif
