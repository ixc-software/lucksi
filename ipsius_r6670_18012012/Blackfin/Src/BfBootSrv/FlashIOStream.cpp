#include "stdafx.h"
#include "FlashIOStream.h"
#include "StoreRange.h"

namespace 
{
    bool const CWithWritingVerify = false; //вынести в конфиг проэкта 
} // namespace 

namespace BfBootSrv
{
    class RWStream
    {
        Utils::SafeRef<SectorStorageDeviceBase> m_flash;        
        Utils::SafeRef<IWriteProgress> m_prog;
        const StoreRange m_range;        
        SectorInfo m_currSector;
        dword m_offs; // absolute offset in flash
        const bool m_useForRead;
        bool m_seekAccepted;

        // return writhed size
        dword WriteInCurrSector(const byte *pSrc, size_t length)
        {
            dword last = m_offs + length - 1; // последняя позиция записываемого блока

            dword writeSize = length;
            if (last > m_currSector.LastIndex()) // выход за границы текущего сектора
            {
                writeSize = m_currSector.LastIndex() - m_offs + 1;
                ESS_ASSERT(m_range.NextSector(m_currSector)); // размер проверен предварительно
                m_flash->EraseSector(m_currSector);
            }

            if (writeSize != 0) m_flash->Write(m_offs, pSrc, writeSize, CWithWritingVerify);
            m_offs += writeSize;

            return writeSize;
        }

    public:

        dword getOffs() const
        {
            return m_offs;
        }

        void SetWriteProgressInd(Utils::SafeRef<IWriteProgress> progInd)
        {
            ESS_ASSERT(!m_useForRead);
            m_prog = progInd;
        }

        RWStream(Utils::SafeRef<SectorStorageDeviceBase> flash, const StoreRange& range, bool useForRead)
            : m_flash(flash),
            m_range(range),
            m_currSector(range.Map().FrontSector()),
            m_offs(m_currSector.BeginOffset()),
            m_useForRead(useForRead),
            m_seekAccepted(true)
        {                       
        }

        void Read(void *pDest, size_t length) // can throw OutOfRange
        {
            ESS_ASSERT(m_useForRead);
            m_seekAccepted = false;

            if (m_offs + length > m_range.Map().BackSector().LastIndex()) ESS_THROW(OutOfRange);                       
            m_flash->Read(m_offs, pDest, length);
            m_offs += length;
        }

        void Write(const void *pSrc, size_t length) // can throw OutOfRange
        {
            ESS_ASSERT(!m_useForRead);
            m_seekAccepted = false;

            if (!m_range.Map().IsValidateRange(m_offs, length)) ESS_THROW(OutOfRange);
            if (m_currSector.BeginOffset() == m_offs) 
                m_flash->EraseSector(m_currSector); // first write

            const byte* pByteSrc = reinterpret_cast<const byte*>(pSrc);

            size_t percent = length / 100;
            while (length > 0)
            {
                dword writed = WriteInCurrSector(pByteSrc, length);
                length -= writed;
                pByteSrc += writed;

                if (!m_prog.IsEmpty()) 
                {
                    int progress = (length == 0 || percent == 0) ? 100 : 100 - length / percent;                                        
                    m_prog->WritePercent( progress );                
                }
            }          

        }

        // Jump forward to minSkipBytes with alignment to sector begin
        void SectorSeek(dword minSkipBytes)
        {
            /* bool vs throw vs lockStream -- ? */
            //ESS_ASSERT(m_offs == m_currSector.BeginOffset()); // небыло RW
            ESS_ASSERT(m_seekAccepted);
            if ( !m_range.JumpSector(m_currSector, minSkipBytes) ) ESS_THROW(NoFreeSectors); // return false ?
            m_offs = m_currSector.BeginOffset();
            //return true;
        }

        // Остаточный размер с учетом выравнивания тек позиции по началу след. сектора.
        int SizeBeyondCurrPos() const
        {
            return m_range.SizeBeyondOffs(m_offs);
        }
    };


    // -------------------------------------------------------------------------------------

    
    FlashIStream::FlashIStream( Utils::SafeRef<SectorStorageDeviceBase> flash, const StoreRange& range ) : 
        m_streamBase( new RWStream(flash, range, true) ),
        m_reader(*this)
    {
    }

    // -------------------------------------------------------------------------------------

    FlashIStream::~FlashIStream()
    {
    }

    // -------------------------------------------------------------------------------------

    Utils::IBinaryReader& FlashIStream::getReader()
    {
        return m_reader;
    }

    // -------------------------------------------------------------------------------------

    void FlashIStream::SectorSeek( dword minSkipBytes )
    {
        m_streamBase->SectorSeek(minSkipBytes);
    }

    // -------------------------------------------------------------------------------------

    byte FlashIStream::ReadByte()
    {
        Platform::byte val;
        Read(&val, 1);        
        return val;
    }

    // -------------------------------------------------------------------------------------

    void FlashIStream::Read(void *pDest, size_t length) // can throw SectorStorageDeviceBase::ErrOutOfRange;
    {
        m_streamBase->Read(pDest, length);
    }

    // ------------------------------------------------------------------------------------

    BfBootSrv::dword FlashIStream::getOffset() const
    {
        return m_streamBase->getOffs();
    }

    // -------------------------------------------------------------------------------------

    FlashOStream::FlashOStream( Utils::SafeRef<SectorStorageDeviceBase> flash,const StoreRange& range ) 
        : m_streamBase( new RWStream(flash, range, false) ),
        m_writer(*this)
    {
    }

    // -------------------------------------------------------------------------------------

    FlashOStream::~FlashOStream()
    {
    }

    // -------------------------------------------------------------------------------------

    void FlashOStream::SectorSeek( dword minSkipBytes )
    {
        m_streamBase->SectorSeek( minSkipBytes );
    }

    // -------------------------------------------------------------------------------------

    int FlashOStream::SizeBeyondCurrPos() const
    {
        return m_streamBase->SizeBeyondCurrPos();
    }

    // -------------------------------------------------------------------------------------

    void FlashOStream::Write( byte b )
    {
        Write(&b, 1);
    }

    // -------------------------------------------------------------------------------------

    void FlashOStream::Write( const void *pSrc, size_t length ) /* can throw OutOfRange */
    {
        m_streamBase->Write(pSrc, length);
    }

    // -------------------------------------------------------------------------------------

    void FlashOStream::SetWriteProgressInd( Utils::SafeRef<IWriteProgress> progInd )
    {
        m_streamBase->SetWriteProgressInd(progInd);
    }

} // namespace BfBootSrv

