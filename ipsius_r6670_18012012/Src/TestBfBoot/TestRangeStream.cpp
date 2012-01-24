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
    
   
    // ���� ������/������ � ����������� �������������.    
    class TestFileIO
    {
        const SectorMap& m_sectorMap; // ������ ����� ��������.
        RangeIOBase& m_rangeStorage;  // ���������� ������� � ����� ����������� ��������.

        std::vector<int> m_fileSizeTable; // ������� ������� ����������� ������.
      
        // -------------------------------------------------------------------------------------

        // ������ ���� ������������������ � m_fileSizeTable ��� �������� index.
        // ��������� ��� ����������.
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

        // ��������� ���� �������� size �� �������� � ������������ ��� ������ � m_fileSizeTable.
        // ����������� ����� ����� ��� ������� � m_fileSizeTable.
        void AddFile(int size)
        {            
            shared_ptr<FlashOStream> outStream = m_rangeStorage.OutStreamAll();
            for (int i = 0; i < m_fileSizeTable.size(); ++i)
            {
                outStream->SectorSeek(m_fileSizeTable.at(i));
            }

            // ����������� ����� - ��� ���������� �����
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

            // ����������� ���� ���������� ��� �������            
            AddFile(m_sectorMap.Sector(i++).Size() / 2); 

            // ����������� ���� ���������� ����� ������
            AddFile(m_sectorMap.Sector(i++).Size());                         

            // ����������� ���� ���������� 1.5 �������                        
            AddFile(m_sectorMap.Sector(i++).Size() + m_sectorMap.Sector(i++).Size() / 2); 

            ESS_ASSERT(m_fileSizeTable.size() == 3);

            // ������ ����������� ������ � �������� �� �����������.
            for (int i = 0; i < m_fileSizeTable.size(); ++i)
            {
                Read(i);
            }  
        }
    };

    // ------------------------------------------------------------------------------------

    // ������������ ����������������� ������/������ ����� ������� ��������.
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


    // �������� ������������� �������� � ������ ������ �������� ������.
    void TestSizeBeyondCurrPos(RangeIOBase& rangeStorage)
    {
        IDebugRangeIOBase* pDebugIntf = dynamic_cast<IDebugRangeIOBase*>(&rangeStorage);
        ESS_ASSERT(pDebugIntf);

        // ��� ���� �������� ...
        for (int rangeIndex = RangeIOBase::min + 1; rangeIndex < RangeIOBase::max; ++rangeIndex)
        {	
            //... ��� ���� �������� ������� ...
            const StoreRange& range = pDebugIntf->getRange(rangeIndex);
            int offs = 0;
            for (int i = 0; i < range.Map().SectorCount(); ++i)
            {	
                // ... ����� ������ ������ � ������ i-�� �������
                shared_ptr<FlashOStream> out = rangeStorage.OutStreamRange( RangeIOBase::NamedRangeIndex(rangeIndex) );
                if (i != 0) out->SectorSeek(offs);                    
                out->getWriter().WriteDword(0);
                offs += range.Map().Sector(i).Size();
                
                // ������ ������� ����������:
                TUT_ASSERT(out->SizeBeyondCurrPos() == range.Map().FullSize() - offs);
            }
        }
    }

    // ------------------------------------------------------------------------------------

   
    // class vs function - ?
    // ���� ������������ ���������� ��� ������/������ � SectorSeek
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
                
                if (m_range.Map().SectorCount() > 1) // ��������� �� ������ ���������� �������
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
                if (m_range.Map().SectorCount() > 1) // ��������� �� ������ ���������� �������
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
            m_regionIndex(regionIndex), // ��������� ������
            m_range( static_cast<IDebugRangeIOBase&>(rangeStorage).getRange(regionIndex) )            
        {
            TestSeek(); // ������������ ���������� ��� SectorSeek
            TestWrite();// ������������ ���������� ��� ������ � �����
            TestRead(); // ������������ ���������� ��� ������ �� ������
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
        


        // ������������ ����������������� ������/������ ����� ������� ��������.
        shared_ptr<FlashOStream> outStream = rangeDev.OutStreamAll();
        shared_ptr<FlashIStream> inStream = rangeDev.InStreamAll();
        RwThroughSectors(sectorDev.getMap(), outStream->getWriter(), inStream->getReader());

        // ���� ������/������ � ����������� �������������.
        // (�������� ������ SectorSeek)        
        TestFileIO testFileIO(rangeDev, sectorDev.getMap());

        // �������� ������������� �������� � ������ ������ FlashOStream::SizeBeyondCurrPos()
        TestSizeBeyondCurrPos(rangeDev);

        // �������� ������������ ���������� ��������.
        TestThrow testThrow(rangeDev, RangeIOBase::CfgDefault);       
    }
} // namespace TestBfBoot
