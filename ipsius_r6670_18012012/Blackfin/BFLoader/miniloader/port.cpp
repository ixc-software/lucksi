#include <ccblkfn.h>
#include "port.h"
#include "display.h"
#include "timer0.h"
#include "macros.h"
#include <stdio.h>

byte sabme[]={0x00,0x01,0x7f};
//-------------------------------------------------------------------

EX_INTERRUPT_HANDLER(PORTF_IntA_ISR)
{
word tmp = (*pPORTFIO>>2) & 0x000f;
byte key_num=0;
word i;

  *pPORTFIO_CLEAR = 0x003C;
  if (tmp==0) return;
  while(tmp)
  {
    key_num++;
    tmp>>=1;
  }
  sprintf((char*)str, "Key %d is pressed",key_num);  
  Lcd_send_msg(str,1);                
      
  if (key_num==1)
  {
  }
  if (key_num==2) 
  {
  }    
  if (key_num==3) 
  {
  }        
  if (key_num==4) 
  {
  }     
}

//-------------------------------------------------------------------

void Set_Led(byte num, byte state)
{
  if (num>5) return;
  if (state) *pPORTFIO_SET	= 1<<(6+num);                
  else *pPORTFIO_CLEAR	= 1<<(6+num);          
}
//-------------------------------------------------------------------

void Led_toggle(void)
{
/*	
  if (led_on) 
  {
    Set_Led(1, 0);
  	led_on=0;
  }
  else 
  {
    Set_Led(1, 1);
    led_on++;      
  }
*/
  *pPORTGIO_TOGGLE = 0x0030;  	  
}

//-------------------------------------------------------------------

void Init_PIO(void)
{
	int temp;	
	
	temp = *pPORTF_FER;
	temp++;
	*pPORTF_FER = 0x0000;

	*pPORTFIO_INEN		= 0x003c;		// Pushbuttons 
	*pPORTFIO_DIR		= 0x0FC0;		// LEDs
	*pPORTFIO_EDGE		= 0x003C;
	*pPORTFIO_MASKA		= 0x003C;
	*pPORTFIO_SET 		= 0x0FC0;
	*pPORTFIO_CLEAR		= 0x0FC0;
	*pPORTFIO_SET 		= 0x0040;	  //RESET for DS2155
	
	
	*pPORTGIO_DIR		= 0x6000;		// LEDs
	*pPORTGIO_SET 		= 0x6000;
	*pPORTGIO_CLEAR		= 0x6000;
	
}
//-------------------------------------------------------------------
void Init_PIO_Interrupts(void)
{
  // assign core IDs to interrupts
  *pSIC_IAR3 &= 0xFFFF0FFF;
  *pSIC_IAR3 |= 0x00005000;
  register_handler(ik_ivg12,PORTF_IntA_ISR);	// PORTF_IntA_ISR -> IVG 12	
  *pSIC_IMASK |= IRQ_PFA_PORTFG;
}
//-------------------------------------------------------------------
// initialise ASYNC MEMORY controller	
section("L1_code") void InitASYNC(void)
{
  *pEBIU_AMGCTL = 0x00FA; // Bank 0,1,2 for Flash memory, bank 3 for DS2155
  ssync();
}
//-------------------------------------------------------------------
