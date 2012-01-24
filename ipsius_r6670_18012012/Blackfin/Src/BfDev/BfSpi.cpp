#include "stdafx.h"

#include <ccblkfn.h>
#include <cdefBF537.h>

#include "BfSpi.h"

namespace
{
	BfDev::BfSpi *SpiObj = 0;

	enum
	{
		CTransferMaxTimeMs = 5
	};
}


namespace BfDev
{
    	
    //-------------------------------------------------------------------

    BfSpi::BfSpi(dword sysFreq, dword speedHz):
    	m_sysFreq(sysFreq),
		m_speedHz(speedHz)
    {
		ESS_ASSERT(!SpiObj && "SPI Port already created");    	
		SpiObj = this;
		Init();        
    }
         
    //-------------------------------------------------------------------
    
    BfSpi::~BfSpi()
    {
    	SpiObj = 0;
		ssync();
		*pSPI_CTL &= (~SPE);					// disable SPI
		ssync();
    	
    }
    
    //-------------------------------------------------------------------    

	void BfSpi::Init()
	{
		int temp;	

		temp = *pPORTF_FER;
		ssync();
		temp |= 0x3800;
		*pPORTF_FER = temp;
		ssync();
		*pPORTF_FER = temp;
		ssync();
					
		
		*pSPI_CTL 	&=  (~SPE);					// disable SPI
		*pSPI_CTL = 0x01 | MSTR;
		*pSPI_CTL 	|=  SPE;					// enable SPI
		ssync();
		SetSpeed(m_speedHz);		
	}
						
    //-------------------------------------------------------------------	

	void BfSpi::SetSpeed(dword speedHz)
	{
		m_speedHz = speedHz;
		int baud = SpiBaud(m_sysFreq, m_speedHz); 
		ESS_ASSERT(baud > 0);

		*pSPI_CTL 	&=  (~SPE);					// disable SPI
		*pSPI_BAUD  =  baud;				
		*pSPI_CTL 	|=  SPE;					// enable SPI
		ssync();					
	}

    //-------------------------------------------------------------------	

	dword BfSpi::GetSpeedHz() const
	{
		return m_speedHz;
	}	
	    
    //-------------------------------------------------------------------	    

	byte BfSpi::Transfer(byte data)
	{
		m_timer.Set(CTransferMaxTimeMs,0);
		while (*pSPI_STAT & TXS)
		{
			ESS_ASSERT(!m_timer.TimeOut() && "SPI transfer timeout");			
		}
		
		*pSPI_TDBR = data;								// send data
		
		m_timer.Set(CTransferMaxTimeMs,0);
		while (*pSPI_STAT & TXS)
		{
			ESS_ASSERT(!m_timer.TimeOut() && "SPI transfer timeout");			
		}

		while (!(*pSPI_STAT & RXS))
		{		
			ESS_ASSERT(!m_timer.TimeOut() && "SPI transfer timeout");						
		}

		return *pSPI_RDBR;					// get input		
	}
	
    //-------------------------------------------------------------------	
    
} //namespace BfDev
