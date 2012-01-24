#ifndef BFSTORAGE_H
#define BFSTORAGE_H

#include "stdafx.h"
#include "Utils/ErrorsSubsystem.h"
#include "BfBootCore/ImgDescrList.h"


namespace BfBootSrv
{
    class Config;
    class RangeIOBase;
    class FlashOStream;
    class FlashIStream;

    using Platform::dword;
    using Platform::byte;
    
    class BfStorage : boost::noncopyable
    {
    public:       
       
        // Open new write session.        
        void OpenForWrite(dword size);

        // write data in opened file 
        void Write(const std::vector<byte>& dataCunk); // can throw NoOpened, OutOfRange        
        void Write(const void* data,  dword size); // can throw NoOpened, OutOfRange;
        
        
        void AbortWrite();

        // close current write session
        const BfBootCore::ImgDescriptor& CloseWrite(const std::string& name, dword crc); // can throw NoOpened;
        
        // �������� ���������� ����� �� ������.
        void OpenForRead(const BfBootCore::ImgDescriptor& img);

        // ������ size ���� � buffer, ������� ������ ����������.
        // ������ ����������� ����� ������ ��������� ������ ������.
        void Read(std::vector<byte>&data, dword size); // can throw InvalidDescr;

        dword getOffset(const BfBootCore::ImgDescriptor& img);
        
        void CloseRead();        

        // ������� ���������� � ��������� ���������� (��������) ������ � �����������
        // ���������������� ��������� Config. ���������� false ���� ������ �������.
        // ��������� ��� �������� ������/������.
        void DeleteLastImg();

        // ������� ���������� � ���� ���������� ������ � �����������
        // ���������������� ��������� Config. ������� ������ �� ����� (�� ������ ������������)
        void DeleteAll();

        BfStorage(Config& imgTable, RangeIOBase& flash);

        bool Find(BfBootCore::ImgId id, BfBootCore::ImgDescriptor& out) const;

    private:
        boost::shared_ptr<FlashIStream> CreateReadStream(const BfBootCore::ImgDescriptor& img);

        // Write session
        boost::shared_ptr<FlashOStream> m_ostream;
        dword m_requestedSize;
        dword m_writedSize;

        boost::shared_ptr<FlashIStream> m_istream;        
        BfBootCore::ImgDescrList& m_imgTable;

        RangeIOBase& m_flash;
    };

    

} // namespace BfBootSrv

#endif
