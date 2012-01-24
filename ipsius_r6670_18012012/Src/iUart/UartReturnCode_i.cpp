#include "stdafx.h"
#include "UartReturnCode.h"

namespace {
	using namespace QextSerial;

	/*const char *ToStringImpl(QextSerial::ErrorCode code)
	{
		switch(code)
		{
		case E_NO_ERROR:
			return "No Error has occured";
		case E_INVALID_FD:                    
			return "Invalid file descriptor (port was not opened correctly)";
		case E_NO_MEMORY:                    
			return "Unable to allocate memory tables (POSIX)";
		case E_CAUGHT_NON_BLOCKED_SIGNAL:                    
			return "Caught a non-blocked signal (POSIX)";
		case E_PORT_TIMEOUT:                    
			return "Operation timed out (POSIX)";
		case E_INVALID_DEVICE:                    
			return "The file opened by the port is not a character device (POSIX)";
		case E_BREAK_CONDITION:                    
			return "The port detected a break condition";
		case E_FRAMING_ERROR:                    
			return "The port detected a framing error (usually caused by incorrect baud rate settings)";
		case E_IO_ERROR:                    
			return "There was an I/O error while communicating with the port";
		case E_BUFFER_OVERRUN:                    
			return "Character buffer overrun";
		case E_RECEIVE_OVERFLOW:                    
			return "Receive buffer overflow";
		case E_RECEIVE_PARITY_ERROR:                    
			return "The port detected a parity error in the received data";
		case E_TRANSMIT_OVERFLOW:                    
			return "Transmit buffer overflow";
		case E_READ_FAILED:                    
			return "General read operation failure";
		case E_WRITE_FAILED:                    
			return "General write operation failure";
        case E_PORT_IS_BUSY:
            return "Port is busy";
		}
		return "Unknown error";
	}*/
}
namespace iUart
{
    UartReturnCode::UartReturnCode(class BfDev::AdiDeviceResult const &)
    {
        m_code = QextSerial::E_NO_ERROR;
    }

    bool UartReturnCode::isSuccess() const
    {
        return m_code == QextSerial::E_NO_ERROR;
    }

    std::string UartReturnCode::ToString(Platform::word code)
    {
        return QextSerial::ToString(static_cast<QextSerial::ErrorCode>(code));
    }
};

