#ifndef __LOG_UART_STREAM__
#define __LOG_UART_STREAM__

// DON'T INCLUDE THIS FILE DIRECTLY!
// For use in LogWrapperLibrary.h only

#include "iUart/IUartIO.h"

namespace iLogW
{
    // Class uart-stream for log listing output
    class LogUartStream : public LogBasicStream
    {
    public:
        LogUartStream(int uartPort,
        	boost::shared_ptr<ILogRecordFormater> formater = boost::shared_ptr<ILogRecordFormater>());
    private:
        void Write(const iLogW::LogString &data);  // override;
    private:
		iUart::IUartIO *m_uart;
    };
};  // namespace iLog

#endif

