#ifndef __ZIPARCHIVE__
#define __ZIPARCHIVE__

#include "Platform/Platform.h"

#include "Utils/IBasicInterface.h"
#include "Utils/ErrorsSubsystem.h"
#include "Utils/ManagedList.h"
#include "Utils/UtilsDateTime.h"

namespace Utils
{

    class ZipItemBase : boost::noncopyable
    {
        std::string m_name; 
        Utils::DateTime m_dt;
        int m_size; 
        int m_compressedSize;

        virtual void FileBeginRead() = 0;
        virtual bool FileRead(void *pData, int buffSize, int &bytesReaded) = 0;

    protected:

        ZipItemBase(const std::string &name, const Utils::DateTime &dt,
            int size, int compressedSize) :
            m_name(name),
            m_dt(dt),
            m_size(size),
            m_compressedSize(compressedSize)
        {
            // ... 
        }

    public:

        virtual ~ZipItemBase()
        {
        }

        int Size() const { return m_size; }
        const std::string& Name() const { return m_name; }

        std::string ToString() const 
        {
            return m_name;
        }

        void BeginRead()
        {
            FileBeginRead();
        }

        // return OK, if file read completed. Function after read completed throw exception
        bool Read(void *pData, int buffSize, int &bytesReaded)  // can throw
        {
            return FileRead(pData, buffSize, bytesReaded);
        }

        bool Read(std::vector<Platform::byte> &buff, int maxBytes) 
        {
            ESS_ASSERT(maxBytes > 0);

            buff.resize(maxBytes);
            int bytesReaded;
            bool res = Read(&buff[0], buff.size(), bytesReaded);

            buff.resize(bytesReaded);
            return res;
        }

        void ReadAll(std::vector<Platform::byte> &buff)
        {
            Read(buff, m_size);
        }

        
    };

    // ----------------------------------------------------------
    
    // .zip file presentation 
    class ZipArchive
    {
        class ZipItem;  // stored item (file or directory)

        // RAII wrapper for handle 
        class ZipHandle
        {
            void *m_handle;

        public:
            ZipHandle(const std::string &fileName);
            ~ZipHandle();
            void* get() { return m_handle; }
        };

        ZipHandle m_handle;

        int m_currPos;
        int m_totalItems;
        Utils::ManagedList<ZipItem> m_items;

        void FocusEnter(int itemIndex);
        void FocusLeave(int itemIndex);

        ZipItem& Item(int indx);

        bool PosIsOK(int pos) const;

    public:

        ESS_TYPEDEF(ZipException);
        ESS_TYPEDEF_FULL(ZipExceptionGeneral,      ZipException);  // unzip API error
        ESS_TYPEDEF_FULL(ZipExceptionBadCRC,       ZipException);
        ESS_TYPEDEF_FULL(ZipExceptionFileOpen,     ZipException);

        ZipArchive(const std::string &fileName);  // can throw
        ~ZipArchive();

        void First();
        bool Next();
        bool Eof() const;

        bool SetCurrent(const std::string &name);

        ZipItemBase& Current();                
    };
    
        
}  // namespace Utils

#endif
