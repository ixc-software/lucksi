#include "stdafx.h"
#include "FlashEmul.h"
#include "Utils/ExeName.h"

namespace 
{
    
    const Platform::dword CSectorSize = 262144;
    const Platform::dword CSectorCount = 64;       

    const Platform::dword CBootImgReservedSize = 6 *  1024 * 1024;    
    const Platform::dword CBootImgSectorsCount = CBootImgReservedSize / CSectorSize; // количество секторов для загрузчика
    BOOST_STATIC_ASSERT(CBootImgReservedSize % CSectorSize == 0);

    const char* CFolder = "FlashEmulDump";
    const char CFileFiller = '*';   

   
} // namespace 


namespace BfEmul
{
    void BfSectorDevEmul::EraseEmulation( dword off, dword size )
    {
        AssertSize(off, size);

        m_file.seek(off);
        for (dword pos = off; pos < (off + size); ++pos)
        {            
            m_file.write(&CFileFiller, 1);
        }
        m_file.flush();
    }

    void BfSectorDevEmul::PhWrite( dword off, const void* data, dword dataSize, bool withVerify ) /* can throw ErrWrite */
    {
        AssertSize(off, dataSize);

        if (withVerify) // неточная проверка того, что этот участок был предварительно стерт
        {
            m_file.seek(off);
            std::vector<char> block(dataSize);
            m_file.read(&block.at(0), dataSize);

            for ( int i = off; i < block.size(); ++i)
            {
                if (block.at(i) != CFileFiller) 
                    ESS_THROW(BfBootSrv::SectorStorageDeviceBase::ErrWrite);
            }           
        }


        m_file.seek(off);
        m_file.write(reinterpret_cast<const char*>(data), dataSize);
        m_file.flush();
    }

    void BfSectorDevEmul::AssertSize( dword off, dword size ) const /* избыточно */
    {
        ESS_ASSERT( (off + size) <= m_file.size() );
    }

    void BfSectorDevEmul::PhRead( dword off, void* data, dword dataSize ) const
    {
        AssertSize(off, dataSize);
        m_file.seek(off);
        ESS_ASSERT( m_file.read(reinterpret_cast<char*>(data), dataSize) == dataSize);
    }

    void BfSectorDevEmul::PhErase(const BfBootSrv::SectorInfo &sector)
    {
        EraseEmulation(sector.BeginOffset(), sector.Size());
    }

    void BfSectorDevEmul::PhEraseFull()
    {
        EraseEmulation(0, m_file.size());
    }

    BfSectorDevEmul::BfSectorDevEmul( QString fileName, bool cleanDump ) 
        : //m_file(fileName),
        m_base(m_sectors, *this)
    {
        QDir dir(Utils::ExeName::GetExeDir().c_str());
        dir.cdUp();
        if (!dir.exists(CFolder))
        {
            dir.mkdir(CFolder);
        }
        dir.cd(CFolder);

        QDir::setCurrent(dir.absolutePath());

        m_file.setFileName(fileName);                
        if (cleanDump) m_file.remove();
        
        for (int i = 0; i < CSectorCount; ++i)
        {	
            m_sectors.AddSector(CSectorSize);                        
        }
     
        ESS_ASSERT(m_file.open(QIODevice::ReadWrite));
        
        m_file.resize(m_sectors.FullSize());
    }

    /*BfSectorDevEmul::~BfSectorDevEmul()
    {
        m_file.resize(0);
    }*/

    // ------------------------------------------------------------------------------------


    BfRangeStorageDevEmul::BfRangeStorageDevEmul( QString fileName /*= "defaultFlash.bin"*/, bool erase /*= false*/) 
        : m_flash(fileName, erase),
        m_base(m_flash.get())
    {                
        m_base.BindRegion(BfBootSrv::RangeIOBase::CfgPrim, CBootImgSectorsCount, 1);
        m_base.BindNextRegion(BfBootSrv::RangeIOBase::CfgSecond, 1);
        m_base.BindNextRegion(BfBootSrv::RangeIOBase::CfgDefault, 1);
        m_base.BindNextRegion(BfBootSrv::RangeIOBase::AppStorRange, CSectorCount - CBootImgSectorsCount - 3);   
    }
} // namespace BfEmul

