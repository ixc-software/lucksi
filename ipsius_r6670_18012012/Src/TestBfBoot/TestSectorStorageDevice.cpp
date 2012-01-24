#include "stdafx.h"

#include "TestSectorStorageDevice.h"
#include "FlashEmul.h"

namespace 
{
    using namespace TestBfBoot;
    using namespace BfBootSrv;
    using namespace Platform;
    using boost::shared_ptr;    

    class TestSectorDev
    {
        BfSectorDevEmul m_emul;
        SectorStorageDeviceBase& m_flash;

        std::vector<byte> m_dataForWrite; // data using for Write to flash, have size equal to flash.

        // -------------------------------------------------------------------------------------

        void RwThroughSectors()
        {
            m_flash.EraseAll();
            m_flash.Write(0, m_dataForWrite);

            std::vector<byte> rxData;      
            m_flash.Read(0, rxData, m_dataForWrite.size());

            TUT_ASSERT(m_dataForWrite == rxData);    
        }

        // -------------------------------------------------------------------------------------

        void TestNoEraseThrow()
        {
            m_flash.EraseAll();
            m_flash.Write(0, m_dataForWrite); // ���� ���������

            bool exceptionOk = false;
            try
            {
                byte b = 0;
                m_flash.Write(0, &b, sizeof(b)); // ������� ������ � ��� ���������� ������
            }
            catch(BfBootSrv::SectorStorageDeviceBase::ErrWrite& err)
            {
                exceptionOk = true;
            }
            TUT_ASSERT(exceptionOk);
        }

        // -------------------------------------------------------------------------------------

        void TestOutOfRangeThrow()
        {
            m_flash.EraseAll();            

            bool exceptionOk = false;
            try
            {
                byte b = 0;            
                m_flash.Write(m_flash.getMap().FullSize(), &b, 1);
            }
            catch(BfBootSrv::SectorStorageDeviceBase::ErrOutOfRange& err)
            {
                exceptionOk = true;
            }
            TUT_ASSERT(exceptionOk);
        }

        // -------------------------------------------------------------------------------------

        void TestRWBySector()
        {
            m_flash.EraseAll();
            //std::vector<byte> nullData(dataSize, 0);
            m_flash.Write(0, m_dataForWrite); // ���� ���������  

            for (int i = 0; i < m_flash.getMap().SectorCount(); ++i)
            {
                const BfBootSrv::SectorInfo& sector = m_flash.getMap().Sector(i);
                m_flash.EraseSector(sector);
                int off = sector.BeginOffset();
                int size = sector.Size();
                ESS_ASSERT(m_dataForWrite.size() >= (off + size) );
                m_flash.Write(off, &m_dataForWrite.at(off), size);
            }

            std::vector<byte> rxData;      
            m_flash.Read(0, rxData, m_dataForWrite.size());

            TUT_ASSERT(m_dataForWrite == rxData);                  
        }

        // -------------------------------------------------------------------------------------

    public:
        TestSectorDev()
            : m_flash(m_emul.get())
        {
            // Creating data for write            
            dword dataSize = m_flash.getMap().FullSize();
            for (int i = 0; i < dataSize; ++i)
            {
                m_dataForWrite.push_back(i);
            }

            // �������� ������-������ c ��������� ������ ��������.   
            RwThroughSectors();           

            // �������� ������������ ���������� ErrWrite
            TestNoEraseThrow();            

            // �������� ������������ ���������� ErrOutOfRange
            TestOutOfRangeThrow();

            // ����������� ������ � ���������� ����
            TestRWBySector();          
        }
    };
} // namespace 

namespace TestBfBoot
{
    void TestSectorStorageDevice()
    {        
        TestSectorDev test;  
    }
} // namespace TestBfBoot
