#ifndef __UART_LOADER_MANAGEMENT__
#define __UART_LOADER_MANAGEMENT__

#include "stdafx.h"
#include "Platform/platformtypes.h"
#include "BfUartLoader.h"


namespace BfDev
{
    using namespace Platform;    
    
    //------------------------------------------------------------------------------                
    
    class BfUartLoaderManagement : boost::noncopyable,
    	public IBfUartLoaderIO
    {   	
		public:    	
	    	BfUartLoaderManagement(iUart::IUartIO &uartIO);
			~BfUartLoaderManagement();	    	
	    	
		// IBfUartLoaderIO	 impl.
		private:
	        iUart::IUartIO &Uart();
	        const iUart::IUartIO &Uart() const;
	        bool isSlaveReady() const;
	        
		private:
			
			iUart::IUartIO &m_uartIO;
    	
    };
    
    //------------------------------------------------------------------------------                
    	
}; //namespace BfDev

#endif







