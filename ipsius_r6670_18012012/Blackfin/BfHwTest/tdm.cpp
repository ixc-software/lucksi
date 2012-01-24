#include "macros.h"
#include "tdm.h"

#define Word_Size  			2		// data transfer size in Bytes
#define CLKDIV  			(SYS_CLK/2048000UL/2)-1


// semaphores and counters for reveived and transmitted blocks ( 1 block = 'Number_of_Samples' Frames )
volatile bool new_block_received = false;			// indicates that a full block has been received
volatile bool block_transmitted = false;			// indicates that a processed block has been transmitted

volatile int Number_of_received_blocks =0;			// counter of received blocks
volatile int Number_of_transmitted_blocks =0;		// counter of transmitted blocks

//	Receive Buffer:
//		RIN0(0).. RIN31(0), RIN0(1)..RIN31(1) ... ... RIN0(Number_of_Samples-1)..RIN31(Number_of_Samples-1) ....
//		SIN0(0).. SIN31(0), SIN0(1)..SIN31(1) ... ... SIN0(Number_of_Samples-1)..SIN31(Number_of_Samples-1)
//	Transmit Buffer:
//		ROUT0(0).. ROUT31(0), ROUT0(1)..ROUT31(1) ... ... ROUT0(Number_of_Samples-1)..ROUT31(Number_of_Samples-1) ....
//		SOUT0(0).. SOUT31(0), SOUT0(1)..SOUT31(1) ... ... SOUT0(Number_of_Samples-1)..SOUT31(Number_of_Samples-1)


volatile short sSport0RxBuffer[Number_of_Channels * Number_of_Samples * 2 * 2];
volatile short sSport0TxBuffer[Number_of_Channels * Number_of_Samples * 2 * 2];


extern tDMA_descriptor DMA_TX_second; // declaration
extern tDMA_descriptor DMA_RX_second; // declaration

section("L1_data_a") tDMA_descriptor DMA_TX_first = {&DMA_TX_second, sSport0TxBuffer};
section("L1_data_a") tDMA_descriptor DMA_TX_second = {&DMA_TX_first, (sSport0TxBuffer + sizeof(sSport0TxBuffer)/sizeof(sSport0TxBuffer[0])/2)};
section("L1_data_a") tDMA_descriptor DMA_RX_first = {&DMA_RX_second, sSport0RxBuffer};
section("L1_data_a") tDMA_descriptor DMA_RX_second = {&DMA_RX_first, (sSport0RxBuffer + sizeof(sSport0RxBuffer)/sizeof(sSport0RxBuffer[0])/2)};


// RIN;ROUT - control TDM;
// SIN;SOUT - data TDM;  

volatile short* rin_current_half[2] = 
{
  (sSport0RxBuffer),
  (sSport0RxBuffer + sizeof(sSport0RxBuffer)/sizeof(sSport0RxBuffer[0]) / 2)
};
volatile short* sin_current_half[2] =
{
  (sSport0RxBuffer + sizeof(sSport0RxBuffer)/sizeof(sSport0RxBuffer[0]) / 4),
  (sSport0RxBuffer + sizeof(sSport0RxBuffer)/sizeof(sSport0RxBuffer[0]) * 3 / 4)
};
volatile short* rout_current_half[2] = 
{
  (sSport0TxBuffer),
  (sSport0TxBuffer + sizeof(sSport0TxBuffer)/sizeof(sSport0TxBuffer[0]) / 2)
};
volatile short* sout_current_half[2] = 
{
  (sSport0TxBuffer + sizeof(sSport0TxBuffer)/sizeof(sSport0TxBuffer[0]) / 4),
  (sSport0TxBuffer + sizeof(sSport0TxBuffer)/sizeof(sSport0TxBuffer[0]) * 3 / 4)
};

int interruptLatch;


EX_INTERRUPT_HANDLER(Sport0RxIsr)
{	
	// confirm interrupt handling (NO Error Handling Yet!)
	*pDMA3_IRQ_STATUS |= 0x0001;
	ssync();

	new_block_received = true;					// set semaphore
	Number_of_received_blocks++;				// count
    Process_Data();
}


EX_INTERRUPT_HANDLER(Sport0TxIsr)
{	
	// confirm interrupt handling (NO Error Handling Yet!)
	*pDMA4_IRQ_STATUS |= 0x0001;
	ssync();

	block_transmitted = true;					// set semaphore
	Number_of_transmitted_blocks++;				// count
}

void Init_Sport0()
{		    		
	*pSPORT0_MTCS0 = 0xffffffff;				// enable transmit channels 0-31
	*pSPORT0_MTCS1 = 0x00000000;
	*pSPORT0_MTCS2 = 0x00000000;
	*pSPORT0_MTCS3 = 0x00000000;
	*pSPORT0_MRCS0 = 0xffffffff;				// enable receive channels 0-31
	*pSPORT0_MRCS1 = 0x00000000;
	*pSPORT0_MRCS2 = 0x00000000;
	*pSPORT0_MRCS3 = 0x00000000;
    
	*pSPORT0_RCR1  = RFSR;
	*pSPORT0_RCR2  = 0x0107;
	*pSPORT0_TCR1  = DITFS | TFSR;
	*pSPORT0_TCR2  = 0x0107;
	*pSPORT0_MCMC1 = 0x3000;
	*pSPORT0_MCMC2 = 0x001C;
		
}

void Init_SPORT0_DMA()
{
  // Configure DMA3
  *pDMA3_CONFIG = WNR | WDSIZE_16 | DMA2D | DI_EN  | NDSIZE_4| FLOW_LARGE;
  *pDMA3_NEXT_DESC_PTR = (&DMA_RX_first);	
  *pDMA3_X_COUNT = 2;	
  *pDMA3_X_MODIFY = Number_of_Channels * Number_of_Samples * Word_Size;		
  *pDMA3_Y_COUNT = Number_of_Channels * Number_of_Samples;	
  *pDMA3_Y_MODIFY = (1 - (Number_of_Channels * Number_of_Samples)) * Word_Size;
	
	
  // configure SPORT0 DMA for 2d autobuffer mode (transmit channel 2)
  *pDMA4_CONFIG = WDSIZE_16 | DMA2D | DI_EN  | NDSIZE_4| FLOW_LARGE;
  *pDMA4_NEXT_DESC_PTR = (&DMA_TX_first);
  *pDMA4_X_MODIFY = Number_of_Channels * Number_of_Samples * Word_Size;
  *pDMA4_X_COUNT = 2;
  *pDMA4_Y_MODIFY = (1 - (Number_of_Channels * Number_of_Samples)) * Word_Size;
  *pDMA4_Y_COUNT = Number_of_Channels * Number_of_Samples;

}

void Enable_DMA_Sport0()
{
	// enable DMAs
	*pDMA3_CONFIG	= (*pDMA3_CONFIG | DMAEN);
	*pDMA4_CONFIG	= (*pDMA4_CONFIG | DMAEN);
	ssync();	
	
	// enable Sport0 TX and RX
	*pSPORT0_TCR1 	= (*pSPORT0_TCR1 | TSPEN);
	*pSPORT0_RCR1 	= (*pSPORT0_RCR1 | RSPEN);
	ssync();	
}

void Init_DMA_Interrupts()
{
	// configure interrupts
	register_handler(ik_ivg8, Sport0RxIsr);		// assign ISR to interrupt vector
	register_handler(ik_ivg9, Sport0TxIsr);		// assign ISR to interrupt vector
	
	*pSIC_IAR0 &= 0xF00FFFFF;
	*pSIC_IAR0 |= 0x02100000;
	
	*pSIC_IMASK |= IRQ_DMA3;	
	*pSIC_IMASK |= IRQ_DMA4;			
}



#ifdef ECHO_CHIP_MAIN

	void Process_Data()	
	{
	  int i,j;
	
	  short receive_half;
	  short transmit_half;
				
	  volatile short*	copy_pointer_receive;
	  volatile short*	copy_pointer_transmit;


	  // RIN;ROUT - control TDM;
	  // SIN;SOUT - data TDM;  
  
	  // do processing	
			
	  // calcuate the half of the input and output buffers that can be processed and updated in this cycle
	  receive_half =  (Number_of_received_blocks == (Number_of_received_blocks>>1)<<1) ? 1 : 0;			// just received even or odd? 
	  transmit_half = (Number_of_transmitted_blocks == (Number_of_transmitted_blocks>>1)<<1) ? 1 : 0;	// even or odd NOT being transmitted? 

	  // copy RIN to ROUT (this is always done)
	  copy_pointer_receive = rin_current_half[receive_half];
	  copy_pointer_transmit = sout_current_half[transmit_half];	  
//	  copy_pointer_transmit = rout_current_half[transmit_half];
	  for (i=0; i<Number_of_Channels * Number_of_Samples; i++) *copy_pointer_transmit++ = *copy_pointer_receive++;
	  copy_pointer_transmit = rout_current_half[transmit_half];	  
	  for (i=0; i<Number_of_Channels * Number_of_Samples; i++) *copy_pointer_transmit++ = 0xff;	  
	  

	// copy SIN to SOUT as well
	//  copy_pointer_receive = sin_current_half[receive_half];
	//  copy_pointer_transmit = sout_current_half[transmit_half];
	//  for (i=0; i< Number_of_Channels * Number_of_Samples; i++)	*copy_pointer_transmit++ = *copy_pointer_receive++;    				
	}

#endif

#ifdef TEST_MAIN	

	void Process_Data()	
	{
	  int i,j;
	
	  short receive_half;
	  short transmit_half;
				
	  volatile short*	copy_pointer_receive;
	  volatile short*	copy_pointer_transmit;


	  // RIN;ROUT - control TDM;
	  // SIN;SOUT - data TDM;  
  
	  // do processing	
			
	  // calcuate the half of the input and output buffers that can be processed and updated in this cycle
	  receive_half =  (Number_of_received_blocks == (Number_of_received_blocks>>1)<<1) ? 1 : 0;			// just received even or odd? 
	  transmit_half = (Number_of_transmitted_blocks == (Number_of_transmitted_blocks>>1)<<1) ? 1 : 0;	// even or odd NOT being transmitted? 

	  // copy RIN to ROUT (this is always done)
	  copy_pointer_receive = rin_current_half[receive_half];
	  copy_pointer_transmit = rout_current_half[transmit_half];
	  for (i=0; i< Number_of_Samples; i++)
	  {
		  for (j=0; j< Number_of_Channels; j++)
		  {	  		  	
	  	  	if(j==0) 
	  	  	{
	  	  		*copy_pointer_transmit++ = 0xaa;	  	  			  	
				copy_pointer_receive++;
	  	  		continue;
	  	  	}
	  	  	if(j <= PSP_channels) *copy_pointer_transmit++ = *copy_pointer_receive++;
	  	  	else 
	  	  	{
	  	  		*copy_pointer_transmit++ = 0;
				copy_pointer_receive++;
	  	  	}	  	  	
		  }
	  }

	// copy SIN to SOUT as well
	//  copy_pointer_receive = sin_current_half[receive_half];
	//  copy_pointer_transmit = sout_current_half[transmit_half];
	//  for (i=0; i< Number_of_Channels * Number_of_Samples; i++)	*copy_pointer_transmit++ = *copy_pointer_receive++;    				
	}

#endif	

#if defined (T1_DEBUG) ||  defined (LOAD_SECRET)

	void Process_Data()	
	{
	  int i,j;
	
	  short receive_half;
	  short transmit_half;
				
	  volatile short*	copy_pointer_receive;
	  volatile short*	copy_pointer_transmit;


	  // RIN;ROUT - control TDM;
	  // SIN;SOUT - data TDM;  
  
	  // do processing	
			
	  // calcuate the half of the input and output buffers that can be processed and updated in this cycle
	  receive_half =  (Number_of_received_blocks == (Number_of_received_blocks>>1)<<1) ? 1 : 0;			// just received even or odd? 
	  transmit_half = (Number_of_transmitted_blocks == (Number_of_transmitted_blocks>>1)<<1) ? 1 : 0;	// even or odd NOT being transmitted? 

	  // copy RIN to ROUT (this is always done)
	  copy_pointer_receive = rin_current_half[receive_half];
	  copy_pointer_transmit = rout_current_half[transmit_half];
	  for (i=0; i< Number_of_Samples; i++)
	  {
		  for (j=0; j< Number_of_Channels; j++)
		  {	  		  	
	  	  	if(j==0) 
	  	  	{
	  	  		*copy_pointer_transmit++ = 0xaa;	  	  			  	
				copy_pointer_receive++;
	  	  		continue;
	  	  	}
	  	  	else 
	  	  	{
	  	  		*copy_pointer_transmit++ = *copy_pointer_receive++;
				
	  	  	}	  	  	
		  }
	  }

	// copy SIN to SOUT as well
	//  copy_pointer_receive = sin_current_half[receive_half];
	//  copy_pointer_transmit = sout_current_half[transmit_half];
	//  for (i=0; i< Number_of_Channels * Number_of_Samples; i++)	*copy_pointer_transmit++ = *copy_pointer_receive++;    				
	}

#endif	
