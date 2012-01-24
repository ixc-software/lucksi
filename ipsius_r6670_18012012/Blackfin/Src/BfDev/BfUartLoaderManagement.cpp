#include "stdafx.h"

#include <cdefBF537.h>
#include <ccblkfn.h>
#include "SysProperties.h"
#include "BfUartLoaderManagement.h"


namespace BfDev
{
		
	BfUartLoaderManagement *pLoaderManagement=0;		
	
    //-------------------------------------------------------------------			
		
	BfUartLoaderManagement::BfUartLoaderManagement(iUart::IUartIO &uartIO):
		m_uartIO(uartIO)
	{
		ESS_ASSERT(!pLoaderManagement && "BfUartLoaderManagement already created");	    	
		pLoaderManagement = this;	    		    		
		
		*pPORTGIO_INEN		|= 0x0002;		// isSlaveNotReady Pin
	}

    //-------------------------------------------------------------------						
		
	iUart::IUartIO &BfUartLoaderManagement::Uart()
	{
		return m_uartIO;
	}
	
    //-------------------------------------------------------------------						

    const iUart::IUartIO &BfUartLoaderManagement::Uart() const 
    {
        return m_uartIO;
    }

    //-------------------------------------------------------------------					
	
	BfUartLoaderManagement::~BfUartLoaderManagement()
	{
		*pPORTGIO_INEN		&= ~(word)0x0002;		// isSlaveNotReady Pin		
		pLoaderManagement =	0;
	}
	
    //-------------------------------------------------------------------				


	bool BfUartLoaderManagement::isSlaveReady() const
	{
		return !(*pPORTGIO & 0x0002);
	}

	
} ;//namespace BfDev
