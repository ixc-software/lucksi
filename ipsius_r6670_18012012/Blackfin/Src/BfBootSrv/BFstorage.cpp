#include "stdafx.h"
#include "BfStorage.h"
#include "Config.h"
#include "BfBootCore/UserResponseCode.h"

namespace BfBootSrv
{
    using namespace BfBootCore;

    void BfStorage::OpenForWrite( dword size )
    {
        if (size == 0) ESS_THROW_T(BoardException, errSizeNull);
        if (m_ostream) ESS_THROW_T(BoardException, errAlreadyOpened);
        shared_ptr<FlashOStream> out = m_flash.OutStreamRange(RangeIOBase::AppStorRange);

        try
        {
            for (int i = 0; i < m_imgTable.Size(); ++i)
            {	
                out->SectorSeek(m_imgTable[i].Size);
            }
        }
        catch(NoFreeSectors&)
        {
            ESS_THROW_T(BoardException, errNoEnoughtMemory);
        }
        

        if (out->SizeBeyondCurrPos() < size) ESS_THROW_T(BoardException, errNoEnoughtMemory);

        m_ostream = out;
        m_requestedSize = size;   // ?   
        m_writedSize = 0;
    }

    void BfStorage::AbortWrite()
    {
        m_ostream.reset();
    }

    void BfStorage::Write(const std::vector<byte>& dataCunk) // can throw NoOpened, OutOfRange;
    {
        if (!m_ostream) ESS_THROW_T(BoardException, errNotOpened);
        if (m_requestedSize - m_writedSize < dataCunk.size()) ESS_THROW_T(BoardException, errWriteApwardsOfDeclare);
        m_writedSize += dataCunk.size();
        m_ostream->getWriter().WriteVectorByte(dataCunk);
    }

    void BfStorage::Write(const void* data,  dword size) // can throw NoOpened, OutOfRange;
    {
        if (!m_ostream) ESS_THROW_T(BoardException, errNotOpened);
        if (m_requestedSize - m_writedSize < size) ESS_THROW_T(BoardException, errWriteApwardsOfDeclare);
        m_writedSize += size;
        m_ostream->getWriter().WriteData(data, size);
    }

    const ImgDescriptor& BfStorage::CloseWrite(const std::string& name, dword crc) // can throw NoOpened;
    {
        if (!m_ostream) ESS_THROW_T(BoardException, errNotOpened);

        ImgDescriptor dscr;
        dscr.Id = m_imgTable.Size();
        dscr.Name = name;
        dscr.Size = m_requestedSize;
        m_imgTable.Add(dscr);            
        m_ostream.reset();

        m_requestedSize = 0;
        m_writedSize = 0;

        // crc verification
        {
            OpenForRead(dscr);
            std::vector<byte> buff;
            const dword CBuffSize = 10 * 1024;
            dword offs = 0;
            BfBootCore::CRC CRCobj;
            while(offs < dscr.Size)
            {
                dword size = ((dscr.Size - offs) >= CBuffSize) ? CBuffSize : dscr.Size - offs;
                m_istream->getReader().ReadVectorByte(buff, size);
                CRCobj.ProcessBlock(buff);            
                offs += size;
            }    
            CloseRead();
            if (CRCobj.Release() != crc) 
            {
                m_imgTable.DeleteLast();
                ESS_THROW_T(BoardException, errCRC);                
            }
        }            
 
        return m_imgTable.Back();
    }

    void BfStorage::OpenForRead( const ImgDescriptor& img )
    {
        if (img.Id >= m_imgTable.Size() || m_imgTable[img.Id] != img)
        {
            ESS_THROW_T(BoardException, errInvalidDescr);
        }            

        if (m_istream) ESS_THROW_T(BoardException, errAlreadyOpened);
        m_istream = CreateReadStream(img);
    }

    boost::shared_ptr<FlashIStream> BfStorage::CreateReadStream(const ImgDescriptor& img)
    {
        boost::shared_ptr<FlashIStream> istream = m_flash.InStreamRange(RangeIOBase::AppStorRange);        

        for (int i = 0; i < img.Id; ++i)
        {	
            istream->SectorSeek(m_imgTable[i].Size);
        }

        return istream;
    }



    void BfStorage::Read(std::vector<byte>&data, dword size) // can throw InvalidDescr;
    {
        if (!m_istream) ESS_THROW_T(BoardException, errNotOpened);
        m_istream->getReader().ReadVectorByte(data, size);
    }

    void BfStorage::CloseRead()
    {
        m_istream.reset();
    }

    void BfStorage::DeleteLastImg()
    {
        if (m_istream || m_ostream) ESS_THROW_T(BoardException, errAlreadyOpened);

        if (m_imgTable.Size() == 0) ESS_THROW_T(BoardException, errDelNothing); // throw vs nop -- ?

        m_imgTable.DeleteLast(); // запись о файле уничтожена

        shared_ptr<FlashOStream> out = m_flash.OutStreamRange(RangeIOBase::AppStorRange);
        for (int i = 0; i < m_imgTable.Size(); ++i)
        {	
            out->SectorSeek(m_imgTable[i].Size);            
        }

        out->getWriter().WriteByte(0); // затираем первый сектор файла.
    }

    void BfStorage::DeleteAll()
    {
        while(m_imgTable.Size() != 0)
        {
            DeleteLastImg();
        }
    }

    BfStorage::BfStorage( Config& imgTable, RangeIOBase& flash ) : 
        m_imgTable(imgTable.AppImgTable.Value()), m_flash(flash)
    {

    }

    bool BfStorage::Find( ImgId id, ImgDescriptor& out ) const
    {
        if (m_imgTable.Size() <= id) return false;
        out = m_imgTable[id];                        
        return true;
    }

    BfBootSrv::dword BfStorage::getOffset( const BfBootCore::ImgDescriptor& img )
    {
        return CreateReadStream(img)->getOffset();        
    }
} // namespace BfBooSrv


