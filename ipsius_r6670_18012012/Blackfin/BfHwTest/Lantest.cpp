#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <cdefBF537.h>
#include <services/services.h>	// for types u32, u16, etc

#include "Lantest.h"
#include "led.h"

/////////
//
/////////

#define TXFRM_SIZE (64-18)
#define RCVE_BUFSIZE 1576

#define NO_TX_BUFS		4
#define NO_RX_BUFS		4



// PHY-related constants
#define BR_BUB_PHYADD	0x01	// Rev 1.1 EZ-KIT, BUB  = 0x1F 

unsigned char g_rx_src_addr[6];
unsigned char g_SrcAddr[6] = {0x00,'E','Z','K','I','T'};



//
//		Set MAC address
//

void SetupMacAddr(unsigned char *mac)
{
	unsigned int lo;
	int i;
	unsigned char byt;
	
	lo = 0;
	for (i=3;i>=0; i--) {
		byt = mac[i];
		lo = (lo<<8) | byt;		
	}
	*pEMAC_ADDRLO = lo;
	
	lo = 0;
	for (i=5;i>=4; i--) {
		byt = mac[i];
		lo = (lo<<8) | byt;		
	}
	*pEMAC_ADDRHI = lo;
}

//
//		Wait until the previous MDC/MDIO transaction has completed
//

void PollMdcDone(void)
{
/*
	unsigned long ulEndTime = (GetTickCount() + 10000);
	// poll the STABUSY bit
	do{
		asm("nop;");
	}while( ((*pEMAC_STAADD) & STABUSY)  && (GetTickCount() < ulEndTime) );
*/	

	unsigned int nTimer = SetTimeout(1000);
	if( ((unsigned int)-1) != nTimer )
	{
		// poll the STABUSY bit
		do{
			asm("nop;");
		}while( ((*pEMAC_STAADD) & STABUSY)  && (!IsTimedout(nTimer)) );
	}
	
	ClearTimeout(nTimer);
}

//
//		Write an off-chip register in a PHY through the MDC/MDIO port
//

void WrPHYReg(u16 PHYAddr, u16 RegAddr, u32 Data)
{
	PollMdcDone();
	
	*pEMAC_STADAT = Data;

	*pEMAC_STAADD = SET_PHYAD(PHYAddr) | SET_REGAD(RegAddr) |
		STAOP | STABUSY;
}

//
//		Read an off-chip register in a PHY through the MDC/MDIO port
//

u16 RdPHYReg(u16 PHYAddr, u16 RegAddr)
{
	PollMdcDone();

	*pEMAC_STAADD = SET_PHYAD(PHYAddr) | SET_REGAD(RegAddr) |
		/*EMAC2_STAOP_RD |*/ STABUSY;
	
	PollMdcDone();
	
	return (u16)*pEMAC_STADAT;
}

//
//		Read all current PHY register values into a memory buffer
//



//
//		Setup various system registers
//

void SetupSystemRegs(void)
{
	unsigned int nTimer;

	// Set FER regs to MUX in Ethernet pins
	// MUX these pins to Ethernet
	*pPORTH_FER = 0xffff;

	// Program EMAC2 System Control Reg
	// set MDC clock divisor (min period is 400 ns)
	// sysctl = EMAC2_MDCDIV(26);	// %54 for 132 MHz SCLK
	// sysctl = EMAC2_MDCDIV(10);	// %22 for  54 MHz SCLK
//	sysctl = SET_MDCDIV(23);		// %23 for  120 MHz SCLK   //KG
	*pEMAC_SYSCTL = ( (SET_MDCDIV(23)) | RXCKS | RXDWA);
	
	WrPHYReg(BR_BUB_PHYADD, PHY_CNTL_REG, PHY_CNTL_RST);
	
//	ulEndTime = (GetTickCount() + 10000);
//	do{
//		asm("nop;");
//	}while( (PHY_CNTL_RST & RdPHYReg(BR_BUB_PHYADD, PHY_CNTL_REG)) && (GetTickCount() < ulEndTime) );


	nTimer = SetTimeout(1000);
	if( ((unsigned int)-1) != nTimer )
	{
		do{
			asm("nop;");
		}while( (PHY_CNTL_RST & RdPHYReg(BR_BUB_PHYADD, PHY_CNTL_REG)) && (!IsTimedout(nTimer)) );
	}
	
	ClearTimeout(nTimer);

	
	// The status register (reg 1) should read 0x7809 to show no link detected
	// Program PHY registers
	WrPHYReg(BR_BUB_PHYADD, PHY_CNTL_REG, (PHY_CNTL_DPLX|PHY_CNTL_ANEG_RST|PHY_CNTL_SPEED) );
	
	// stay here until the link is detected
//	ulEndTime = (GetTickCount() + 10000);
//	do{
//		asm("nop;");
//	}while ( ((PHY_STAT_LINK & RdPHYReg(BR_BUB_PHYADD, PHY_STAT_REG)) == 0) && (GetTickCount() < ulEndTime) );
	
	nTimer = SetTimeout(1000);
	if( ((unsigned int)-1) != nTimer )
	{
		do{
			asm("nop;");
		}while ( ((PHY_STAT_LINK & RdPHYReg(BR_BUB_PHYADD, PHY_STAT_REG)) == 0) && (!IsTimedout(nTimer)) );
	}
	
	ClearTimeout(nTimer);
}

//
//		format a TX  buffer
//

ADI_ETHER_BUFFER *SetupTxBuffer(int datasize,unsigned int root)
{
	ADI_ETHER_FRAME_BUFFER *frmbuf;
	ADI_ETHER_BUFFER *buf;
	unsigned int i;
	int nobytes_buffer = sizeof(ADI_ETHER_BUFFER[2])/2;	// ensure a mult. of 4
	
	// setup a frame of datasize bytes + 14 byte ethernet header
	buf = (ADI_ETHER_BUFFER *)malloc(nobytes_buffer+sizeof(ADI_ETHER_FRAME_BUFFER)+datasize);
	if( NULL == buf)
	{
		return NULL;
	}
	
	frmbuf = (ADI_ETHER_FRAME_BUFFER *)(((char *)buf) + nobytes_buffer);
	
	// set up the buffer
	memset(buf,0,nobytes_buffer);
	buf->Data = frmbuf;
	
	//set up first desc to point to transmit frame buffer
	buf->Dma[0].NEXT_DESC_PTR = &(buf->Dma[1]);
	buf->Dma[0].START_ADDR = (u32)buf->Data;
	// config files alrady zero, so 
	// memory read, linear,  retain fifo data, interrupt after whole buffer, 
	// dma interrupt disabled
	buf->Dma[0].CONFIG.b_DMA_EN = 1;		// enabled
	buf->Dma[0].CONFIG.b_WDSIZE = 2;		// wordsize is 32 bits
	buf->Dma[0].CONFIG.b_NDSIZE = 5;		// 5 half words is desc size.
	buf->Dma[0].CONFIG.b_FLOW   = 7;		// large desc flow
	
	
	//set up second desc to point to status word
	buf->Dma[1].NEXT_DESC_PTR = (DMA_REGISTERS*)NULL;
	buf->Dma[1].START_ADDR = (u32)&buf->StatusWord;
	// config files alrady zero, so 
	// linear,  retain fifo data, interrupt after whole buffer, 
	// dma interrupt disabled
	buf->Dma[1].CONFIG.b_DMA_EN = 1;		// enabled
	buf->Dma[1].CONFIG.b_WNR    = 1;		// write to memory
	buf->Dma[1].CONFIG.b_WDSIZE = 2;		// wordsize is 32 bits
	buf->Dma[1].CONFIG.b_NDSIZE = 0;		// 0 when FLOW is 0.
	buf->Dma[1].CONFIG.b_FLOW   = 0;		// stop
	
	// set up the frame buffer
	frmbuf->NoBytes = 14+datasize; // ethernet header
	memset(frmbuf->Dest, 0xFF, 6);  //memcpy(frmbuf->Dest,DstAddr,6);
	memcpy(frmbuf->Srce, g_SrcAddr,6);
	frmbuf->LTfield = datasize;
	
	for(i=0; i<datasize; i++) frmbuf->Data[i] = (u8)(i+root)&0xff;
//	for(i=0; i<datasize; i++) frmbuf->Data[i] = 0xaa;
	
	return buf;
	
}


//
//		setup a RX  buffer
//

ADI_ETHER_BUFFER *SetupRxBuffer(void)
{
	ADI_ETHER_FRAME_BUFFER *frmbuf;
	ADI_ETHER_BUFFER *buf;
	int i;
	int nobytes_buffer = sizeof(ADI_ETHER_BUFFER[2])/2;	// ensure a mult. of 4
	
	// setup a frame of datasize bytes + 14 byte ethernet header
	buf = (ADI_ETHER_BUFFER *)malloc(nobytes_buffer+sizeof(ADI_ETHER_FRAME_BUFFER)+RCVE_BUFSIZE);
	if( NULL == buf)
	{
		return NULL;
	}
		
	frmbuf = (ADI_ETHER_FRAME_BUFFER *)(((char *)buf) + nobytes_buffer);
	
	// set up the buffer
	memset(buf,0,nobytes_buffer);	// clear ether buf
	buf->Data = frmbuf;
	memset(frmbuf, 0xfe, RCVE_BUFSIZE);	// background pattern for data buf
	
	//set up first desc to point to receive frame buffer
	buf->Dma[0].NEXT_DESC_PTR = &(buf->Dma[1]);
	buf->Dma[0].START_ADDR = (u32)buf->Data;
	// config files alrady zero, so 
	// linear,  retain fifo data, interrupt after whole buffer, 
	// dma interrupt disabled
	buf->Dma[0].CONFIG.b_DMA_EN = 1;		// enabled
	buf->Dma[0].CONFIG.b_WNR    = 1;		// write to memory
	buf->Dma[0].CONFIG.b_WDSIZE = 2;		// wordsize is 32 bits
	buf->Dma[0].CONFIG.b_NDSIZE = 5;		// 5 half words is desc size.
	buf->Dma[0].CONFIG.b_FLOW   = 7;		// large desc flow
	
	
	//set up second desc to point to status word
	buf->Dma[1].NEXT_DESC_PTR = (DMA_REGISTERS*)NULL;
	buf->Dma[1].START_ADDR = (u32)&buf->IPHdrChksum;

	// config files alrady zero, so 
	// linear,  retain fifo data, interrupt after whole buffer, 
	// dma interrupt disabled, and zro size next desc
	buf->Dma[1].CONFIG.b_DMA_EN = 1;		// enabled
	buf->Dma[1].CONFIG.b_WNR    = 1;		// write to memory
	buf->Dma[1].CONFIG.b_WDSIZE = 2;		// wordsize is 32 bits
	buf->Dma[1].CONFIG.b_FLOW   = 0;		// stop
	
	return buf;
}





int LanTest()
{
	int iResult = 1;
	//unsigned long ulEndTime;
	unsigned int nTimer;
	
	ADI_ETHER_BUFFER *txbuf = NULL;
	ADI_ETHER_BUFFER *txfst = NULL;
	ADI_ETHER_BUFFER *txlst = NULL;
	ADI_ETHER_BUFFER *rxfst = NULL;
	ADI_ETHER_BUFFER *rxbuf = NULL;
	ADI_ETHER_BUFFER *rxlst = NULL;
	
	int ib;
	int irxh;
	int itxh;
	
	int RxCounter = 0;
	
	u32 txstatus;
	u32 rxstatus;
	
	// first, we setup various non-EMAC stuff
	SetupSystemRegs();
	
	// next, we set up a MAC ADDRESS
	SetupMacAddr(g_SrcAddr);
	
	// initialize the TX DMA channel registers
	*pDMA2_X_COUNT  = 0;
	*pDMA2_X_MODIFY = 4;
	
	// initialize the RX DMA channel registers
	*pDMA1_X_COUNT  = 0;
	*pDMA1_X_MODIFY = 4;
	
	// set up a transmit frames and form a chain of them
	txlst = NULL;
	txfst = NULL;
	for( ib = 0; ib < NO_TX_BUFS; ib++ ) 
	{
		txbuf = SetupTxBuffer(TXFRM_SIZE,ib);
		if( NULL == txbuf )
		{	// memory allocation failed
			return 0;
		}
		
		if (txfst==NULL) 
			txfst = txbuf;
		if (txlst != NULL) 
		{
			txlst->pNext = txbuf;							// chain this buffer on
			txlst->Dma[1].NEXT_DESC_PTR = &txbuf->Dma[0];	// chain the descriptors
			txlst->Dma[1].CONFIG.b_NDSIZE = 5;				// five elements
			txlst->Dma[1].CONFIG.b_FLOW   = 7;				// large descriptors
		}
		txlst = txbuf;
	}
	// loop the transmit chain
	txlst->Dma[1].NEXT_DESC_PTR = &txfst->Dma[0];	
	txlst->Dma[1].CONFIG.b_NDSIZE = 5;				// five elements
	txlst->Dma[1].CONFIG.b_FLOW   = 7;				// large descriptors
	txlst->pNext = txfst;
	
	// set up a receive frames and form a chain of them
	rxlst = NULL;
	rxfst = NULL;
	for (ib=0;ib<NO_RX_BUFS;ib++) 
	{
		rxbuf = SetupRxBuffer();
		if( NULL == rxbuf )
		{	// memory allocation failed
			return 0;
		}
		
		if (rxfst==NULL) 
			rxfst = rxbuf;
		if (rxlst != NULL) 
		{
			rxlst->pNext = rxbuf;							// chain this buffer on
			rxlst->Dma[1].NEXT_DESC_PTR = &rxbuf->Dma[0];	// chain the descriptors
			rxlst->Dma[1].CONFIG.b_NDSIZE = 5;				// five elements
			rxlst->Dma[1].CONFIG.b_FLOW   = 7;				// large descriptors
		}
		rxlst = rxbuf;
	}
	// loop the receive chain
	rxlst->Dma[1].NEXT_DESC_PTR = &rxfst->Dma[0];	
	rxlst->Dma[1].CONFIG.b_NDSIZE = 5;			// five elements
	rxlst->Dma[1].CONFIG.b_FLOW   = 7;			// large descriptors		
	rxlst->pNext = rxfst;
	
	// start the TX DMA channel before enabling the MAC
	txbuf = txfst;
	if (txfst != NULL) 
	{
		*pDMA2_NEXT_DESC_PTR = &txfst->Dma[0];
		*pDMA2_CONFIG =  *((u16*)&txfst->Dma[0].CONFIG);
	}

	// start the RX DMA channel before enabling the MAC
	rxbuf = rxfst;
	if (rxfst != NULL) 
	{
		*pDMA1_NEXT_DESC_PTR = &rxfst->Dma[0];
		*pDMA1_CONFIG = *((u16*)&rxfst->Dma[0].CONFIG);
	}
	
	// reset counters,                allow rollover, enable counters
	*pEMAC_MMC_CTL = RSTC | CROLL | MMCE;
	
	// finally enable sending/receiving at the mac
	
	
	PollMdcDone();

	// enable RX, TX, and full duplex
	*pEMAC_OPMODE = (TE | RE | FDMODE);
	
	// now poll for completion on the tx and rx buffers
	txbuf = txfst;
	itxh  = 0;
	
	rxbuf = rxfst;
	irxh  = 0;

//	ulEndTime = (GetTickCount() + 0x200000);
	nTimer = SetTimeout(15000);
	while ( ((rxbuf!=NULL) || (txbuf!=NULL)) ) 
	{
		if( IsTimedout(nTimer) )
		{
			iResult = 0;
			break;
		}
		else
		{
			if ((txbuf != NULL) && ((txbuf->StatusWord & TX_COMP) !=0)) 
			{
				
				
				// enable the transmit DMA for the sending the next packets
				PollMdcDone();
				if( NO_TX_BUFS > 0 )
				{
					*pEMAC_OPMODE &= (~TE);
				}
				
				// frame marked as transmitted
				txstatus = txbuf->StatusWord; // save the status
				txbuf->StatusWord = 0;
				if(TX_OK != (txstatus & TX_OK) )
				{
					iResult = 0;
					break;
				}
				
				txbuf = txbuf->pNext;
				// store current status in history list
				if (++itxh >= NO_TX_BUFS) 
				{
					itxh = 0;
				}
				
				// enable the transmit DMA for the sending the next packets
				// start the TX DMA channel before enabling the MAC
				txbuf = txfst;
				if (txfst != NULL) 
				{
					*pDMA2_NEXT_DESC_PTR = &txfst->Dma[0];
					*pDMA2_CONFIG =  *((u16*)&txfst->Dma[0].CONFIG);
				}
				
				PollMdcDone();
				if( NO_TX_BUFS > 0 ) 
				{
					*pEMAC_OPMODE |= TE;
				}
			}
			
			
			if ((rxbuf != NULL) && ((rxbuf->StatusWord & RX_COMP) !=0)) 
			{
				// frame marked as received
				rxstatus = rxbuf->StatusWord; // save the status
				rxbuf->StatusWord = 0;
				if( rxstatus & RX_OK ) 
				{
					// recvved OK
					memcpy(g_rx_src_addr,rxbuf->Data->Srce,6);	
					RxCounter++;
	
					if ((RxCounter % 1000000) == 0x0) 
					{
						iResult = 1;
						break;
					}
	
					memset(g_rx_src_addr,0x0,6);
				}
				else
				{
					iResult = 0;
					break;
				}
					
				rxbuf = rxbuf->pNext;
				if (++irxh >= NO_RX_BUFS) 
				{
					irxh = 0;
				}
			}
		}// end else(tickcount)
	}
	
	ClearTimeout(nTimer);

	*pEMAC_OPMODE = 0;

	if( iResult )
	{	// this will verify that the correct PHY address is being used
		if( 0xFFFF == RdPHYReg(BR_BUB_PHYADD, 31) )
		{
			iResult = 0;
		}
	}
	
	// delete the tx & rx buffers
	{
		ADI_ETHER_BUFFER *pHead;
		ADI_ETHER_BUFFER *pTemp = txfst;
		
		do{
			pHead = pTemp;
			pTemp = pHead->pNext;
			pHead->pNext = NULL;
			memset(pHead, 0x00, sizeof(ADI_ETHER_BUFFER) );
			free(pHead);
		}while( NULL != pTemp->pNext );
		
		pTemp = rxfst;
		do{
			pHead = pTemp;
			pTemp = pHead->pNext;
			pHead->pNext = NULL;
			memset(pHead, 0x00, sizeof(ADI_ETHER_BUFFER) );
			free(pHead);
		}while( NULL != pTemp->pNext );
				
	}
	
	return iResult;	
}
