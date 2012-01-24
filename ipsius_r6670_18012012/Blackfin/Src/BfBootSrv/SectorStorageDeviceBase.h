#ifndef SECTORSTORAGEDEVICEBASE_H
#define SECTORSTORAGEDEVICEBASE_H

#include "SectorInfo.h"
#include "SectorMap.h"
#include "Utils/SafeRef.h"

namespace BfBootSrv
{
    using Platform::byte;
    using Platform::dword;

    // »нтерфейс физического носител€ данных.
    class IPhisicalSectorDev : Utils::IBasicInterface
    {
    public:       
        // „тение в data запрошенного размера с носител€.
        virtual void PhRead(dword off, void* data, dword dataSize) const = 0;        

        // «апись данных на носитель.
        // —ектор соответствующий смещению должен быть предварительно стерт (провер€етс€ чтением-сравнением)!
        virtual void PhWrite(dword off, const void* data, dword dataSize, bool withVerify) = 0; // can throw ErrWrite        

        virtual void PhErase(const SectorInfo &sector) = 0;
        virtual void PhEraseFull() = 0;

        virtual std::string getDeviceSignature() = 0;
    };

    // ------------------------------------------------------------------------------------

    class SectorStorageDeviceBase : 
        boost::noncopyable,
        public virtual Utils::SafeRefServer
    {
        const SectorMap& m_sectors;
        IPhisicalSectorDev& m_devImpl;    

    public:

        ESS_TYPEDEF(Err);    // todo св€зать с кодами ответов сервера      
        ESS_TYPEDEF_FULL(ErrOutOfRange, Err);
        ESS_TYPEDEF_FULL(ErrWrite, Err); // ќшибка записи. ¬озможна€ причина запись в нестертый сектор. 

        // «амен€ет содержимое data прочтенными данными.
        void Read(dword off, std::vector<byte>& data, dword dataSize) const // can throw ErrOutOfRange
        {            
            data.resize(dataSize);
            if (dataSize == 0) return;
            Read(off, &data[0], dataSize);                         
        }

        // „тение в data запрошенного размера.
        void Read(dword off, void* data, dword dataSize) const // can throw ErrOutOfRange
        {
            ESS_ASSERT(m_sectors.SectorCount() > 0);
            if (!m_sectors.IsValidateRange(off, dataSize)) ESS_THROW(ErrOutOfRange);
            m_devImpl.PhRead(off, data, dataSize);
        }

        // «аписать data по смещению off
        void Write(dword off, const std::vector<byte>& data, bool withVerify = true) // can throw ErrOutOfRange, ErrWrite
        {
            Write(off, &data[0], data.size(), withVerify);
        }

        void Write(dword off, void const * const data, dword dataSize, bool withVerify = true) // can throw ErrOutOfRange, ErrWrite
        {
            if (dataSize == 0) return;
            ESS_ASSERT(m_sectors.SectorCount() > 0);
            if (!m_sectors.IsValidateRange(off, dataSize)) ESS_THROW(ErrOutOfRange);
            m_devImpl.PhWrite(off, data, dataSize, withVerify);
        }


        void EraseSector(int sectorIndex)
        {
            ESS_ASSERT(m_sectors.SectorCount() > sectorIndex);
            EraseSector(m_sectors.Sector(sectorIndex));
        }

        void EraseSector(const SectorInfo& sector)
        {
            ESS_ASSERT(m_sectors.SectorCount() > 0);
            ESS_ASSERT(sector.BeginOffset() < m_sectors.FullSize());
            m_devImpl.PhErase(sector);
        }

        void EraseAll()
        {
            ESS_ASSERT(m_sectors.SectorCount() > 0);
            m_devImpl.PhEraseFull();
        }
        
        const SectorMap& getMap() const
        {            
            return m_sectors;
        }

        SectorStorageDeviceBase(const SectorMap& sectors, IPhisicalSectorDev& devImpl) 
            : m_sectors(sectors), m_devImpl(devImpl)
        {}

    };
} // namespace BfBootSrv

#endif
