#include <cdefBF537.h>
#include <ccblkfn.h>

#define SDRAM_START  0x00000000	// start address of SDRAM
#define SDRAM_SIZE	 0x02000000	// size of SDRAM in 32-bit words.  MT48LC32M8A2 32M x 16 bits (64 MB)

void Init_SDRAM();
int TEST_SDRAM();


void Init_SDRAM()
{
	// Initalize EBIU control registers to enable all banks	
	*pEBIU_AMBCTL1 = 0xFFFFFF02;
	ssync();
	
	// -- not sure why there is a read here, possibly anomaly 05000157?
	//temp = *pEBIU_AMBCTL1;
	//temp++;
	
	*pEBIU_AMGCTL = 0x00FF;
	ssync();
	
	
	// Check if already enabled
	if( SDRS != ((*pEBIU_SDSTAT) & SDRS) )
	{
//		return;
		*pEBIU_SDSTAT = *pEBIU_SDSTAT & 0xFFFE;
	}
	
	
	//SDRAM Refresh Rate Control Register
	*pEBIU_SDRRC = 0x03A0; //0x03A3;
	
	//SDRAM Memory Bank Control Register
//	*pEBIU_SDBCTL = 0x0025;
	*pEBIU_SDBCTL = 0x0023;
	
	//SDRAM Memory Global Control Register	
	*pEBIU_SDGCTL = 0x0091998d;//0x998D0491;
	ssync();
}



int TEST_SDRAM()
{
	
	volatile unsigned int *pDst;
	int nIndex = 0;
	int bError = 1; 	// returning 1 indicates a pass, anything else is a fail
	int n;


	// write incrementing values to each SRAM location
	for(nIndex = 0, pDst = (unsigned int *)SDRAM_START; pDst < (unsigned int *)(SDRAM_START + SDRAM_SIZE); pDst++, nIndex++ )
	{
		*pDst = nIndex;
		ssync();
	}
	
	// verify incrementing values
	for(nIndex = 0, pDst = (unsigned int *)SDRAM_START; pDst < (unsigned int *)(SDRAM_START + SDRAM_SIZE); pDst++, nIndex++ )
	{
		if( nIndex != *pDst )
		{
			bError = 0;
			break;
		}
	}
		
	// write all FFFF's 
	for(nIndex = 0xFFFFFFFF, pDst = (unsigned int *)SDRAM_START; pDst < (unsigned int *)(SDRAM_START + SDRAM_SIZE); pDst++ )
	{
		*pDst = nIndex;
	}
	
	// verify all FFFF's 
	for(nIndex = 0xFFFFFFFF, pDst = (unsigned int *)SDRAM_START; pDst < (unsigned int *)(SDRAM_START + SDRAM_SIZE); pDst++ )
	{
		if( nIndex != *pDst )
		{
			bError = 0;
			break;
		}
	}
	
	// write all AAAAAA's 
	for(nIndex = 0xAAAAAAAA, pDst = (unsigned int *)SDRAM_START; pDst < (unsigned int *)(SDRAM_START + SDRAM_SIZE); pDst++ )
	{
		*pDst = nIndex;
	}
	
	// verify all AAAAA's 
	for(nIndex = 0xAAAAAAAA, pDst = (unsigned int *)SDRAM_START; pDst < (unsigned int *)(SDRAM_START + SDRAM_SIZE); pDst++ )
	{
		if( nIndex != *pDst )
		{
			bError = 0;
			break;
		}
	}
		
	// write all 555555's 
	for(nIndex = 0x55555555, pDst = (unsigned int *)SDRAM_START; pDst < (unsigned int *)(SDRAM_START + SDRAM_SIZE); pDst++ )
	{
		*pDst = nIndex;
	}
	
	// verify all 55555's 
	for(nIndex = 0x55555555, pDst = (unsigned int *)SDRAM_START; pDst < (unsigned int *)(SDRAM_START + SDRAM_SIZE); pDst++ )
	{
		if( nIndex != *pDst )
		{
			bError = 0;
			break;
		}
	}
    

	return bError;
}

