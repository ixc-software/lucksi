#ifndef  __macros_
	#define __macros_

//--------------------------------------------------------------------------//
// Header files																//
//--------------------------------------------------------------------------//
#include <sys\exception.h>
#include <cdefBF537.h>
#include <ccblkfn.h>

#define SYS_CLK 	120000000UL
//#define SYS_CLK 	47500000UL
//#define BAUD_RATE   115200UL
#define BAUD_RATE   19200UL

typedef unsigned char  byte;
typedef unsigned int   word;
typedef unsigned long  dword;

extern byte str[500];
extern byte sbuff[256];
extern byte RXcounter;
extern byte buff[256];      //буфер для вывода отладочной информации	

//---------------------------------------
// для дров Сахарова
typedef unsigned char  BOOL;
   #ifndef TRUE
      #define TRUE  (1)
      #define FALSE (0)
   #endif
  #define CLEAR_BIT(ADDR, BIT) (ADDR &= ~(1 << BIT))
  #define CLR_BIT CLEAR_BIT
  #define SET_BIT(ADDR, BIT) (ADDR |= (1 << BIT))
  #define TEST_BIT(ADDR, BIT) (ADDR & (1 << BIT))   
//---------------------------------------

#endif
