#ifndef __LOGFILESTREAM__
#define __LOGFILESTREAM__

// DON'T INCLUDE THIS FILE DIRECTLY!
// For use in LogWrapperLibrary.h only

namespace iLogW
{

    // Class file-stream for log listing output
    class LogFileStream : public LogBasicStream
    {

    public:

        LogFileStream(const std::string &fileName, 
        	boost::shared_ptr<ILogRecordFormater> formater = boost::shared_ptr<ILogRecordFormater>()) : 
        	LogBasicStream(formater)        	
        {
        }

        void Close()
        {
        	// nothing
        }
        
        void Write(const iLogW::LogString &data)  // override
        {
        	// nothing
        }
        
    };
	
};  // namespace iLog

#endif

