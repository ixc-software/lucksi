#ifndef __LOGPRINT__
#define __LOGPRINT__

#include "stdafx.h"
#include "iLog/LogWrapper.h"

namespace RtpTests
{
    // class holds some parameters to print log
    class LogPrint : boost::noncopyable
    {
        iLogW::LogStore &m_logStore;

        bool m_printToCout; 
        std::string m_filename;
        //boost::shared_ptr<iLogW::ILogSessionHandler> m_pLogHandler;
    
    public:

        LogPrint(iLogW::LogStore &logStore, const std::string &filename, 
            bool printToCout)
            : m_logStore(logStore), 
            m_filename(filename), m_printToCout(printToCout)
        {
        }
        
        iLogW::LogStore &getLogStore() const { return m_logStore; }
        const std::string &getFileName() const { return m_filename; }
        bool isPrintToCout() const { return m_printToCout; }
        
        static boost::shared_ptr<iLogW::ILogSessionHandler> 
            CreateLogHandler(const LogPrint &logPrint, bool delaySync);
    };

} // namespace RtpTests

#endif
