#ifndef FLASHEMUL_H
#define FLASHEMUL_H

#include "BfBootSrv/RangeIOBase.h"
#include "BfBootSrv/SectorStorageDeviceBase.h"

namespace BfEmul
{
    using Platform::byte;
    using Platform::dword;

	class BfSectorDevEmul : boost::noncopyable,
		public BfBootSrv::IPhisicalSectorDev
    {
        mutable QFile m_file;
        BfBootSrv::SectorMap m_sectors;
        BfBootSrv::SectorStorageDeviceBase m_base;

        void EraseEmulation(dword off, dword size);

        void AssertSize(dword off, dword size) const; // избыточно

    // IPhisicalSectorDev
    private:

        // Чтение в data запрошенного размера с носителя.
        void PhRead(dword off, void* data, dword dataSize) const;

        // Запись данных на носитель.
        // Сектор соответствующий смещению должен быть предварительно стерт (проверяется чтением-сравнением)!
        void PhWrite(dword off, const void* data, dword dataSize, bool withVerify);// can throw ErrWriteWithoutErase;

        void PhErase(const BfBootSrv::SectorInfo &sector);
        void PhEraseFull();

        std::string getDeviceSignature() { return "BfSectorDevEmulation";}

    public:

        BfSectorDevEmul(QString fileName = "defaultFlash.bin", bool cleanDump = true);       
        //~BfSectorDevEmul();
        BfBootSrv::SectorStorageDeviceBase& get() { return m_base; }

    };

    // -------------------------------------------------------------------------------------

	class BfRangeStorageDevEmul : boost::noncopyable
    {
        BfSectorDevEmul m_flash;
        BfBootSrv::RangeIOBase m_base;
    public:

        BfRangeStorageDevEmul(QString fileName = "defaultFlash.bin", bool erase = false);

        BfBootSrv::RangeIOBase& getRangeIO() { return m_base;}
        BfBootSrv::SectorStorageDeviceBase& getSectorDev() { return m_flash.get(); }
    };
    
} // namespace BfEmul

#endif
