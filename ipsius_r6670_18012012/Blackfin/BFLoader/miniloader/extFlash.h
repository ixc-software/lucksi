

#if !defined(__EXTFLASH__)

  #define __EXTFLASH__

void EF_ReadToBuff(word page, byte *pBuff);
byte EF_WriteFromBuff(word page, byte *pBuff);
byte EF_Init();
byte EF_FlashWaitReady();
  
void EF_FlashSectorErase(byte sector);
void EF_FlashChipErase();
  


#endif


