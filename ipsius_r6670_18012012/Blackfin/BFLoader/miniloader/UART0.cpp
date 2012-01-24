#include <cdefBF537.h>
#include <ccblkfn.h>
#include "timer0.h"
#include "macros.h"
#include "commands.h"
#include <stdio.h>


//#define DIVIDER SYS_CLK/(16*BAUD_RATE)


void Init_UART(unsigned long speed)
{
	volatile int temp;
	unsigned long divider = SYS_CLK/(16*speed);
	
	// set FERF registers
    *pPORTF_FER |= 0x0003;
    ssync();
    
	// Configure UART0 RX and UART0 TX pins  
	*pPORT_MUX = 0;
    ssync();
  
	*pUART0_GCTL = UCEN;
	
	*pUART0_LCR = DLAB;
	*pUART0_DLL = divider;
	*pUART0_DLH = divider>>8;
	*pUART0_LCR = 0x03;
 	
	temp = *pUART0_RBR;
	temp = *pUART0_LSR;
	temp = *pUART0_IIR;

	*pUART0_IER = ETBEI;
}


int PutChar(const char cVal)
{
	int nStatus = 0;
	unsigned int nTimer = SetTimeout(1000);
	if( ((unsigned int)-1) != nTimer )
	{	
		do{ 
			if( (*pUART0_LSR & THRE) )
			{
				*pUART0_THR = cVal;
				nStatus = 1;
				break; //return 1;
				//asm("nop;");  
			}
		}while( !IsTimedout(nTimer) );
	}
	
	ClearTimeout(nTimer);
	
	return nStatus;
}

void UART_SendByte(byte val)
{
	int nStatus = 0;
	unsigned int nTimer = SetTimeout(1000);
	if( ((unsigned int)-1) != nTimer )
	{	
		do{ 
			if( (*pUART0_LSR & THRE) )
			{
				*pUART0_THR = val;
				nStatus = 1;
				break; //return 1;
				//asm("nop;");  
			}
		}while( !IsTimedout(nTimer) );
	}
	
	ClearTimeout(nTimer);	
}


int GetChar(char *const cVal)
{
	int nStatus = 0;
	unsigned int nTimer = SetTimeout(1000);
	if( ((unsigned int)-1) != nTimer )
	{
		do{ 
			if( DR == (*pUART0_LSR & DR) )
			{
				*cVal = *pUART0_RBR;
				nStatus = 1;
				break; //return 1;		
				//asm("nop;");  
			}
		}while( !IsTimedout(nTimer) );
	}
	
	ClearTimeout(nTimer);

	return nStatus;
}

byte ReadCom()
{	
	byte temp;
	while(!(DR == (*pUART0_LSR & DR)));
	// read
	temp = *pUART0_RBR;	
	return temp;
	
}

byte UART_ReadByte()
{	
	byte temp;
	while(!(DR == (*pUART0_LSR & DR)));
	// read
	temp = *pUART0_RBR;	
	return temp;
	
}



void SendAnswer(byte c)
{
//  UART_SendByte(COM_SYNC);
  UART_SendByte(c);  
}




