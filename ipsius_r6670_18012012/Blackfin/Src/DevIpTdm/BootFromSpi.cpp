#include "stdafx.h"
#include "BootFromSpi.h"
#include "BootFromSpiAsm.h"
#include "Utils/ErrorsSubsystem.h"

#include <cplbtab.h> 


namespace
{
	using namespace Platform;
	
	int CMaxSleep = 10;
	
	void DisableCache()
	{			
		dcache_invalidate_both(); 				
		disable_data_cache();		
		
		icache_invalidate(); 				
		// disable instruction cache
		volatile dword* pReg = (volatile dword*)IMEM_CONTROL;				
		dword mask = 0xfffffffd;
		*pReg = *pReg & mask;			
	}
	
	
	void PrepeareInterrupts()
	{					
		*pSIC_IAR0 = 0x22211000;
        *pSIC_IAR1 = 0x43333332;
        *pSIC_IAR2 = 0x55555444;
        *pSIC_IAR3 = 0x66655555;
        
        *pSIC_ISR = 0;
        *pSIC_IWR = 0xffffffff;
        
        *pSIC_IMASK = 0; 		
	}
	
}


namespace DevIpTdm
{
	void BootFromSpi(Platform::dword spiOffs, Platform::word devider)
	{						
		DisableCache();						
		PrepeareInterrupts();
		cli();
		
		//std::cout << "Booting from main Cpu throth spi..." << std::endl;
		BootFromSpiAsm(spiOffs, devider);			
		
		int CMaxSleep = 20;         
        for(int i = 0; i < CMaxSleep; ++i)  
        {
            iVDK::Sleep(50);
        }                     		
		ESS_HALT("BootFromSpiFailed!");
	}
}
