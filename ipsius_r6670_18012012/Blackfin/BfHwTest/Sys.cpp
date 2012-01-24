#include "sys.h"
#include "macros.h"
#include <stdio.h>
#include "led.h"

#include "UartSimple.h"
#include "flash.h"
#include "Timer0.h"

namespace DevIpTdm
{
	
	enum
	{
		CSysClk = 120000000,
		CTimeOut =  CSysClk/1000,
		CSdramStart = 0x00000000,	// start address of SDRAM
		CSdramSize = 0x02000000		// size of SDRAM in 32-bit words.  MT48LC32M8A2 32M x 16 bits (64 MB)				
//		CSdramSize = (0x201 * 4)
	};			
	
	enum
    {
    	CLoadBlockSize = 256,
    	CSlaveBinFlashOffset = 0xc800,  //for normal test
    	CSlaveBinSize = 30000
//    	CSlaveBinFlashOffset = 0x800000,  // for vdk echo proj
//    	CSlaveBinSize = 550000

    };	
	
	
    //-------------------------------------------------------------------                            
    
    void SecondBFLoadSpi()
    {    	
        
		*pPORTGIO_DIR |= 0x0080;		// ResetEcho
		*pPORTGIO_CLEAR	= 0x0080;                    	    			
        Delay(10);
		*pPORTGIO_DIR &= ~0x0080;		
        Delay(10);        
        
		Send0Msg("Start Loading of Echo Board\n");
		dword sended = 0;		
	
		for (dword i=0; i < CSlaveBinSize; i += CLoadBlockSize)
		{
			while(isFlashBusy());
			FlashRead(CSlaveBinFlashOffset + sended, (byte*)buff, CLoadBlockSize);
			SlaveLoadBlock((byte*)buff, CLoadBlockSize);
			sended += CLoadBlockSize;
		}										

		Send0Msg("End of Loading of Echo Board\n");    		
    }                        
	
	
	
    //-------------------------------------------------------------------		
	
    void PllInit()
    {
		int temp = cli();
        #ifdef  LOW_SYS_FREQ		
		    *pPLL_DIV = 0x000F;        
        #else		    
    		*pPLL_DIV = 0x0005;
        #endif
        
		*pPLL_CTL = 0x3000;  
		idle();
		sti(temp);
		
		*pPORTFIO_INEN |= 0x0200;
		*pPORTFIO_SET  = 0x0200;   		
		
	}	
	
    //-------------------------------------------------------------------		

	void SDRAMInit()
	{
		*pEBIU_AMBCTL1 = 0xFFFFFF02;
		*pEBIU_AMGCTL = 0x00FF;
		*pEBIU_SDSTAT = *pEBIU_SDSTAT & 0xFFFE;		
		*pEBIU_SDRRC = 0x03A0; //0x03A3;
		//	*pEBIU_SDBCTL = 0x0025;
		
#ifdef  SAMSUNG_MEM		
		*pEBIU_SDBCTL = 0x0013;
#else		
		*pEBIU_SDBCTL = 0x0023;
#endif		
		*pEBIU_SDGCTL = 0x0091998d;//0x998D0491;
		ssync();
	}
	
    //-------------------------------------------------------------------				
	
	void InitASYNC()
	{
	  
		*pEBIU_AMGCTL = 0x00FA; // Bank 0,1,2 for Flash memory, bank 3 for DS2155
		*pEBIU_AMBCTL1 = (*pEBIU_AMBCTL1 & 0x0000FFFF) | 0xffc20000;
		ssync();
	  
	}
		
    //-------------------------------------------------------------------			
/*
	int SDRAMTest()
	{
		volatile unsigned int *pDst;
		volatile int nIndex;
		volatile int u;		
		int bError = 1; 	// returning 1 indicates a pass, anything else is a fail
		int n;

		while (1)
		{

		pDst = (unsigned int *)0x1fc;						    		    		    
           *pPORTGIO_SET= 0x0030;      
		   *pDst = 0x5555;
           *pDst++;
		   *pDst = 0xaaaa;           
			ssync();		   
            u = *pDst;
            *pDst--;            
            nIndex = *pDst;                        
		   *pPORTGIO_CLEAR	= 0x0030;
        	sprintf((char*)buff, "RD addr = %x  val %x\n", pDst, nIndex);  
      	    Send0Msg(buff);                  					   		   
           *pDst++;      	    
        	sprintf((char*)buff, "RD addr = %x  val %x\n", pDst, u);  
      	    Send0Msg(buff);                  					   		   
      	    
      	    Delay(500);
		}
		
		for(int i=0; i<16; i++)
		{
		    *pDst = 1<<i;
        	sprintf((char*)buff, "WR addr = %x by val %x\n", pDst, 1<<i);  
      	    Send0Msg(buff);                  			
        	sprintf((char*)buff, "RD addr = %x  data %x\n", pDst, *pDst);  
      	    Send0Msg(buff);                  			        		    		    
		}
		
		return 0;
		
		pDst = 0;
		*pDst = 1;
		pDst++;		
		*pDst = 2;		
		ssync();
		
		pDst = 0;
    	sprintf((char*)buff, "addr = %x  data %x\n", pDst, *pDst);  
  	    Send0Msg(buff);                  			
        pDst++;
    	sprintf((char*)buff, "addr = %x  data %x\n", pDst, *pDst);  
  	    Send0Msg(buff);                  			        
  	    return 0;
		    
	}
*/

	int SDRAMTest()
	{
	
		volatile unsigned int *pDst;
		int nIndex = 0;
		int bError = 1; 	// returning 1 indicates a pass, anything else is a fail
		int n;

		for(nIndex = 0, pDst = (unsigned int *)CSdramStart; pDst < (unsigned int *)(CSdramStart + CSdramSize); pDst++, nIndex++)
		{
//	    	sprintf((char*)buff, "addr = %x  val %x\n", pDst, nIndex);  
//	  	    Send0Msg(buff);                  			
    
			*pDst = nIndex;
			ssync();
		}
	
		for(nIndex = 0, pDst = (unsigned int *)CSdramStart; pDst < (unsigned int *)(CSdramStart + CSdramSize); pDst++, nIndex++)
		{
			if( nIndex != *pDst )
			{
                #ifdef MEM_TEST_DEBUG			    
			    	sprintf((char*)buff, "Err addr = %x  must be %x, read %x\n", pDst, nIndex, *pDst);  
			  	    Send0Msg(buff);                  			
                #endif			  	
				bError = 0;
				break;
			}
		}
		
		for(nIndex = 0xFFFFFFFF, pDst = (unsigned int *)CSdramStart; pDst < (unsigned int *)(CSdramStart + CSdramSize); pDst++ )
		{
			*pDst = nIndex;
		}
	
		for(nIndex = 0xFFFFFFFF, pDst = (unsigned int *)CSdramStart; pDst < (unsigned int *)(CSdramStart + CSdramSize); pDst++ )
		{
			if( nIndex != *pDst )
			{
				bError = 0;
				break;
			}
		}
	
		for(nIndex = 0xAAAAAAAA, pDst = (unsigned int *)CSdramStart; pDst < (unsigned int *)(CSdramStart + CSdramSize); pDst++ )
		{
			*pDst = nIndex;
		}
	
		for(nIndex = 0xAAAAAAAA, pDst = (unsigned int *)CSdramStart; pDst < (unsigned int *)(CSdramStart + CSdramSize); pDst++ )
		{
			if( nIndex != *pDst )
			{
				bError = 0;
				break;
			}
		}
		
		for(nIndex = 0x55555555, pDst = (unsigned int *)CSdramStart; pDst < (unsigned int *)(CSdramStart + CSdramSize); pDst++ )
		{
			*pDst = nIndex;
		}
	
		for(nIndex = 0x55555555, pDst = (unsigned int *)CSdramStart; pDst < (unsigned int *)(CSdramStart + CSdramSize); pDst++ )
		{
			if( nIndex != *pDst )
			{
				bError = 0;
				break;
			}
		}   
		return bError;
	}

    //-------------------------------------------------------------------			
    
    bool GetKey()
    {
        return !(*pPORTFIO & 0x0200);
    }
    	
	
	
};
