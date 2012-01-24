#ifndef _I_UART_MANAGEMENT_I_H_
#define _I_UART_MANAGEMENT_I_H_

#include "Utils/IBasicInterface.h"
#include "qextserialportconst.h"
#include "UartConst.h"
#include "UartReturnCode.h"

namespace iUart
{
    class IUartManagement : public Utils::IBasicInterface
    {
    public: 
        virtual UartReturnCode Open() = 0;
        virtual void Close() = 0;
        virtual void SetParityCheck(ParityType) = 0;
        virtual void SetDataBits(DataBits) = 0;
        virtual void SetStopBits(StopBits) = 0;
        virtual void SetBaudRate(int) = 0;
        virtual void SetFlowControl(FlowType) = 0 ;
        virtual void SetReadWriteTimeout(int intervalMs) = 0;
        virtual QextSerial::LineStatus LineStatus() const = 0;
    };
};
#endif
