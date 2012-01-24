#ifndef _I_UART_EVENTS_I_H_
#define _I_UART_EVENTS_I_H_

#include "Utils/IBasicInterface.h"
#include "iUart/UartReturnCode.h"

namespace iUart
{
    class Uart;

    class IUartEvents : public Utils::IBasicInterface
    {
    public:
        virtual void Error(const Uart *src, UartReturnCode error) = 0;
    };
};    
#endif
