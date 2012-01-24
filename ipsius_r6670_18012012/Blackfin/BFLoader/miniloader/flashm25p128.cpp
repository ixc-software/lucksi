
/*

  flash module

*/

#include <ccblkfn.h>
#include "macros.h"
#include "spi.h"
#include "flashm25p128.h"
#include "timer0.h"
#include "uart0.h"



//-------------------------------------------------------------------

byte FlashWaitReady()
{
	byte res=1;
	byte err=0;
	
	while(res & 0x01)
	{
		res = FlashStatRegRead();
		err++;
		if (err > 50) break;
		Delay(1);
	}		
	return res;
}

//-------------------------------------------------------------------

byte FlashStatRegRead()
//Status Register Read  D7h
{
	byte result;
	
	SELECT();
	Spi(0x05);
	result = Spi(0xFF);
	DESELECT();
	return result;
}

//-------------------------------------------------------------------

byte FlashIDRead()
//Status Register Read  D7h
{
	byte manufacturer;
	byte memType;	
	byte memCap;	
	
	SELECT();
	Spi(0x9f);
	manufacturer = Spi(0xFF);
	memType = Spi(0xFF);
	memCap = Spi(0xFF);
	DESELECT();
	if (manufacturer != 0x20) return 0;
	if (memType != 0x20) return 0;	
	if (memCap != 0x18) return 0;			
	return memCap;
}

//-------------------------------------------------------------------

byte FlashInit()
{
	byte k;  
	
	FlashWriteEnable();	
	
	DESELECT();
	k = FlashIDRead();
	if(k) return k; 
	return 0;
}

//-------------------------------------------------------------------

void FlashPageRead(word page, word adr, word len, byte *buff)
//Main Memory Page Read D2h
{
	word i;
	SELECT();
	Spi(0x03);					//cmd
	Spi(page>>8);
	Spi(page);
	Spi(adr & 0x00FF);          //A7-A0
	for (i = 0; i < len; i++)
	{
	  *buff = Spi(0xFF);      //read
	  buff++;
	}
	DESELECT();
}

//-------------------------------------------------------------------

bool FlashPageToBuffCompare(word page, byte *buff)
{
	word i;
	byte rdBuff[256];
	FlashPageRead(page, 0, 256, &rdBuff[0]);
	for(i=0; i<256; i++)
	{
		if(rdBuff[i] != *buff)return false;
		buff++;
	}	
	return true;  
}

//-------------------------------------------------------------------

void FlashPageErase(word page)
//Page Erase  81h
{
	SELECT();
	Spi(0x81);                  //cmd
	Spi(page>>7);               //x, P11-P7
	Spi((page<<1) & 0x00FF);    //P6-P0, x
	Spi(0xFF);                  //xx
	DESELECT();
	FlashWaitReady();
}

//-------------------------------------------------------------------
// 0 - 63
void FlashSectorErase(byte sector)
{
	if (sector > 63) return;
	word address = sector * 1024;
	SELECT();
	Spi(0xd8);
	Spi(address>>8);
	Spi(address);
	Spi(0);      
	DESELECT();
	FlashWaitReady();
}

//-------------------------------------------------------------------

void FlashWriteEnable()
{
	SELECT();
	Spi(0x06);
	DESELECT();
}

//-------------------------------------------------------------------

void FlashWriteDisable()
{
	SELECT();
	Spi(0x04);
	DESELECT();
}

//-------------------------------------------------------------------

void FlashChipErase()
{
	SELECT();
	Spi(0xc7);
	DESELECT();
	FlashWaitReady();
	FlashWriteEnable();
}


//-------------------------------------------------------------------

bool FlashPageWrite(word page, word adr, word len, byte *buff, byte n_buff)
{
	word i;
	byte *tempBuff = buff;
	
	FlashWriteEnable();
	SELECT();
	Spi(0x02);
	Spi(page>>8);
	Spi(page);
	Spi(adr & 0x00FF);
	for (i = 0; i < len; i++)
	{
		Spi(*tempBuff);
		tempBuff++;
	}
	DESELECT();
	FlashWaitReady();
		
	return FlashPageToBuffCompare(page, buff);
}

