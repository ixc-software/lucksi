#include "macros.h"
#include "timer0.h"
#include "uart0.h"
#include "display.h"
#include "port.h"
#include "sdram.h"
#include "commands.h"
#include "extFlash.h"


#include <sysreg.h>
#include <ccblkfn.h>
#include <stdio.h>


#define PAGE_SIZE 256

byte str[500];      //буфер для вывода отладочной информации
byte buff[256];      //буфер для вывода отладочной информации
byte sbuff[256];      //буфер для вывода отладочной информации
byte led_timer;
byte RXcounter;

extern bool flahsBusy;

byte boot_ver[4] = {0, 2, 0, 0};  // maj, min[3] 
byte boot_id[4] = {1, 2, 3, 4};  // maj, min[3] 

void WaitSYNC()
{
	byte c;  
  
	c = ReadCom();  
    if (c == COM_SYNC)
    {         
		UART_SendByte(c);
		return;
    }      
}

word ReadAddr()
{
  word addr;
  addr = UART_ReadByte();
  return (addr | ((word)UART_ReadByte() << 8));
}



void main(void)
{
byte i,c;
word j;    
int temp;
byte rxdata;
byte flash_state = 0;  	
word addr,sum;
unsigned long my_id;
  

	temp = cli();
	*pPLL_DIV = 0x0005;
	*pPLL_CTL = 0x3000;  
	idle();
	sti(temp);

	*pPORTGIO_DIR		|= 0x0030;		// LEDs
	*pPORTGIO_SET 		= 0x0030;
	*pPORTGIO_CLEAR		= 0x0010;
	
//	Init_SDRAM();

  	Init_Timers();	
  	Init_Timer_Interrupts();
  	Init_UART(19200);  	  	  	

  	sprintf((char*)str, "BlackFin Boot V1.0\n");  
  	Lcd_send_msg(str,1);                  
  	sprintf((char*)str, "EBIU_SDBCTL = %X", *pEBIU_SDBCTL);  
  	Lcd_send_msg(str,1);                  
  	
  	  	  	
  	led_timer = SetTimeout(100);  	

/*
	while(1)
	{
	    if (IsTimedout(led_timer)) 
	    {       
			Led_toggle();				
		    ClearTimeout(led_timer);
		    led_timer = SetTimeout(100);  			
	    }					   		
		
	}
*/  	  	
  	
    flash_state = EF_Init();  
  	  	
  	
//  	if(TEST_SDRAM()) sprintf((char*)str, "MemTest OK \n");  
//	else sprintf((char*)str, "MemTest FAIL \n");    	
//  	Lcd_send_msg(str,0);                  			  			

	WaitSYNC();  	  		
	  
	while(1)
	{
/*		
	    if (IsTimedout(led_timer)) 
	    {       
			Led_toggle();				
		    ClearTimeout(led_timer);
		    led_timer = SetTimeout(100);  			
	    }					   
*/	    		
		c = ReadCom();
      
		// answer on 'ping'
		if (c == COM_SYNC)
		{
			UART_SendByte(c);
		}
          
		// send self id
		if (c == COM_GET_ID)
		{
//			Led_toggle();						        						
			SendAnswer(c);
			for (i = 0; i < 4; i++) UART_SendByte(boot_id[i]);				

		}

		// boot version
		if (c == COM_BOOT_VER)
		{         
			SendAnswer(c);
			for (i = 0; i < 4; i++) UART_SendByte(boot_ver[i]);
		}
		
		// send buff to PC
		if (c == COM_READ_BUFF)
		{
			// echo command
			SendAnswer(c);

			// send buff
			sum = 0;
			for (j = 0; j < PAGE_SIZE; j++)
			{
				UART_SendByte(buff[j]);
				sum+= buff[j];
			}
			UART_SendByte(sum & 0xFF);
		}		
		
		// UART -> buff
		if (c == COM_WRITE_BUFF)
		{
			// read 256 byte to buff
			sum = 0;
			for (j = 0; j < PAGE_SIZE; j++)
			{
				buff[j] = UART_ReadByte();
				sum+= buff[j];
			}
			
			if (UART_ReadByte() == (sum & 0xFF))
			{      
//				UART_SendByte(COM_SYNC);
				UART_SendByte(c);
			}
			else
			{
//				UART_SendByte(COM_SYNC);
				UART_SendByte(COM_ERROR);        
			}  
		}
				    
		// do reset
		if (c == COM_RESET)
		{
			SendAnswer(c);
		}

		// set speed
		if (c == COM_SET_SPEED)
		{
			
			my_id = ReadAddr();
			my_id += ((dword)ReadAddr() << 16);
         
			SendAnswer(c);

			Delay(10);
			
			Init_UART(my_id);   // re-init
		
			
//			SendAnswer(COM_ERROR);         			
		}
   
		if (c == COM_EF_CHECK)
		{
			SendAnswer(c);
			
			if(EF_FlashWaitReady() & 0x01) UART_SendByte(0);
			else UART_SendByte(1);
//			UART_SendByte(flash_state);			
//			UART_SendByte(0);
//			SendAnswer(COM_ERROR);			
		}
	
        if (c == COM_EF_READ_TO_BUFF)
        {
           addr = ReadAddr();
           EF_ReadToBuff(addr, &buff[0]);
           
           SendAnswer(c);
        }
		
		if (c == COM_EF_WRITE_FROM_BUFF)
		{
			addr = ReadAddr();
			j = EF_WriteFromBuff(addr, &buff[0]);
          
			if (j)  SendAnswer(c);
			else SendAnswer(COM_ERROR);
		}
                          
		if ((c & 0xF0) == COM_TRANSFER_ON)
		{
			EF_FlashChipErase();
//			EF_FlashSectorErase(0);
			SendAnswer(COM_ERROR);        
		}           

	}
}
