#include <cdefBF537.h>
#include <ccblkfn.h>
#include "UartSimple.h"
#include "macros.h"


#define UART0DIV (SYS_CLK/(16*UART0_SPEED))
#define UART1DIV (SYS_CLK/(16*UART1_SPEED))

    //-------------------------------------------------------------------		

	void Uart0Init()
	{
        volatile int temp = 0;
	        
    	*pPORTF_FER |= 0x0003;
		*pPORT_MUX &= ~PFDE;                 	
    	ssync();			
    	*pUART0_GCTL = UCEN;	
    	*pUART0_LCR = DLAB;
    	*pUART0_DLL = UART0DIV;
    	*pUART0_DLH = UART0DIV >>8;
    	*pUART0_LCR = 0x03;
	
    	temp = *pUART0_RBR;
    	temp = *pUART0_LSR;
    	temp = *pUART0_IIR;
		
	}
	
    //-------------------------------------------------------------------			
	
	void Uart1Init()
	{
		
        volatile int temp = 0;		
		
    	*pPORTF_FER |= 0x000C;
		*pPORT_MUX &= ~PFTE;        	
    	ssync();			
    	*pUART1_GCTL = UCEN;	
    	*pUART1_LCR = DLAB;
    	*pUART1_DLL = UART1DIV;
    	*pUART1_DLH = UART1DIV >>8;
    	*pUART1_LCR = 0x03;
	
    	temp = *pUART1_RBR;
    	temp = *pUART1_LSR;
    	temp = *pUART1_IIR;
    			
	}

    //-------------------------------------------------------------------		        

    bool IsRTS(byte portNum)
    {    	
    	volatile unsigned short *reg;
    	reg = (portNum == 1) ? pUART1_LSR : pUART0_LSR;
    	
    	return (*reg & THRE) ? 1 : 0;    	
		
    }    
	    
    //-------------------------------------------------------------------		            
	
    void Send0Msg(const char *msg)
	{        					
	    while (*msg !=0) 
	    {
	    	while(!IsRTS(0));
	    	*pUART0_THR = *msg++;
	    }
	}
	
	    
    //-------------------------------------------------------------------		    
    
    void Send1Msg(const char *msg)
    {
	    while (*msg !=0) 
	    {
	    	while(!IsRTS(1));
	    	*pUART1_THR = *msg++;
	    }    	
    }
    
    //-------------------------------------------------            
    
    bool ReadyData(byte portNum)
    {    	    	
    	volatile unsigned short *reg;
    	reg = (portNum == 1) ? pUART1_LSR : pUART0_LSR;
    	return (*reg & DR) ? true : false;    		    	    
    }
    

    //-------------------------------------------------    
    
    byte ReadByte(byte portNum)
    {    	    	    	
    	return (portNum == 1) ? *pUART1_RBR : *pUART0_RBR;    	    	
    }    
    
                    
    //-------------------------------------------------------------------		

    
            
    

