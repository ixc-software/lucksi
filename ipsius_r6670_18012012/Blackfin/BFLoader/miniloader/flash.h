

#if !defined(__FLASH__)

#define __FLASH__

#include "macros.h"

byte FlashInit();
byte FlashStatRegRead();                                                  //Status Register Read  D7h

void FlashPageRead(word page, word adr, word len, byte *buff);                //Main Memory Page Read D2h
void FlashPageErase(word page);                                               //Page Erase  81h
bool FlashPageWrite(word page, word adr, word len, byte *buff, byte n_buff);  //Main Memory Page Program through Buffer 1/2  82h/85h

/*
void FlashBuffRead(word adr, word len, byte *buff, byte n_buff);              //Buffer 1/2 Read   D4h/D6h
void FlashBuffWrite(word adr, word len, byte *buff, byte n_buff);             //Buffer 1/2 Write  84h/87h
void FlashBuffFill(word adr, word len, byte buff, byte n_buff);               //Buffer 1/2 Write  84h/87h
BOOL FlashBuffToPageWrite(word page, byte n_buff);                            //Buffer 1/2 to Main Memory Page Program with Built-in Erase  83h/86h

void FlashBlockErase(word block);                                             //Block Erase  50h
void FlashPageToBuffRead(word page, byte n_buff);                             //Main Memory Page to Buffer 1/2 Transfer 53h/55h
BOOL FlashPageToBuffCompare(word page, byte n_buff);                          //Main Memory Page to Buffer 1/2 Compare  60h/61h
BOOL FlashAutoRewrite(word page, byte n_buff);                                //Auto Page Rewrite trough Buffer 1/2 to 58h/59h
*/

#endif


