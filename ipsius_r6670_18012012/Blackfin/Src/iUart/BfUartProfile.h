#ifndef _BF_UART_PROFILE_H_
#define _BF_UART_PROFILE_H_


#include "stdafx.h"
#include "Platform/PlatformTypes.h"
#include "iUart/UartConst.h"

namespace BfDev {	class AdiDeviceCommandsTable; }
namespace iUart
{
	
	using Platform::byte;    
	using Platform::word;	
	using Platform::dword;
	
    
    struct BfUartProfile
    {
		BfUartProfile(int port,
			int countBuffers = 32,
			int sizeBuffer = 16,
			int rate = 115200, 
			iUart::DataBits dataBits = iUart::DataBits_8,
			iUart::StopBits stopBits = iUart::StopBits_1);
            
        void Validation() const;

        void FillCommandTable(BfDev::AdiDeviceCommandsTable &table) const;
		
    	bool m_logIsEnable;
		bool IsLogEnable() const {	return m_logIsEnable; }
		
		int m_port;
		int Port() const {	return m_port; }
		
				
                
        // Enables/disables synchronous mode for the driver 
        bool m_isSynchronousMode; 

        // Specifies the dataflow method the device is to use.
        // Supports the chained buffer or chained buffer with loopback method 
        bool m_dataflowIsLoopback;

        // Enables/Disables error reporting from the device driver.  This command is used to enable or disable 
        // the generation of the line status events; overrun, parity and framing errors and the break interrupt. 
        bool m_errorReporting;
        
        // Sets the number of data bits in the word.  The value passed is the number of bits.  Acceptable values 
        // are 5, 6, 7 or 8.  
        iUart::DataBits m_dataBits;
        
        // Sets the number of stop bits.  This command sets the number of stop bits.  A value of 1 sets one stop 
        // bit, a value of 2 sets two stop bits for non-5 bit word lengths and 1,5 stop bits for a 5 bit word length.  
        iUart::StopBits m_stopBits;
        
        // Enables parity checking.  This command turns on/off parity transmission and checking.  Line status 
        // events must also be turned on for reporting of parity events.  
		iUart::ParityType m_parityType;

        
        int m_baudRate;
        
        bool m_useDma;
                
        bool m_isAutoComplete;
        char m_autoCompleteChar;
        
        // Enables or disables the CTS/RTS hardware flow control logic for the UART. 
        bool m_isCtsRtsControlOn;

        // Sets active low or active high polarity for the CTS/RTS signals. 
        // By default these signals are active low.
        bool m_isCtsRtsLowPolarity;

        // Sets the threshold for CTS/RTS assertion either low (FIFOs half full) or high (FIFOs completely full).  
        // By default the threshold is set to low.  
        bool m_isCtsRtsLowThreshold;
        
        dword m_countBuffers;
        dword m_sizeBuffer;
        
    };

} // namespace BfDev

#endif

