#include "stdafx.h"
#include "TestRangeStream.h"

#include "FlashEmul.h"
#include "BfBootSrv/RangeIOBase.h"
#include "Utils/Random.h"
#include "Platform/Platform.h"

namespace TestBfBoot
{
    using Platform::byte;
    using BfBootSrv::FlashOStream;
    using BfBootSrv::FlashIStream;
    using BfBootSrv::RangeIOBase;
    using BfBootSrv::SectorMap;
    using BfBootSrv::SectorStorageDeviceBase;
    using BfBootSrv::IDebugRangeIOBase;
    using BfBootSrv::StoreRange;
    using boost::shared_ptr;
    
   
    // Тест чтения/записи с посекторным выравниванием.    
    class TestFileIO
    {
        const SectorMap& m_sectorMap; // Полная карта секторов.
        RangeIOBase& m_rangeStorage;  // Абстракция доступа к флеши посредствам областей.

        std::vector<int> m_fileSizeTable; // Таблица размера сохраняемых файлов.
      
        // -------------------------------------------------------------------------------------

        // Читает файл зарегистрированный в m_fileSizeTable под индексом index.
        // Проверяет его содержимое.
        void Read(int index)
        {
            shared_ptr<FlashIStream> inStream = m_rangeStorage.InStreamAll();            
            ESS_ASSERT(m_fileSizeTable.size() > index);
            for (int i = 0; i < index; ++i)
            {
                inStream->SectorSeek(m_fileSizeTable.at(i));
            }

            std::vector<byte> data;
            inStream->getReader().ReadVectorByte(data, m_fileSizeTable.at(index));

            TUT_ASSERT(data.size() == m_fileSizeTable.at(index));
            for (int i = 0; i < data.size(); ++i)
            {
                TUT_ASSERT( data.at(i) == index );
            }
        }

        // -------------------------------------------------------------------------------------

        // Добавляет файл размером size на носитель и регистрирует его размер в m_fileSizeTable.
        // Заполнитель файла равен его индексу в m_fileSizeTable.
        void AddFile(int size)
        {            
            shared_ptr<FlashOStream> outStream = m_rangeStorage.OutStreamAll();
            for (int i = 0; i < m_fileSizeTable.size(); ++i)
            {
                outStream->SectorSeek(m_fileSizeTable.at(i));
            }

            // заполнитель файла - его порядковый номер
            std::vector<byte> data(size, m_fileSizeTable.size());
            outStream->getWriter().WriteVectorByte(data);            

            m_fileSizeTable.push_back(size);
        }         

    public:
        TestFileIO(RangeIOBase& rangeStorage, const SectorMap& sectorMap)
            : m_sectorMap(sectorMap),
            m_rangeStorage(rangeStorage)
        {
            int i = 0;

            // Добавляется файл занимающий пол сектора            
            AddFile(m_sectorMap.Sector(i++).Size() / 2); 

            // Добавляется файл занимающий целый сектор
            AddFile(m_sectorMap.Sector(i++).Size());                         

            // Добавляется файл занимающий 1.5 сектора                        
            AddFile(m_sectorMap.Sector(i++).Size() + m_sectorMap.Sector(i++).Size() / 2); 

            ESS_ASSERT(m_fileSizeTable.size() == 3);

            // Чтение добавленных файлов и проверка их содержимого.
            for (int i = 0; i < m_fileSizeTable.size(); ++i)
            {
                Read(i);
            }  
        }
    };

    // ------------------------------------------------------------------------------------

    // Тестирование последовательного чтения/записи через границы секторов.
    void RwThroughSectors(const SectorMap& map, Utils::IBinaryWriter& out, Utils::IBinaryReader& in)
    {        
        int restSize = 0;            
        for (int i = 0; i < map.SectorCount(); ++i)
        {
            byte filler = i;
            int halfCurr = map.Sector(i).Size() / 2;                
            std::vector<byte> data(halfCurr + restSize, filler);
            restSize = map.Sector(i).Size() - halfCurr;
            ESS_ASSERT(restSize > 0);

            out.WriteVectorByte(data);

            std::vector<byte> txData;
            in.ReadVectorByte(txData, data.size());
            TUT_ASSERT(txData == data);
        }
    }

    // ------------------------------------------------------------------------------------


    // Проверка распределения регионов и работы метода пишущего потока.
    void TestSizeBeyondCurrPos(RangeIOBase& rangeStorage)
    {
        IDebugRangeIOBase* pDebugIntf = dynamic_cast<IDebugRangeIOBase*>(&rangeStorage);
        ESS_ASSERT(pDebugIntf);

        // Для всех регионов ...
        for (int rangeIndex = RangeIOBase::min + 1; rangeIndex < RangeIOBase::max; ++rangeIndex)
        {	
            //... для всех секторов региона ...
            const StoreRange& range = pDebugIntf->getRange(rangeIndex);
            int offs = 0;
            for (int i = 0; i < range.Map().SectorCount(); ++i)
            {	
                // ... после записи данных в начало i-го сектора
                shared_ptr<FlashOStream> out = rangeStorage.OutStreamRange( RangeIOBase::NamedRangeIndex(rangeIndex) );
                if (i != 0) out->SectorSeek(offs);                    
                out->getWriter().WriteDword(0);
                offs += range.Map().Sector(i).Size();
                
                // должно остатся свободными:
                TUT_ASSERT(out->SizeBeyondCurrPos() == range.Map().FullSize() - offs);
            }
        }
    }

    // ------------------------------------------------------------------------------------

   
    // class vs function - ?
    // Тест выбрасывания исключений при чтении/записи и SectorSeek
    class TestThrow
    {
        RangeIOBase& m_rangeStorage;        
        const StoreRange& m_range;
        RangeIOBase::NamedRangeIndex m_regionIndex;

        void TestSeek()
        {
            bool ok = false;
            try
            {
                shared_ptr<FlashOStream> out = m_rangeStorage.OutStreamRange(m_regionIndex);      
                out->SectorSeek(m_range.Map().FullSize());
            }
            catch (BfBootSrv::NoFreeSectors&)
            { 
                ok = true;
            }
            TUT_ASSERT(ok && "Expected throw NoFreeSectors");
        }
        
        void TestWrite()
        {
            bool ok = false;
            try
            {
                shared_ptr<FlashOStream> out = m_rangeStorage.OutStreamRange(m_regionIndex);     
                
                if (m_range.Map().SectorCount() > 1) // смещаемся на начало последнего сектора
                {
                    out->SectorSeek(m_range.Map().FullSize() - m_range.Map().BackSector().Size());
                }
                std::vector<byte> data( m_range.Map().BackSector().Size() + 1 );
                out->getWriter().WriteVectorByte(data);
            }
            catch (BfBootSrv::OutOfRange&)
            {        
                ok = true;
            }
            TUT_ASSERT(ok);   
        }

        void TestRead()
        {
            bool ok = false;
            try
            {
                shared_ptr<FlashIStream> in = m_rangeStorage.InStreamRange(m_regionIndex);    
                if (m_range.Map().SectorCount() > 1) // смещаемся на начало последнего сектора
                {
                    in->SectorSeek(m_range.Map().FullSize() - m_range.Map().BackSector().Size());
                }                
                std::vector<byte> data( m_range.Map().BackSector().Size() + 1 );
                in->getReader().ReadVectorByte(data, m_range.Map().BackSector().Size() + 1);
            }
            catch (BfBootSrv::OutOfRange&)
            {        
                ok = true;
            }
            TUT_ASSERT(ok);   
        }
    public:
        TestThrow(RangeIOBase& rangeStorage, RangeIOBase::NamedRangeIndex regionIndex)
            : m_rangeStorage(rangeStorage),            
            m_regionIndex(regionIndex), // некоторый регион
            m_range( static_cast<IDebugRangeIOBase&>(rangeStorage).getRange(regionIndex) )            
        {
            TestSeek(); // Выбрасывание исключений при SectorSeek
            TestWrite();// Выбрасывание исключений при записи в поток
            TestRead(); // Выбрасывание исключений при чтении из потока
        }
    };

    // ------------------------------------------------------------------------------------    
   
    void TestRangeStream()
    {           
        BfRangeStorageDevEmul devEmul;        
        TestRangeStream(devEmul.getSectorDev(), devEmul.getRangeIO());                         
    }

    // ------------------------------------------------------------------------------------

    void FillSector(SectorStorageDeviceBase& sectorDev, BfBootSrv::SectorInfo& sector, byte filler)
    {
        std::vector<Platform::byte> data(sector.Size(), filler);
        sectorDev.Write(sector.BeginOffset(), data);
    }

    void TestRangeStream(SectorStorageDeviceBase& sectorDev, RangeIOBase& rangeDev)
    {
        BfBootSrv::SectorInfo sector = sectorDev.getMap().Sector(2);

        FillSector(sectorDev, sector, 'D');
        
        //sectorDev.EraseAll();
        rangeDev.EraseRange(RangeIOBase::CfgPrim);
        
        FillSector(sectorDev, sector, 'x');
        


        // Тестирование последовательного чтения/записи через границы секторов.
        shared_ptr<FlashOStream> outStream = rangeDev.OutStreamAll();
        shared_ptr<FlashIStream> inStream = rangeDev.InStreamAll();
        RwThroughSectors(sectorDev.getMap(), outStream->getWriter(), inStream->getReader());

        // Тест чтения/записи с посекторным выравниванием.
        // (проверка работы SectorSeek)        
        TestFileIO testFileIO(rangeDev, sectorDev.getMap());

        // Проверка распределения регионов и работы метода FlashOStream::SizeBeyondCurrPos()
        TestSizeBeyondCurrPos(rangeDev);

        // Проверка выбрасывания исключений потоками.
        TestThrow testThrow(rangeDev, RangeIOBase::CfgDefault);       
    }
} // namespace TestBfBoot
