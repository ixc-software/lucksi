#ifndef _UART_RETURN_CODE_H_
#define _UART_RETURN_CODE_H_

#include "Platform/PlatformTypes.h"
#include "iUart/UartReturnCode_i.h"

namespace BfDev
{
	class AdiDeviceResult; 
};

namespace iUart 
{
    class UartReturnCode 
    {
    public:
        UartReturnCode(int code)
        {
            m_code = code;
        }
        UartReturnCode(const BfDev::AdiDeviceResult &);

		bool isSuccess() const;
		
		Platform::word Code() const
		{
			return m_code;
		}
		static std::string ToString(Platform::word code);
		
		std::string ToString() const
		{
			return ToString(m_code);
		}
    private:
        Platform::word m_code;
    };
};
#endif
