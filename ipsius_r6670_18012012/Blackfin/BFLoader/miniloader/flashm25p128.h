

#ifndef __FLASH_M25P128__
#define __FLASH_M25P128__

#include "macros.h"

byte FlashInit();
byte FlashStatRegRead();                                                  //Status Register Read  D7h

void FlashPageRead(word page, word adr, word len, byte *buff);                //Main Memory Page Read D2h
//void FlashPageErase(word page);                                               //Page Erase  81h
bool FlashPageWrite(word page, word adr, word len, byte *buff, byte n_buff);  //Main Memory Page Program through Buffer 1/2  82h/85h


bool FlashPageToBuffCompare(word page, byte *buff);

void FlashSectorErase(byte sector);
void FlashChipErase();

void FlashWriteEnable();
void FlashWriteDisable();
byte FlashWaitReady();


#endif


