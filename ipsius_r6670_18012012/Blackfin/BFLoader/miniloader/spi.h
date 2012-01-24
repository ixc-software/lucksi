

#if !defined(__SPI__)

#define __SPI__

#include "../macros.h"

#define SELECT()		*pPORTFIO_CLEAR = SF_CS
#define DESELECT()		*pPORTFIO_SET = SF_CS
#define SF_CS			0x0400

void SpiInit(void);
byte Spi(byte data);

#endif


