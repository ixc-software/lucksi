
#include <ccblkfn.h>
#include "macros.h"
#include "spi.h"
#include "flashm25p128.h"

// -----------------------------------------

byte EF_FlashWaitReady()
{
	return FlashWaitReady();
}

// -----------------------------------------

void EF_ReadToBuff(word page, byte *pBuff)
{
   FlashPageRead(page, 0, 256, pBuff); 
}


// ----------------------------------------

byte EF_WriteFromBuff(word page, byte *pBuff)
{
//   FlashPageErase(page);
   return(FlashPageWrite(page, 0, 256, pBuff, 0));
}


// ----------------------------------------

byte EF_Init()
{
   SpiInit(); 
   return FlashInit();     
}

// ----------------------------------------

void EF_FlashSectorErase(byte sector)
{
	FlashSectorErase(sector);
}

// ----------------------------------------

void EF_FlashChipErase()
{
	FlashChipErase();
}

bool EF_FlashPageToBuffCompare(word page, byte *buff)
{
	return FlashPageToBuffCompare(page, buff);	
}
