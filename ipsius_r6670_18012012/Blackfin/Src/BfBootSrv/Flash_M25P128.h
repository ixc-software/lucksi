#ifndef FLASH_M25P128_H
#define FLASH_M25P128_H


#include "BfDev/SysProperties.h"
#include "ExtFlash/M25P128.h"
#include "BfBootSrv/RangeIOBase.h"
#include "BfDev/SpiBusMng.h"

#include "SectorStorageDeviceBase.h"

namespace BfBootSrv
{

    // Нужна возможность гибкой замены драйвера ?
    // Вынести все константы типа CBootImgSize итп в отдельны хидер. Проверять ассертом соответствие с ExtFlash::M25P128::getMap - ?
    // Это даст возможность синхронизировать параметры эмуляции с реально используемой флешю.

    using Platform::dword;    
    
    /*
        Полная емкость: 16 777 216 байт (16MB)
        Размер сектора: 262144 байт
        Количество секторов: 64
        
        Скорость чтения: speedKHz * 100 байт/сек - скорость передачи по шине spi 
        Скорость записи: speedKHz * 100 байт/сек (+ 7мсек на каждую страницу 256байт) 
        Время стирания сектора: 4сек
        Время стирания всей флеши: 256сек        
    */

    class Flash_M25P128 : public IPhisicalSectorDev
    {                
        mutable ExtFlash::M25P128 m_extFlash;
        SectorMap m_sectors;
        SectorStorageDeviceBase m_base;
        scoped_ptr<RangeIOBase> m_rangeBase;

    // own
    private:
        void WaitExtFlashFree();               

    // IPhisicalSectorDev
    private:
        // Чтение в data запрошенного размера с носителя.
        void PhRead(dword off, void* data, dword dataSize) const;
        // Запись данных на носитель.
        // Сектор соответствующий смещению должен быть предварительно стерт (проверяется чтением-сравнением)!
        void PhWrite(dword off, const void* data, dword dataSize, bool withVerify); // can throw ErrWriteWithoutErase
        void PhErase(const SectorInfo &sector);
        void PhEraseFull();
        std::string getDeviceSignature();

    public:        

        Flash_M25P128(const BfDev::SpiBusPoint &point); 
        RangeIOBase& getRangeIO();
        
        /*SectorStorageDeviceBase& getSectorDev()
        {
            return m_base;
        }*/        
    };

    
} // namespace BfBootSrv

#endif
