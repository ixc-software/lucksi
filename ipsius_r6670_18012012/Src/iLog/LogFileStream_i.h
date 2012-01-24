#ifndef __LOGFILESTREAM__
#define __LOGFILESTREAM__

// DON'T INCLUDE THIS FILE DIRECTLY!
// For use in LogWrapperLibrary.h only

#include "Utils/StringList.h"
#include "Utils/QtHelpers.h"

namespace iLogW
{
    using boost::shared_ptr;

    // Class file-stream for log listing output
    class LogFileStream : public LogBasicStream
    {
        std::string m_fileName;
        bool m_closed;
        Utils::StringList m_data;

        void Save()
        {
            ESS_ASSERT(!m_closed);

            m_data.SaveToFile(m_fileName.c_str());
            m_closed = true;
        }

    public:

        LogFileStream(const std::string &fileName, 
            boost::shared_ptr<ILogRecordFormater> formater = boost::shared_ptr<ILogRecordFormater>()) : 
        	LogBasicStream(formater),
        	m_fileName(fileName), m_closed(false)
        {
        }

        ~LogFileStream()
        {
            if (!m_closed) Save();
        }

        void Close()
        {
            Save();
        }
        
        void Write(const iLogW::LogString &data)  // override
        {
            ESS_ASSERT(!m_closed);

            m_data.push_back( Utils::WStringToQString(data) );
        }
    };

    // Class file-stream for log listing output
    class LogDirectFileStream : public LogBasicStream
    {
        std::basic_ofstream<iLogW::LogString::value_type> m_file;
    public:
        ESS_TYPEDEF(FileError);

        LogDirectFileStream(const std::string &fileName, 
            boost::shared_ptr<ILogRecordFormater> formater = boost::shared_ptr<ILogRecordFormater>()) : 
            LogBasicStream(formater),
            m_file(fileName.c_str(), std::ios_base::in | std::ios_base::trunc)
        {
            if (!m_file) ESS_THROW(FileError);
        }
    private:
        void Write(const iLogW::LogString &data)  // override
        {
            m_file << data << std::endl;
        }
    };
};  // namespace iLog

#endif

