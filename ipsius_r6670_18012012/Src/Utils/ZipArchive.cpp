#include "stdafx.h"

//#include "minizip/unzip.h"
#include "zlib/unzip.h"
#include "Utils/StringUtils.h"
#include "Utils/ExeName.h"
#include "ZipArchive.h"

// ---------------------------------------------------
namespace
{
	QString FileName(const QString &arg)
	{
		QFileInfo fileInfo(arg);
		if(!fileInfo.isAbsolute())
		{ 
			fileInfo = QString(Utils::ExeName::GetExeDir().c_str()).append(arg);
		}
		return fileInfo.absoluteFilePath();
	}

} // namespace


namespace Utils
{

    class ZipArchive::ZipItem : public ZipItemBase
    {
        enum State
        {
            stFocused,
            stFileRead,
            stFileReadDone,
            stUnfocused,
        };

        ZipHandle &m_handle;
        State m_state;

        static Utils::DateTime ExtractDT(const unz_file_info &file_info) 
        {
            using namespace Utils;

            tm_unz ti = file_info.tmu_date;
            Date date(ti.tm_year, ti.tm_mon + 1, ti.tm_mday);
            Time time(ti.tm_hour, ti.tm_min, ti.tm_sec);
            
            return DateTime(date, time);
        }

        void CloseFile()
        {
            m_state = stFileReadDone;

            int res = unzCloseCurrentFile(m_handle.get());
            if (res == UNZ_OK) return;

            if (res == UNZ_CRCERROR) ESS_THROW(ZipExceptionBadCRC);  
            ESS_THROW(ZipExceptionGeneral);  
        }

        virtual void FileBeginRead()  // override
        {
            ESS_ASSERT(m_state != stUnfocused);

            if (m_state == stFileRead) CloseFile();

            m_state = stFocused;
        }

        bool FileRead(void *pData, int buffSize, int &bytesReaded) // override
        {
            ESS_ASSERT(m_state != stUnfocused);
            ESS_ASSERT(m_state != stFileReadDone);

            if (m_state == stFocused)  // open file
            {
                bool ok = (unzOpenCurrentFile(m_handle.get()) == UNZ_OK);
                if (!ok) ESS_THROW(ZipExceptionGeneral);  // can't open file

                m_state = stFileRead;
            }

            // read
            ESS_ASSERT(m_state == stFileRead);

            int res = unzReadCurrentFile(m_handle.get(), pData, buffSize);
            if (res < 0) 
            {
                CloseFile();
                ESS_THROW(ZipExceptionGeneral);  // read error
            }

            bytesReaded = res;

            // read completed
            if (res == 0)
            {
                CloseFile();
                return true;
            }

            return false;
        }

    public:

        ZipItem(ZipHandle &handle, const std::string &fileName, const unz_file_info &file_info) :
            ZipItemBase(fileName, ExtractDT(file_info), file_info.uncompressed_size, file_info.compressed_size),
            m_handle(handle)
        {
            m_state = stFocused;
        }

        void FocusEnter()
        {
            ESS_ASSERT(m_state == stUnfocused);

            m_state = stFocused;
        }

        void FocusLeave()
        {
            ESS_ASSERT(m_state != stUnfocused);

            if (m_state == stFileRead) CloseFile();

            m_state = stUnfocused;
        }

    };

    // ----------------------------------------------------------------

    ZipArchive::ZipHandle::ZipHandle( const std::string &fileName )
    {
		m_handle = unzOpen(FileName(fileName.c_str()).toStdString().c_str());
        if (m_handle == 0) ESS_THROW(ZipExceptionFileOpen);
    }

    ZipArchive::ZipHandle::~ZipHandle()
    {
        if (unzClose(m_handle) != UNZ_OK) ESS_THROW(ZipExceptionGeneral);
    }
}


// ---------------------------------------------------

namespace Utils
{

    ZipArchive::ZipArchive( const std::string &fileName ) : 
        m_handle(fileName),
        m_items(true, 0, true)
    {
        // global info        
        unz_global_info gi;
        if (unzGetGlobalInfo(m_handle.get(), &gi) != UNZ_OK) ESS_THROW(ZipExceptionGeneral);

        m_currPos = -1;
        m_totalItems = gi.number_entry;

        if (m_totalItems <= 0) ESS_THROW(ZipExceptionGeneral);
        m_items.AddEmptyItems(m_totalItems);

        // do first
        First();
    }

    // ---------------------------------------------------

    ZipArchive::~ZipArchive()
    {
        // nothing 
    }

    // ---------------------------------------------------

    void ZipArchive::First()
    {
        FocusLeave(m_currPos);

        if (unzGoToFirstFile(m_handle.get()) != UNZ_OK) 
        {
            m_currPos = -1;
            ESS_THROW(ZipExceptionGeneral);
        }

        FocusEnter(0);
    }

    // ---------------------------------------------------

    bool ZipArchive::Next()
    {
        FocusLeave(m_currPos);

        bool eof = unzGoToNextFile(m_handle.get()) != UNZ_OK;

        FocusEnter(m_currPos + 1);

        if (Eof())
        {
            if (!eof) ESS_THROW(ZipExceptionGeneral); // logic error
            return false;
        }
        
        if (eof) ESS_THROW(ZipExceptionGeneral);  // logic error
        return true;
    }

    // ---------------------------------------------------

    bool ZipArchive::Eof() const
    {
        return !PosIsOK(m_currPos);
    }

    // ---------------------------------------------------

    ZipItemBase& ZipArchive::Current()
    {
        if (!PosIsOK(m_currPos)) ESS_THROW(ZipExceptionGeneral);

        return Item(m_currPos);        
    }

    // ---------------------------------------------------

    bool ZipArchive::PosIsOK( int pos ) const
    {
        return (pos >= 0) && (pos < m_items.Size());
    }

    // ---------------------------------------------------

    void ZipArchive::FocusEnter( int itemIndex )
    {
        if (PosIsOK(itemIndex))
        {
            if (m_items[itemIndex] == 0)
            {
                char filename_inzip[1024];
                unz_file_info file_info;

                int err = unzGetCurrentFileInfo(m_handle.get(), &file_info, 
                    filename_inzip, sizeof(filename_inzip), 0, 0, 0, 0);

                if (err != UNZ_OK) ESS_THROW(ZipExceptionGeneral);

                m_items.Set(itemIndex, new ZipItem(m_handle, filename_inzip, file_info));
            }
            else
            {
                Item(itemIndex).FocusEnter();
            }

        }

        m_currPos = itemIndex;
    }

    // ---------------------------------------------------

    void ZipArchive::FocusLeave( int itemIndex )
    {
        if (PosIsOK(itemIndex))
        {
            Item(itemIndex).FocusLeave();
        }
    }

    // ---------------------------------------------------

    ZipArchive::ZipItem& ZipArchive::Item( int indx )
    {
        ESS_ASSERT(m_items[indx] != 0);
        return *(m_items[indx]);        
    }

    bool ZipArchive::SetCurrent( const std::string &name )
    {
        First();

        std::string nameUp = Utils::StringUpCase(name);

        while(!Eof())
        {
            if ( nameUp == Utils::StringUpCase(Current().Name()) )
            {
                return true;
            }

            Next();
        }

        return false;
    }

}  // namespace Utils

