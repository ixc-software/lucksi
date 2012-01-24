#ifndef _BF_UART_MANAGEMENT_H_
#define _BF_UART_MANAGEMENT_H_

#include "stdafx.h"
#include "iLog/iLogSessionCreator.h"
#include "IBfUart.h"
#include "BfUartProfile.h"

namespace iUart
{

    class BfUart;
    
    class BfUartManagement : boost::noncopyable,
        public IBfUartManagement
    {
        BfDev::AdiDevice &m_device;    
    public:	
        BfUartManagement(BfDev::AdiDevice &device, 
        	const BfUartProfile &profile); // can throw
        ~BfUartManagement();
    // IBfUartManagement
    public:
        UartReturnCode Open();
        UartReturnCode Close();
        UartReturnCode SetBoudRate(int rate);
        UartReturnCode RunAutobaud(char c);
        UartReturnCode RunAutobaud(int divisorBits);
        UartReturnCode EnableAutocomplete(char c);
        UartReturnCode DisableAutocomplete();
        UartReturnCode Dataflow(bool par);
        UartReturnCode DataflowChained();
        UartReturnCode DataflowLoopback();
        UartReturnCode EnableOddParityCheck();
        UartReturnCode EnableEvenParityCheck();
        UartReturnCode DisableParityCheck();
    };
}; // namespace BfDev

#endif
