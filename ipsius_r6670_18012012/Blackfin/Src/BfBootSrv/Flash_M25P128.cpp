#include "stdafx.h"
#include "Flash_M25P128.h"

namespace 
{
    const int CSleepInterval = 50;

    // Предпологаемый максимальный размер образа загрузчика.                       
    const Platform::dword CBootImgReservedSize = 6 *  1024 * 1024;            

} // namespace 

namespace BfBootSrv
{
    void Flash_M25P128::PhRead( dword off, void* data, dword dataSize ) const
    {
        m_extFlash.Read(off, reinterpret_cast<byte*>(data), dataSize);
    }

    // ------------------------------------------------------------------------------------

    void Flash_M25P128::PhWrite(dword off, const void* data, dword dataSize, bool withVerify) // can throw ErrWriteWithoutErase;
    {
        try
        {
        	bool ready = m_extFlash.isReady();
            m_extFlash.Write(off, reinterpret_cast<const byte*>(data), dataSize, withVerify);
        }
        catch (ExtFlash::M25P128VerifyError&) 
        {
            ESS_THROW(SectorStorageDeviceBase::ErrWrite);
        }            
    }

    // ------------------------------------------------------------------------------------

    void Flash_M25P128::PhErase( const SectorInfo &sector )
    {
        m_extFlash.SectorErase(m_sectors.FindSector(sector));
        //WaitExtFlashFree();
    }

    // ------------------------------------------------------------------------------------

    void Flash_M25P128::PhEraseFull()
    {
        m_extFlash.ChipErase();
        //WaitExtFlashFree();
    }

    // ------------------------------------------------------------------------------------

    std::string Flash_M25P128::getDeviceSignature()
    {
        return m_extFlash.GetSignature().ToString();
    }

    // ------------------------------------------------------------------------------------

    Flash_M25P128::Flash_M25P128(const BfDev::SpiBusPoint &point) : 
        m_extFlash(point, true),            
        m_base(m_sectors, *this)
    {
        const ExtFlash::FlashMap& flashMap = m_extFlash.GetMap();            

        // Заполнение собственной карты секторов.
        for (int i = 0; i < flashMap.Sectors; ++i)
        {
            m_sectors.AddSector(flashMap.SectorSize);
        }
        ESS_ASSERT(m_sectors.FullSize() == flashMap.TotalBytes);

        // Распределение абстракции регионов по имеющимся секторам.
        m_rangeBase.reset(new RangeIOBase(m_base));
        
        Platform::dword bootImgSectorsCount = CBootImgReservedSize / flashMap.SectorSize;
        if (CBootImgReservedSize % flashMap.SectorSize) ++bootImgSectorsCount;

        ESS_ASSERT(bootImgSectorsCount + RangeIOBase::max <= flashMap.Sectors);

        ESS_ASSERT(flashMap.SectorSize > 4 * 1024 && "Size of range CfgXXX must be greater then 4KB");
        m_rangeBase->BindRegion(RangeIOBase::CfgPrim, bootImgSectorsCount, 1);
        m_rangeBase->BindNextRegion(RangeIOBase::CfgSecond, 1);
        m_rangeBase->BindNextRegion(RangeIOBase::CfgDefault, 1);

        // Остальные сектора отдаем AppStorRange.
        m_rangeBase->BindNextRegion(
            RangeIOBase::AppStorRange,
            flashMap.Sectors - bootImgSectorsCount - 3
            );
    }

    // ------------------------------------------------------------------------------------

    RangeIOBase& Flash_M25P128::getRangeIO()
    {
        ESS_ASSERT(m_rangeBase);
        return *m_rangeBase;
    }

    // ------------------------------------------------------------------------------------

    void Flash_M25P128::WaitExtFlashFree()
    {
        while (m_extFlash.isBusy())
        {
            Platform::ThreadSleep(CSleepInterval);
        }
    }

} // namespace BfBootSrv

