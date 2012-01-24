#ifndef _UART_PROFILE_I_H_
#define _UART_PROFILE_I_H_


#include "UartConst.h"

namespace iUart
{
    struct UartProfile
    {
		UartProfile(int port,
			int rate = 115200, 
			DataBits dataBits = DataBits_8,
			StopBits stopBits = StopBits_1,
            ParityType parityType = ParityNone,
            FlowType flowType = FlowOff);
            
        void Validation() const;
	
		int m_port;
		int Port() const {	return m_port; }

        static bool IsValidDataBit(DataBits val);
        static bool IsValidStopBit(StopBits val);
        static bool IsValidBaudRate(int val);

        // static bool IsValidParityType(ParityType val);
        // static bool IsValidFlowType(FlowType val);
    
        DataBits m_dataBits;
        StopBits m_stopBits;
        ParityType m_parityType;
        FlowType m_flowType;
        int m_baudRate;
        int m_readWriteTimeoutMs;
    };

} // namespace BfDev

#endif

