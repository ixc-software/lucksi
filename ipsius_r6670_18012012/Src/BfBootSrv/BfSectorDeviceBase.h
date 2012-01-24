#ifndef BFSECTORDEVICEBASE_H
#define BFSECTORDEVICEBASE_H

namespace BfBootSrv
{
    
    // Информация о секторе
    class SectorInfo
    {            
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
    };

    // -----------------------------------------------------

    class SectorMap : boost::noncopyable
    {
        std::vector<SectorInfo> m_map;

    public:

        int SectorCount() const
        {}

        int FullSize() const
        {}

        const SectorInf& Sector(int index) const
        {}

        void AddSector(int sectorSize) // no const!
        {}

        bool ValidateRange(dword off, dword length) const
        {}
    };

    // -----------------------------------------------------

    class BfSectorDeviceBase : boost::noncopyable
    {
        SectorMap m_sectors;

    protected:       

        void AddSector(dword size)
        {
            //...
        }
    
    public:

        ESS_TYPEDEF(Err);    // todo связать с кодами ответов сервера      
        ESS_TYPEDEF_FULL(ErrOutOfRange, Err);
        ESS_TYPEDEF_FULL(ErrWriteWithoutErase, Err);

        // Заменяет содержимое data прочтенными данными.
        void Read(dword off, std::vector<byte>& data, dword dataSize) const // can throw ErrOutOfRange
        {            
            data.resize(dataSize);
            if (dataSize == 0) return;
            Read(off, &data[0], dataSize);                         
        }

        // Чтение в data запрошенного размера.
        void Read(dword off, void* data, dword dataSize) const // can throw ErrOutOfRange
        {
            ESS_ASSERT(m_sectors.SectorCount() > 0);
            if (!m_sectors.ValidateRange(off, dataSize)) ESS_THROW(ErrOutOfRange);
            PhRead(dword off, void* data, dword dataSize);
        }

        // Записать data по смещению off
        void Write(dword off, const std::vector<byte>& data) // can throw ErrOutOfRange, ErrWriteWithoutErase
        {
            Write(off, &data[0], data.size());
        }

        void Write(dword off, void const * const data, dword dataSize, bool withVerify) // can throw ErrOutOfRange, ErrWriteWithoutErase
        {
            ESS_ASSERT(m_sectors.SectorCount() > 0);
            if (!m_sectors.ValidateRange(off, dataSize)) ESS_THROW(ErrOutOfRange);
            PhWrite(dword off, void const * const data, dword dataSize);
        }

        void EraseSector(const SectorInfo& sector)
        {
            ESS_ASSERT(m_sectors.SectorCount() > 0);
            ESS_ASSERT(sector.BeginOffset < m_sectors.getFullSize()); // ассерт если запрещено изменение сектора вне карты
            PhErase(...);
        }

        void EraseAll()
        {
            ESS_ASSERT(m_sectors.SectorCount() > 0);
            PhErase(m_sectors.getSector(0).BeginOffset, m_sectors.getFullSize());
        }

        int SectorCount() const
        {
            return m_sectors.SectorCount();
        }

        int getFullSize() const
        {
            return m_sectors.getFullSize();
        }

        const SectorInfo& getSector(int index) const
        {
            return m_sectors;
        }


    // overrides
    private:

        // Чтение в data запрошенного размера с носителя.
        virtual void PhRead(dword off, void* data, dword dataSize) const = 0;        

        // Запись данных на носитель.
        // Сектор соответствующий смещению должен быть предварительно стерт (проверяется чтением-сравнением)!
        virtual void PhWrite(dword off, const void* data, dword dataSize, bool withVerify) = 0; // can throw ErrWriteWithoutErase        
       
        virtual void PhErase(SectorInfo &sector) = 0;
        virtual void PhEraseFull() = 0;

    };
    
    // ------------------------------------------------------------------------------------

    // название потоков ?

    class IStreamIn : Utils::IBasicInterface
    {};
    class IStreamOut : Utils::IBasicInterface
    {};

    Utils::BinaryReader, Utils::BinaryWriter -> выделить виртуальные интерфейсы - !!

    class FlashIStream : public IStreamIn
    {
        typedef Utils::BinaryReader<FlashIStream> Reader;

        SafeRef<BfSectorDeviceBase> m_flash;
        Reader m_reader;

    // stream semantic for Reader
    public:
        byte ReadByte(); // can throw xxx
        void Read(void *pDest, size_t length); // can throw xxx    

    public:
        FlashIStream(BfSectorDeviceBase&, StoreRange range);
        Reader& getReader(){ return m_reader; }

        void SectorSeek(dword minSkipBytes);
    };

    class FlashOStream : public IStreamOut
    {
        typedef Utils::BinaryWriter<FlashOStream> Writer;  

        SafeRef<BfSectorDeviceBase> m_flash;
        Writer m_writer;

    // stream semantic for Writer
    public:        
        void WriteByte(byte b);       
        void Write(const void *pSrc, size_t length);

    public:

        FlashOStream(BfSectorDeviceBase&, StoreRange range);        
        Writer& getWriter() {return m_writer;}

        void SectorSeek(dword minSkipBytes);

        int SizeBeyondCurrPos() const;
    };

    // -----------------------------------------------------

    class StoreRange
    {
        int m_beginIndex;  // Индекс сектора в карте секторов   
        int m_sectorCount; // Количество последовательно расположеннных секторов в данной области            

        friend class RangeIOBase;
        StoreRange(int,int);

    public:
        int Count()const;
        int BeginIndex() const;
    };

    // -----------------------------------------------------

    // Заведует разметкой флеши на области и доступом посредством потоков
    class RangeIOBase : boost::noncopyable
    {      
        
    public:

        enum NamedRangeIndex
        {            
            CfgPrim = 0,
            CfgSecond,
            CfgDefault,
            AppStorage,
            TotalRange,

            CRangeCount  // ?
        };               

        shared_ptr<IStreamIn> CreateStreamIn(NamedRangeIndex index)
        {
            new FlashIStream(m_flash, m_rangeMap.at(index));
        }

        shared_ptr<IStreamOut> CreateStreamOut(NamedRangeIndex index) const
        {
            new FlashOStream(m_flash, m_rangeMap.at(index));
        }

        dword getRangeSize(NamedRangeIndex index) const
        {
            return m_rangeMap.at(index);
        }

    protected:
        // Принимает флешь во владение
        RangeIOBase(BfSectorDeviceBase* pFlash)
        {
            ESS_ASSERT(pFlash != 0);
            m_flash.reset(pFlash);
        }

        void BindRegion(NamedRangeIndex regionIndex, int sectorCount)
        {            
        }


    private:
        boost::scoped_ptr<BfSectorDeviceBase> m_flash;
        //std::map<NamedRangeIndex, StoreRange>  m_rangeMap;
        std::vector<StoreRange>  m_rangeMap;

    };

    // ------------------------------------------------------------------------------------

    // Задача - только настройка разбивки на регионы.
    class BootFlash : public RangeIOBase
    {
    public:
        // Принимает флешь во владение
        BootFlash(BfSectorDeviceBase* pFlash) : RangeIOBase(pFlash)
        {
            BindRegion();
        }
    };
    

   
} // namespace BfBootSrv

#endif
