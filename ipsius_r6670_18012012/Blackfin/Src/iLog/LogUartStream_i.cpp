#include "stdafx.h"

#include "iUart/BfUart.h"

#include "LogWrapperLibrary.h"
#include "LogUartStream_i.h"

namespace iLogW
{
    LogUartStream::LogUartStream(int uartPort,
    	boost::shared_ptr<ILogRecordFormater> formater) : 
        LogBasicStream(formater), 
        m_uart(0)
    {
    	if(iUart::BfUart::GetBfUart(uartPort) ==0) return;
    	
    	m_uart = &iUart::BfUart::GetBfUart(uartPort)->IO();
    }

    void LogUartStream::Write( const iLogW::LogString &data )
    {
    	if(m_uart != 0) m_uart->Write(data, false);
    }
};

