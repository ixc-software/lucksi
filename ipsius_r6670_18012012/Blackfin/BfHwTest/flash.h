#ifndef __FLASH__
#define __FLASH__

#include "macros.h"

	void SpiInit();
	byte SpiTransfer(byte data);

	bool FlashInit();		
	bool FlashTest();
	bool isFlashBusy();	
	void FlashRead(dword offset, byte *pBuff, dword count);
	void SlaveLoadBlock(byte *data, dword count);

#endif

