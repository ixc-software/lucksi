#ifndef  __macros_
	#define __macros_

//--------------------------------------------------------------------------//
// Header files																//
//--------------------------------------------------------------------------//
#include <sys\exception.h>
#include <cdefBF537.h>
#include <ccblkfn.h>


#define SAMSUNG_MEM


//--------------------------------------------------------------------------//
//#define ECHO_CHIP_MAIN
//#define TWI_DEBUG_MAIN

#define TEST_MAIN
//    #define ECHO_CHIP_PRESENT
//    #define LINE_BERT_FOREVER
    
//#define T1_DEBUG    
//    #define T1_ALARM    


//#define LOAD_SECRET    
//--------------------------------------------------------------------------//    

//#define MEM_TEST_DEBUG
//#define LOW_SYS_FREQ



#ifdef  LOW_SYS_FREQ
    #define SYS_CLK 	40000000UL
#else    
    #define SYS_CLK 	120000000UL
#endif    

#define UART0_SPEED	115200UL
#define UART1_SPEED	115200UL
#define SPI_SPEED	8000000UL

typedef unsigned char  byte;
typedef unsigned int   word;
typedef unsigned long  dword;

extern char buff[];

#endif
