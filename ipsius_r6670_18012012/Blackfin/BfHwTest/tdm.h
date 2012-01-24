#ifndef _TDM_H_
#define _TDM_H_

//--------------------------------------------------------------------------//
// Header files																//
//--------------------------------------------------------------------------//
#include <sys\exception.h>
#include <cdefBF537.h>

struct sDMA_descriptor
	   {
	     struct sDMA_descriptor* next_descriptor;
	     volatile short          *start_address;
       };
typedef struct sDMA_descriptor tDMA_descriptor;


#define Number_of_Samples   20		// 40 samples/channel
#define Number_of_Channels  32		// 32 channels
#define PSP_channels  15


extern int interruptLatch;
extern volatile bool new_block_received;
extern volatile bool block_transmitted;	
extern volatile int Number_of_received_blocks;		
extern volatile int Number_of_transmitted_blocks;	
extern volatile short sSport0RxBuffer[], sSport0TxBuffer[];

//--------------------------------------------------------------------------//
// Prototypes																//
//--------------------------------------------------------------------------//
void Init_Sport0();
void Init_SPORT0_DMA();
void Enable_DMA_Sport0();
void Init_DMA_Interrupts();
void Process_Data();
#endif

