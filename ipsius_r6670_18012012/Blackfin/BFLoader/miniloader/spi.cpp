
/*

  spi module

*/

#include <cdefBF537.h>
#include <ccblkfn.h>
#include "macros.h"
#include "spi.h"

#define CLKIN 25				// CLKIN frequency is 27 MHz on the BF533 EZ-Kit
#define SF_MAX_CLOCK	200000		// use 200 kHz max. clock rate (debug)


dword	giSfPage;
word	giSfCtr;
dword	giSfSpiBaud;

//--------------------------------------------------

void SpiInit(void)
{

	dword _SCLK = SYS_CLK;

	int temp;	

	temp = *pPORTF_FER;
	ssync();
	temp |= 0x3800;
	*pPORTF_FER = temp;
	ssync();
	*pPORTF_FER = temp;
	ssync();
						
	// calculate divisor constant
	giSfSpiBaud = (_SCLK / 2 / SF_MAX_CLOCK) + 1;	// max. 20 MHz
	*pPORTFIO_DIR |= SF_CS;		// set output direction for all SPI select lines
	DESELECT();

	*pSPI_CTL 	&=  (~SPE);					// disable SPI
	*pSPI_BAUD  =   giSfSpiBaud;				
	*pSPI_CTL = 0x01 | MSTR;
	asm("ssync;");
	*pSPI_CTL 	|=  SPE;					// enable SPI
	asm("ssync;");
			
}

//--------------------------------------------------

byte Spi(byte data)
{

	*pSPI_TDBR = data;					// send data
	while ((*pSPI_STAT & TXS));			// wait for transfer buffer empty
	while (!(*pSPI_STAT & RXS));		// wait for receive buffer full
	return *pSPI_RDBR;					// get input
}

//--------------------------------------------------
