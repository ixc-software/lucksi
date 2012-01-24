#ifndef _I_BF_UART_H_
#define _I_BF_UART_H_

#include "Utils/IBasicInterface.h"
#include "BfDev/AdiDeviceResult.h"
#include "UartReturnCode.h"
#include "IUartIO.h"
#include "UartConst.h"

namespace iUart
{
    class BfUart;

    class IBfUartEvents : public Utils::IBasicInterface
    {
    public:
		virtual void ReadyToWork(const BfUart *src) = 0;

        // This method is called when inbound buffer is full or it is flushed
        virtual void DataReceived(const BfUart *src) = 0;

        // This method is called when outbound buffer is sent
        virtual void DataSent(const BfUart *src) = 0;

        // This method is called when the callback function receives an error-event
        virtual void Error(const BfUart *src, iUart::UartReturnCode error) = 0;
    };

    //------------------------------------------------------------------------------

    class IBfUartManagement : public Utils::IBasicInterface
    {
    public: 
        ESS_TYPEDEF(InitializationFailed);    
        // Management methods
        virtual iUart::UartReturnCode Open() = 0;
        virtual iUart::UartReturnCode Close() = 0;

        virtual iUart::UartReturnCode SetBoudRate(int rate) = 0;
        virtual iUart::UartReturnCode RunAutobaud(char c) = 0;
        virtual iUart::UartReturnCode RunAutobaud(int divisorBits) = 0;
        virtual iUart::UartReturnCode EnableAutocomplete(char c) = 0;
        virtual iUart::UartReturnCode DisableAutocomplete() = 0;
        virtual UartReturnCode DataflowChained() = 0;
        virtual UartReturnCode DataflowLoopback() = 0;
        virtual iUart::UartReturnCode EnableOddParityCheck() = 0;
        virtual iUart::UartReturnCode EnableEvenParityCheck() = 0;
        virtual iUart::UartReturnCode DisableParityCheck() = 0;
    };

};
#endif
