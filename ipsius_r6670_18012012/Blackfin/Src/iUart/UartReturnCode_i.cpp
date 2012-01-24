#include "stdafx.h"
#include "BfDev/AdiDeviceResult.h"
#include "UartReturnCode.h"

namespace iUart
{
	UartReturnCode::UartReturnCode(const BfDev::AdiDeviceResult &res) 
	{
		m_code = res.Code();
		
	}
	
	bool UartReturnCode::isSuccess() const
	{
		return m_code == ADI_DEV_RESULT_SUCCESS;
	}
	
	std::string UartReturnCode::ToString(Platform::word code)	
    {
		
		switch(code)
		{
		case ADI_UART_RESULT_TIMER_ERROR:
			return "An error was detected when attempting to control and configure the timer for autobaud detection.";
		case ADI_UART_RESULT_BAD_BAUD_RATE:
			{
				std::ostringstream out;
				out << "The baud rate of the UART is invalid. This error is usually a result of the client attempting to enable" 
					<< std::endl
					<< "dataflow before the baud rate has been set or detected by the autobaud feature.";
				return out.str();
			}
		case ADI_UART_RESULT_NO_BUFFER:
			return "The driver has no buffer to process or from which to sense the processed element count.";
		}
		
		return BfDev::AdiDeviceResult::ToString(code);
    }
};

