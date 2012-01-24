/*********************************************************************************

Copyright(c) 2004 Analog Devices, Inc. All Rights Reserved.

This software is proprietary and confidential.  By using this software you agree
to the terms of the associated Analog Devices License Agreement.


*********************************************************************************/


/*********************************************************************

Include files

*********************************************************************/


#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <stdio.h>
#include <math.h>
#include <services/services.h>			// system service includes
#include <drivers/adi_dev.h>			// device manager includes
#include <ADI_ETHER_BF537.h>			// BF537 ether driver includes
#include <ccblkfn.h>
#include <cdefbf537.h>
#include <time.h>


/*********************************************************************

Enumerations and defines

*********************************************************************/

//#define UART_DEBUG


#ifdef UART_DEBUG
	char buffer[200];
	#define UART_DEBUG_MESSAGE(P)  	Uart0Send(P)
#else
	#define UART_DEBUG_MESSAGE(P)
#endif


#define MAX_RCVE_FRAME 1560


//#define USE_SYNC


#ifdef ADI_ETHER_BF537_DEBUG
#define CHECK_QUEUES 0
#else
#define CHECK_QUEUES 0
#endif


// PHY-related constants
#define NO_PHY_REGS     0x20

#define PHY_MICREL_8895   1
#define PHY_LAN83   	  2
#define PHY_BAD_DEV   	  0


// -----------------------------------------------------------------------
//                     PHY REGISTER NAMES				//
// -----------------------------------------------------------------------
#define PHYREG_MODECTL		0x0000
//#define PHYREG_MODESTAT		0x0001
#define PHYREG_PHYID1		0x0002
#define PHYREG_PHYID2		0x0003
#define PHYREG_ANAR			0x0004
#define PHYREG_ANLPAR		0x0005
/*
#define PHYREG_ANER			0x0006
#define PHYREG_NSR			0x0010
#define PHYREG_LBREMR		0x0011
#define PHYREG_REC			0x0012
#define PHYREG_10CFG		0x0013
#define PHYREG_PHY1_1		0x0014
#define PHYREG_PHY1_2		0x0015
#define PHYREG_PHY2			0x0016
#define PHYREG_TW_1			0x0017
#define PHYREG_TW_2			0x0018
#define PHYREG_TEST			0x0019
*/

#define PHYREG_INT_SRC_FLG          29
#define PHYREG_INT_MASK             30
#define PHYREG_SPEED_IND            31

#define MICREL_CHIPID0  0
#define MICREL_START_SW 1

#define MICREL_MAC0 104
#define MICREL_MAC1 105
#define MICREL_MAC2 106
#define MICREL_MAC3 107
#define MICREL_MAC4 108
#define MICREL_MAC5 109


#define MII_LAN83C185_ISF_INT1      (1 << 1) /* Auto Negotiation Page received INT1 */
#define MII_LAN83C185_ISF_INT2      (1 << 2) /* parallel detection fault INT2 */
#define MII_LAN83C185_ISF_INT3      (1 << 3) /* Auto negotiation LP acknowledgement INT3 */
#define MII_LAN83C185_ISF_INT4      (1 << 4) /* Link down INT4 */
#define MII_LAN83C185_ISF_INT5      (1 << 5) /* Remote fault detected INT5 */
#define MII_LAN83C185_ISF_INT6      (1 << 6) /* Auto negotiation completed INT6 */
#define MII_LAN83C185_ISF_INT7      (1 << 7) /* Energy ON generated INT7 */

/* By default Auto Negotiation complete and Link down interrupts were enabled */
#define MII_LAN83C185_ISF_DEFAULT  (MII_LAN83C185_ISF_INT4 | MII_LAN83C185_ISF_INT6 | MII_LAN83C185_ISF_INT7)

#define	EMAC2_STAOP_RD	( 0<< 1 )	// Station Management Operation=Read
#define	EMAC2_STAOP_WR	( 1<< 1 )	// Station Management Operation=Write


/*********************************************************************

Data Structures

*********************************************************************/

typedef struct adi_lan983c185_phy_int_counters {
	s32 cPhyIntAutoNegotiationPageRcvd; /* INT 1 */
	s32 cPhyIntParallelDetectionFault;  /* INT 2 */
	s32 cPhyIntAutoNegotiationLPAck;    /* INT 3 */
	s32 cPhyIntLinkDown;                /* INT 4 */
	s32 cPhyIntRemoteFaultDetected;     /* INT 5 */
	s32 cPhyIntAutoNegotiationComplete; /* INT 6 */
	s32 cPhyIntEnergyOn;                /* INT 7 */
}ADI_LAN983C185_PHY_INT_COUNTERS;


typedef struct dma_descriptor {
	struct dma_descriptor	*NEXT_DESC_PTR;
	unsigned long int		START_ADDR;
	u16						CONFIG;
	u16						X_COUNT;
} DMA_DESCRIPTOR;


typedef volatile struct  DMA_REGISTERS {
	DMA_DESCRIPTOR			*NEXT_DESC_PTR;
	u32						START_ADDR;
	u16						CONFIG;
	u16						pad0;
	u32						pad1;
	u16						X_COUNT;
	u16						pad2;
	s16						X_MODIFY;
	u16						pad3;
	u16						Y_COUNT;
	u16						pad4;
	s16						Y_MODIFY;
	u16						pad5;
	u32						CURR_DESC;
	u32						CURR_ADDR;
	u16						IRQ_STATUS;
	u16						pad6;
	u16						PERIPHERAL_MAP;
	u16						pad7;
	u16						CURR_X_COUNT;
	u16						pad8;
	u32						pad9;
	u16						CURR_Y_COUNT;
	u16						pad10;
} DMA_REGISTERS;

typedef volatile struct status_area {
	u16	IPHdrChksum;		// the IP header checksum
	u16	IPPayloadChksum;	// the IP header and payload checksum
	u32	StatusWord;			// the frame status word
	volatile struct status_area		*Next;	// next status area
} STATUS_AREA;

typedef struct adi_ether_frame_buffer {
	u16		NoBytes;		/* the no. of following bytes	*/
	u8		Dest[6];		/* destination MAC address	*/
	u8		Srce[6];		/* source MAC address	*/
	u8		LTfield[2];		/* length/type field	*/
	u8		Data[0];		/* payload bytes	*/
} ADI_ETHER_FRAME_BUFFER;



typedef struct buffer_info {
	// this structure is overlaid on the start of the ADI_ETHER_BUFFER structure on the area reserved for the physical dev. driver
	DMA_DESCRIPTOR			*First;			// first descriptor associated with buffer
	DMA_DESCRIPTOR			*Last;			// last descriptor associated with buffer
	int						NoDesc;			// no. of descriptors associated with buffer
	STATUS_AREA				*Status;		// address of the status word area
} BUFFER_INFO;

typedef struct frame_info {
	u16						IPHdrChksum;		// the IP header checksum
	u16						IPPayloadChksum;	// the IP header and payload checksum
	u32						StatusWord;			// the frame status word
} FRAME_INFO;
typedef struct frame_queue {
	DMA_REGISTERS		*Dma;			// base address of DMA channel registers

	ADI_ETHER_BUFFER		*Active;		// list of curently active frames
	ADI_ETHER_BUFFER		*Pending;		// list of pending frames
	ADI_ETHER_BUFFER		*Completed;		// list of completed frames
	ADI_ETHER_BUFFER		*Queued;		// list of buffers awaiting descriptors
	DMA_DESCRIPTOR			*Avail;			// available dma registers structures
	STATUS_AREA				*AvailStatus;	// available status areas
	int						NoAvail;		// no. of available dma register structures
	int						Channel;		// DMA channel
	int						NoCompletions;
	int						UnProcessed;	// no. of unprocessed buffers
	u32						EnableMac;		// command to enable the MAC, reset to zero one enabled
	bool					Rcve;			// if controlling receiving
	bool					Enabled;		// set once the DMA has been enabled once
	u16						CompletedStatus;// mask to determine if frame has completed

} FRAME_QUEUE;


typedef struct adi_ether_bf537_data {
	void					*CriticalData;
	ADI_DEV_DEVICE_HANDLE	DeviceHandle;	// device handle
	ADI_DMA_MANAGER_HANDLE	DMAHandle;		// handle to the DMA manager
	ADI_DCB_HANDLE			DCBHandle;		// callback handle
	ADI_DCB_CALLBACK_FN		DMCallback;		// client callback function
	ADI_DEV_DIRECTION 		Direction;		// data direction
	bool 					Open;			// device open
	bool					Started;		// device started
	bool					Closing;		// refuse new requests

	//
	FRAME_QUEUE				Rx;				// receive queues
	FRAME_QUEUE				Tx;				// transmit quueues
	ADI_ETHER_BF537_TRACE_DATA Trc;			// trace support data
	int						MaxTraceEntries;// maximum number of trace entries
	int						TraceMaxBytes;	// maximum no. of bytes to be traced
	int						TraceSequence;
	int						TraceFirstByte;
	int						CLKIN;			// clock in value in MHZ


	int						FlowEnabled;	// record if data flow is active
	int						EtherIntIVG;	// IVG for the ethernet interrupt
	int						RXIVG;			// IVG for the RX completion
	int						TXIVG;			// IVG for the TX completion
	int						PhyAddr;		// PHY address
	int						OpMode;			// set these bits n the OPMODE regs
	bool					Port10;			// set port speed to 10 Mbit/s
	bool					GenChksums;		// IP checksums to be calculated
	bool					NoRcveLnth;		// dont insert recv length at start of buffer
	bool					StripPads;		// remove trailing pad bytes
	bool					FullDuplex;		// set full duplex mode
	bool					Negotiate;		// enable auto negotiation
	bool					Loopback;		// loopback at the PHY
	bool					Cache;			// Buffers may be cached
	bool					FlowControl;	// flow control active
	unsigned short			IntMask;		// interrupt mask
	unsigned char			Mac[6];			// MAC address of the board
	ADI_ETHER_STATISTICS_COUNTS	*Stats;
	bool 					bEnableGracefulShutdown; /*flag for graceful shut down the driver */
	ADI_LAN983C185_PHY_INT_COUNTERS        PhyInterruptCounters;    /* Phy interrupt counters */
	u32                                   PhyInterruptMask;        /* Phy Interrupt Mask */                              
} ADI_ETHER_BF537_DATA;



/*********************************************************************

Static data and processor specific macros

*********************************************************************/


static ADI_ETHER_BF537_DATA EtherDev;				// control data pointer

static int NoTxInts=0,NoRxInts=0;
static int NoTxPosts=0,NoRxPosts=0;
static int NoTxStarts,NoRxStarts;
static int FailedPosts;
static int TxErr;

static u8  PhyDevType;



#define FLUSH(P)  {asm volatile("FLUSH[%0++];":"+p"(P));}
//###define FLUSH(P)  asm volatile("NOP;":"+p"(P));
#define FLUSHINV(P)  {asm volatile("FLUSHINV[%0++];":"+p"(P));}
#define SIMPLEFLUSHINV(P)  {ssync(); asm volatile("FLUSHINV[%0++];"::"#p"(P)); ssync();}

/*********************************************************************

Static functions

*********************************************************************/
static void SetupPinMux(void);

static void ExitCriticalRegion(void *xit)
{
	sti((unsigned int)xit);
}

static void* EnterCriticalRegion(void *xit)
{
	unsigned int im = cli();
	sti(EtherDev.IntMask & im); // disable our interrupts
	return (void *)im;
}


#if CHECK_QUEUES
static int CheckQueues(FRAME_QUEUE *q);
#endif

#if 1
static void FlushArea(void *start, void *nd)
{
	start = (void *)(((unsigned int)start)&(~31));
	ssync();
	while (start<nd)
		FLUSH(start);
	ssync();
}


static void FlushInvArea(void *start, void *nd)
{
	start = (void *)(((unsigned int)start)&(~31));
	ssync();
	while (start<nd) FLUSHINV(start);
	ssync();
}
#else
#define FlushArea(s,n)
#define FlushInvArea(s,n)
#endif

static int GetTcpSeqNo(ADI_ETHER_BUFFER *pBuffer)
{
		unsigned short *p,v;
		char *l;
		int u;


		// get protocol
		l = ((char*)(pBuffer->Data) + 23 + 2);
		// tcp protocol
		if(*l == 6)
		{
			v=0;
			l = ((char*)((pBuffer)->Data) + 40 + 2);
			// p points to the seq-num we are not getting only short
			// good enough for testing.
			p = ((unsigned short*)l);

			// change byte-order
			v = ( ((*p & 0xFF) << 8) | (*p >> 8));


		} else v = 0xffff;
		return v;
}

static DumpTraceBuffer(void)
{
	if (EtherDev.MaxTraceEntries>0) {
		ADI_ETHER_BF537_DATA *dev = &EtherDev;
		int i,j;
		ADI_ETHER_BF537_TRACE_ENTRY *te;

		te = (dev->Trc.NoOfEntries<=dev->MaxTraceEntries?dev->Trc.BaseEntry:dev->Trc.OldestEntry);

		for (i=0;i<dev->Trc.NoOfEntries;i++) {
			te = (ADI_ETHER_BF537_TRACE_ENTRY *)(((char *)te) + dev->Trc.EntryLnth);
			switch (te->Dirn) {
			case 'T':
			case 'R':
				printf("%c %4.4x %4.4x ",te->Dirn,te->Seqn,te->NoBytes);
				for (j=0;j<te->NoBytes;j++) {
					printf("%2.2x ",te->Data[j]);
				}
				printf("\n");
				break;
			}
			if (te>=dev->Trc.EndOfData) te = dev->Trc.BaseEntry;
		}

	}
}


static u32 adi_pdd_Open(				// Open a device
	ADI_DEV_MANAGER_HANDLE	ManagerHandle,		// device manager handle
	u32 					DeviceNumber,		// device number
	ADI_DEV_DEVICE_HANDLE 	DeviceHandle,		// device handle
	ADI_DEV_PDD_HANDLE 		*pPDDHandle,		// pointer to PDD handle location
	ADI_DEV_DIRECTION 		Direction,			// data direction
	void					*pEnterCriticalArg,	// enter critical region parameter
	ADI_DMA_MANAGER_HANDLE	DMAHandle,			// handle to the DMA manager
	ADI_DCB_HANDLE			DCBHandle,			// callback handle
	ADI_DCB_CALLBACK_FN		DMCallback			// device manager callback function
);

static u32 adi_pdd_Close(		// Closes a device
	ADI_DEV_PDD_HANDLE PDDHandle	// PDD handle
);

static u32 adi_pdd_Read(		// Reads data or queues an inbound buffer to a device
	ADI_DEV_PDD_HANDLE PDDHandle,	// PDD handle
	ADI_DEV_BUFFER_TYPE	BufferType,	// buffer type
	ADI_DEV_BUFFER *pBuffer			// pointer to buffer
);

static u32 adi_pdd_Write(		// Writes data or queues an outbound buffer to a device
	ADI_DEV_PDD_HANDLE PDDHandle,	// PDD handle
	ADI_DEV_BUFFER_TYPE	BufferType,	// buffer type
	ADI_DEV_BUFFER *pBuffer			// pointer to buffer
);

static u32 adi_pdd_Control(		// Sets or senses a device specific parameter
	ADI_DEV_PDD_HANDLE PDDHandle,	// PDD handle
	u32 Command,					// command ID
	void *pArg						// pointer to argument
);


static ADI_INT_HANDLER(DmaErrorInterruptHandler);
static ADI_INT_HANDLER(EtherInterruptHandler);
static ADI_INT_HANDLER(RxInterruptHandler);
static ADI_INT_HANDLER(TxInterruptHandler);
static void QueueFrames(ADI_ETHER_BF537_DATA *dev, FRAME_QUEUE *q);



#if defined(ADI_ETHER_DEBUG)

static int ValidatePDDHandle(ADI_DEV_PDD_HANDLE PDDHandle);

#endif

#if defined(ADI_ETHER_DEBUG)
#define CHECK_RES if (res == 0) res
#else
#define CHECK_RES res
#endif

/*********************************************************************

Global data

*********************************************************************/


ADI_DEV_PDD_ENTRY_POINT ADI_ETHER_BF537_Entrypoint = {
	adi_pdd_Open,
	adi_pdd_Close,
	adi_pdd_Read,
	adi_pdd_Write,
	adi_pdd_Control
};


#ifdef ADI_ETHER_BF537_DEBUG
#define CheckBuffers() ADI_ETHER_BF537_CheckBuffers()
void ADI_ETHER_BF537_CheckBuffers(void);
#else
#define CheckBuffers() 0
#endif

//
//  checkbuffers
//
typedef struct buffer {
	struct buffer			*next;
} XBUFFER;

XBUFFER *ADI_ETHER_BF537_FreeBuf;
int ADI_ETHER_BF537_NoOfBuffers;
int ADI_ETHER_BF537_NoOfFreeBuffers;
int ADI_ETHER_BF537_NoPosted;
void *ADI_ETHER_BF537_PostedBuffer;
int ADI_ETHER_BF537_OutstandingPosts;

#if CHECK_QUEUES


static void breakpoint(int noq)
{
	if (noq>2) {
		int hh=noq;
	}
}
//
//
//		CheckQueuesForBuffer
static void CheckQueuesForBuffer(FRAME_QUEUE *q, ADI_ETHER_BUFFER *buf)
{
	int noq = 0;
	ADI_ETHER_BUFFER *nxt;
	int inact=0,inpend=0,incomp=0,inqu=0;

	nxt = q->Active;
	while (nxt != NULL) {
		if (nxt == buf) {
			inact = 1;
			breakpoint(++noq);
		}
		nxt = nxt->pNext;
	}
	nxt = q->Pending;
	while (nxt != NULL) {
		if (nxt == buf) {
			inpend = 1;
			breakpoint(++noq);
		}
		nxt = nxt->pNext;
	}
	nxt = q->Completed;
	while (nxt != NULL) {
		if (nxt == buf) {
			incomp = 1;
			breakpoint(++noq);
		}
		nxt = nxt->pNext;
	}
	nxt = q->Queued;
	while (nxt != NULL) {
		if (nxt == buf) {
			inqu = 1;
			breakpoint(++noq);
		}
		nxt = nxt->pNext;
	}

}

static int CheckQueues(FRAME_QUEUE *q)
{
	ADI_ETHER_BUFFER *nxt;
	int na =0;
	DMA_DESCRIPTOR *dmr;
	int nb=0;
	extern int CurRcve,CurXmit;

	nxt = q->Active;
	while (nxt != NULL) {
		nb++;
		CheckQueuesForBuffer(q,nxt);
		nxt = nxt->pNext;
	}
	nxt = q->Pending;
	while (nxt != NULL) {
		nb++;
		CheckQueuesForBuffer(q,nxt);
		nxt = nxt->pNext;
	}
	nxt = q->Completed;
	while (nxt != NULL) {
		nb++;
		CheckQueuesForBuffer(q,nxt);
		nxt = nxt->pNext;
	}
	nxt = q->Queued;
	while (nxt != NULL) {
		nb++;
		CheckQueuesForBuffer(q,nxt);
		nxt = nxt->pNext;
	}
	// next we check the Avail queue
	dmr = q->Avail;
	while (dmr != NULL ) {
		na++;
		dmr = dmr->NEXT_DESC_PTR;
	}
	if (na != q->NoAvail) breakpoint(2);

	CheckBuffers();
	return nb;

}


static int CountQueue(FRAME_QUEUE *q)
{
	ADI_ETHER_BUFFER *nxt;
	int nb=0;

	nxt = q->Active;
	while (nxt != NULL) {
		nb++;
		nxt = nxt->pNext;
	}
	nxt = q->Pending;
	while (nxt != NULL) {
		nb++;
		nxt = nxt->pNext;
	}
	nxt = q->Completed;
	while (nxt != NULL) {
		nb++;
		nxt = nxt->pNext;
	}
	nxt = q->Queued;
	while (nxt != NULL) {
		nb++;
		nxt = nxt->pNext;
	}

	return nb;

}

void ADI_ETHER_BF537_CheckBuffers()
{
	int nr,nt,nf,nx;
	XBUFFER *xb;

	nr = CountQueue(&EtherDev.Rx);
	nt = CountQueue(&EtherDev.Tx);
	nf = ADI_ETHER_BF537_NoOfFreeBuffers;
	if (nt+nr+nf+ADI_ETHER_BF537_NoPosted < ADI_ETHER_BF537_NoOfBuffers-1) breakpoint(4);
}


#else
#define CheckQueues(q)
#define breakpoint(p)
void ADI_ETHER_BF537_CheckBuffers()
{
}

#endif


//
//		Set FER regs to MUX in Ethernet pins
//

static void SetupPinMux(void)
{
	unsigned int fer_val;
	unsigned int *p = &fer_val;

	// FER reg bug work-around
	// read it once
	fer_val = *pPORTH_FER;

	fer_val = 0xffff;

	// write it twice to the same value

	*pPORTH_FER = fer_val;
	*pPORTH_FER = fer_val;

}

//
//		Wait until the previous MDC/MDIO transaction has completed
//

static void PollMdcDone(void)
{
	// poll the STABUSY bit
	while((*pEMAC_STAADD) & STABUSY) {};
}


//
//		Read an off-chip register in a PHY through the MDC/MDIO port
//

static u16 RdPHYReg(u16 PHYAddr, u16 RegAddr)
{
	PollMdcDone();

	*pEMAC_STAADD = SET_PHYAD(PHYAddr) | SET_REGAD(RegAddr) |
		EMAC2_STAOP_RD | STABUSY;

	PollMdcDone();

	return (u16)*pEMAC_STADAT;
}

//
//		Write an off-chip register in a PHY through the MDC/MDIO port
//

static void RawWrPHYReg(u16 PHYAddr, u16 RegAddr, u32 Data)
{

	*pEMAC_STADAT = Data;

	*pEMAC_STAADD = SET_PHYAD(PHYAddr) | SET_REGAD(RegAddr) |
		EMAC2_STAOP_WR | STABUSY;

	PollMdcDone();
}

static void WrPHYReg(u16 PHYAddr, u16 RegAddr, u32 Data)
{
	PollMdcDone();
	RawWrPHYReg(PHYAddr,RegAddr,Data);
}


static u8 CalcPhyAddr(u8 RegAddr)
{
	u8 PHYAddr = RegAddr >> 3;
	PHYAddr |= 0x06;
	PHYAddr &= 0xFE;
	if(RegAddr & 0x20) PHYAddr |= 0x01;		
	return PHYAddr;
}

static void WrMicrelSMI(u8 RegAddr, u8 Data)
{
	WrPHYReg(CalcPhyAddr(RegAddr), RegAddr, Data);
}

static u8 RdMicrelSMI(u8 RegAddr)
{
	return RdPHYReg(CalcPhyAddr(RegAddr), RegAddr);
}


//--------------------------------------------------------------------------

static void SetPhyDevType()
{
	u16 reg2, reg3;		
	u16  sign;

	reg2 = RdPHYReg(1,PHYREG_PHYID1); // read PHY id 1
	reg3 = RdPHYReg(1,PHYREG_PHYID2); // read PHY id 2
	PhyDevType = PHY_BAD_DEV;
	if ((reg2 == 0x07) && ((reg3>>4) == 0xc0a))
	{
	    PhyDevType = PHY_LAN83;
		UART_DEBUG_MESSAGE("Lan83 Finded\n");	    
	}
	if ((reg2 == 0x0022) && (reg3 == 0x1450))
	{
		PhyDevType = PHY_MICREL_8895;	
		UART_DEBUG_MESSAGE("Micrel8895 Finded\n");
		WrMicrelSMI(MICREL_START_SW,1);
	#ifdef UART_DEBUG		
		sign = RdMicrelSMI(MICREL_CHIPID0);			
		sprintf (buffer, "Sign =: 0x%X\n\0", sign);
		UART_DEBUG_MESSAGE((u8*)buffer);			
		reg2 = RdPHYReg(5,PHYREG_PHYID1); // read PHY id 1
		sprintf (buffer, "R2 =: 0x%X\n\0", reg2);
		UART_DEBUG_MESSAGE((u8*)buffer);			
		
		u8 MicMac[6];
		GetMicrelMacAddr(MicMac);
		sprintf (buffer, "Micrel Mac: 0x%X:0x%X:0x%X:0x%X:0x%X:0x%X:\n\0",
				MicMac[0], MicMac[1], MicMac[2], MicMac[3], MicMac[4], MicMac[5]);
		UART_DEBUG_MESSAGE((u8*)buffer);
		
	#endif						
	}
	if(PhyDevType == PHY_BAD_DEV)	
		UART_DEBUG_MESSAGE("Error PhyDevType\n");
}

#ifdef UART_DEBUG
static void Uart0Init()
{
    volatile int temp = 0;
    int divider = 191;
        
	*pPORTF_FER |= 0x0003;
	*pPORT_MUX &= ~PFDE;                 	
	ssync();			
	*pUART0_GCTL = UCEN;	
	*pUART0_LCR = DLAB;
	*pUART0_DLL = divider;
	*pUART0_DLH = divider>>8;
	*pUART0_LCR = 0x03;

	temp = *pUART0_RBR;
	temp = *pUART0_LSR;
	temp = *pUART0_IIR;
}

static void Wait0RTS()
{
    volatile int temp = *pUART0_LSR;
	while(!(temp & THRE))
	{
		temp = *pUART0_LSR;
	};
}


static void Uart0Send(u8 *data)
{
    while(*data != 0)
    {
        // RTS
        Wait0RTS();						
        // send
        *pUART0_THR = *data++;
    }
}

#endif

//--------------------------------------------------------------------------


//
// Setup PHY interrupt mask
//
static int SetupPhyInterrupts(ADI_ETHER_BF537_DATA *pDev)
{
	//
	// we explicitly disable INT-1 (Auto negotiation page received). if enabled link down interrupt is 
	// not getting raised if etherent cable is unplugged multiple times.
	//
	pDev->PhyInterruptMask &= 0x0fd;
	  
	WrPHYReg(pDev->PhyAddr,PHYREG_INT_MASK,pDev->PhyInterruptMask);
	return(pDev->PhyInterruptMask);
}

//
// Acknowledge PHY interrupt
//
static int AckPhyInterrupts(ADI_ETHER_BF537_DATA *pDev)
{
s32 intStatus;

	/* Reading the source will automatically acknowledge the PHY interrupts */
	intStatus = RdPHYReg(pDev->PhyAddr,PHYREG_INT_SRC_FLG); // read interrupt sources, reset signal to high

	return(intStatus);
}

//
// set up the phy
//
static SetPhy(ADI_ETHER_BF537_DATA *dev)
{
	u16 phydat,phydat1;
	clock_t ndtime;
	clock_t period = ((clock_t)CLOCKS_PER_SEC)/2;

	//		Program PHY registers
	phydat = 0;
	int pha;

	// issue a reset
	RawWrPHYReg(dev->PhyAddr, PHYREG_MODECTL, 0x8000);

	// wait half a second
	period = 30000000; // assume 600 MHZ
	ndtime = clock()+period;
	while (clock()<ndtime);
	phydat = RdPHYReg(dev->PhyAddr, PHYREG_MODECTL);


	// advertise flow control supported
	phydat = RdPHYReg(dev->PhyAddr, PHYREG_ANAR);
	phydat |= (1 << 10);
	WrPHYReg(dev->PhyAddr, PHYREG_ANAR, phydat);


	phydat = 0;
	if (dev->Negotiate) {
		phydat |= 0x1000;		// enable auto negotiation
	} else {
		if (dev->FullDuplex) {
			phydat |= (1 << 8);		// full duplex
		} else {
			phydat &= (~(1 << 8));	// half duplex
		}
		if (!dev->Port10) {
			phydat |= (1 << 13);	// 100 Mbps
		} else {
			phydat &= (~(1 << 13));	// 10 Mbps
		}
	}
	if (dev->Loopback) {
		phydat |= (1 << 14);	// enable TX->RX loopback
		//WrPHYReg(dev->PhyAddr, PHYREG_MODECTL, phydat);
	}
	WrPHYReg(dev->PhyAddr, PHYREG_MODECTL, phydat);
	period = 100000000; // assume 600 MHZ
	ndtime = clock()+period;
	while (clock()<ndtime);
	phydat = RdPHYReg(dev->PhyAddr, PHYREG_MODECTL);

	// check for SMSC PHY
//	if ((RdPHYReg(dev->PhyAddr, PHYREG_PHYID1) == 0x7) && ((RdPHYReg(dev->PhyAddr, PHYREG_PHYID2)&0xfff0 ) == 0xC0A0)) 
	UART_DEBUG_MESSAGE("Set Phy CheckPhyDev Exec\n");
	if(PhyDevType == PHY_LAN83)
	{
		SetupPhyInterrupts(dev);
		// enable EMAC PHY_INT
		u32 sysctl = *pEMAC_SYSCTL;

		sysctl |= 0x1;
		*pEMAC_SYSCTL = sysctl;
	}

}

#if 1
static void DumpQueue(char *str1, char *str2, ADI_ETHER_BUFFER *buf)
{
	BUFFER_INFO *bi = (BUFFER_INFO *)buf;
	DMA_DESCRIPTOR *dmr;

	printf("%s %s\n",str1,str2);

	if (bi != NULL) {
		dmr = bi->First;

		while (dmr != NULL) {
			printf("   NEXT_DESC_PTR:%8.8x\n",dmr->NEXT_DESC_PTR);
			printf("   CONFIG:%4.4x\n",dmr->CONFIG);
			printf("   COUNT:%4.4x\n",dmr->X_COUNT);
			printf("\n");
			dmr = dmr->NEXT_DESC_PTR;
		}
	}
}
static void DumpDesc(char *str, FRAME_QUEUE *q)
{

	DumpQueue(str,"Active",q->Active);
	DumpQueue(str,"Pending",q->Pending);
	DumpQueue(str,"Queued",q->Queued);

}

static int  DumpAllQueues;
#endif

static void GetPhyRegs(u16 addr, u16 *regs)
{
	int pha;

	for (pha = 0; pha < NO_PHY_REGS; pha++) 
	{
		regs[pha] = RdPHYReg(addr, pha);
		
		if(PhyDevType == PHY_MICREL_8895 && pha == 1) regs[pha] |= 0x0024;
		
#ifdef UART_DEBUG
		if(pha == 1)
		{
			sprintf (buffer, "R: 0x%X, D: 0x%X\n\0", pha, regs[pha]);
			UART_DEBUG_MESSAGE((u8*)buffer);
		}
#endif
		
	}
#if 1
	{
		FRAME_QUEUE *q;

		if (DumpAllQueues) {
			DumpDesc("TX",&EtherDev.Tx);
			DumpDesc("RX",&EtherDev.Rx);
		}
	}
#endif
}

/*********************************************************************
*
*	Function:		adi_pdd_Open
*
*	Description:	Opens the BF537 EMAC for use
*
*********************************************************************/


static u32 adi_pdd_Open(				// Open a device
	ADI_DEV_MANAGER_HANDLE	ManagerHandle,	// device manager handle
	u32 					DeviceNumber,	// device number
	ADI_DEV_DEVICE_HANDLE	DeviceHandle,	// device handle
	ADI_DEV_PDD_HANDLE 		*pPDDHandle,	// pointer to PDD handle location
	ADI_DEV_DIRECTION 		Direction,		// data direction
	void					*pEnterCriticalArg,		// enter critical region parameter
	ADI_DMA_MANAGER_HANDLE	DMAHandle,		// handle to the DMA manager
	ADI_DCB_HANDLE			DCBHandle,		// callback handle
	ADI_DCB_CALLBACK_FN		DMCallback		// client callback function
)
{

	u32 			Result;		// return value
	void 			*CriticalResult;
	ADI_ETHER_BF537_DATA *dev= &EtherDev;

		
#ifdef UART_DEBUG
	Uart0Init();
#endif

	UART_DEBUG_MESSAGE("EOpen\n");

	SetPhyDevType();
	u8 phyDevAddress = 1;
	if(PhyDevType == PHY_MICREL_8895)
	{
		phyDevAddress = 5;
		UART_DEBUG_MESSAGE("PhyAddr = 5\n");		
	}
	else UART_DEBUG_MESSAGE("PhyAddr = 1\n");
		
	
	// check for errors if required
#ifdef ADI_ETHER_ERROR_CHECKING_ENABLED
	if (DeviceNumber > 0) {		// check the device number
		return (ADI_DEV_RESULT_BAD_DEVICE_NUMBER);
	}
#endif

	EtherDev.IntMask = 0;	// disable all interrupts
	// insure the device the client wants is available
	Result = ADI_DEV_RESULT_DEVICE_IN_USE;
	CriticalResult = EnterCriticalRegion(pEnterCriticalArg);
	if (!dev->Open) {
		// initialize the device settings
		memset(dev,0,sizeof(ADI_ETHER_BF537_DATA));
		dev->CriticalData = pEnterCriticalArg;
		dev->DeviceHandle = DeviceHandle;
		dev->DCBHandle = DCBHandle;
		dev->DMCallback = DMCallback;
		dev->Direction = Direction;
		dev->Started = false;		
		dev->PhyAddr = phyDevAddress;  			
		dev->CLKIN = 25;  //Ezkit
		dev->FullDuplex=false;
		dev->Negotiate = true;
		dev->FlowControl = false;

		dev->EtherIntIVG = 7;

		dev->RXIVG = 11;
		dev->Rx.Channel = 1;
		dev->Rx.Rcve = true;
		dev->Rx.CompletedStatus = 0x01000;
		dev->Rx.EnableMac = 0x0001;

		dev->TXIVG = 11;
		dev->Tx.Channel = 2;
		dev->Tx.CompletedStatus = 0x0001;
		dev->Tx.EnableMac = 0x010000;

		dev->Trc.BaseEntry = NULL;
		dev->MaxTraceEntries = 0;
		dev->TraceSequence = 0;

		dev->PhyInterruptMask = MII_LAN83C185_ISF_DEFAULT; /* INT-4 and INT-6,7 enabled */
			

		dev->Open = true;
		dev->bEnableGracefulShutdown = false;

		// we set the GPIO pins to Ethernet mode
		SetupPinMux();

		*pEMAC_OPMODE = 0;

		Result = ADI_DEV_RESULT_SUCCESS;

	}
	ExitCriticalRegion(CriticalResult);
	if (Result != ADI_DEV_RESULT_SUCCESS) return (Result);



	// save the physical device handle in the client supplied location
	*pPDDHandle = (ADI_DEV_PDD_HANDLE *)dev;



	// return
	return(ADI_DEV_RESULT_SUCCESS);
}





/*********************************************************************
*
*	Function:		adi_pdd_Close
*
*	Description:	Closes down EMAC
*
*********************************************************************/


static u32 adi_pdd_Close(		// Closes a device
	ADI_DEV_PDD_HANDLE PDDHandle			// PDD handle
)
{
	ADI_ETHER_BF537_DATA *dev = (ADI_ETHER_BF537_DATA *)PDDHandle;
	u32 		Result = ADI_DEV_RESULT_SUCCESS;				// return value
	bool active=true;
	FRAME_QUEUE *q;

	UART_DEBUG_MESSAGE("EClose\n");	
	
	// check for errors if required
#if defined(ADI_ETHER_DEBUG)
	if ((Result = ValidatePDDHandle(PDDHandle)) != ADI_DEV_RESULT_SUCCESS) return (Result);
#endif

	dev->Closing = true;
	dev->Open = false;
	if (dev->Started) {
		dev->Started = false;
		
		// disable all LAN IRQs
		adi_int_SICDisable(ADI_INT_DMA1_ETHERNET_RX_PORTH_A);
		adi_int_SICDisable(ADI_INT_DMA2_ETHERNET_TX_PORTH_B);
		adi_int_SICDisable(ADI_INT_PERIPHERAL_ERROR);
		adi_int_SICDisable(ADI_INT_DMA_ERROR);
		
		if(active)
			active = false;
		
		// unhook interrupts
		adi_int_CECUnhook(dev->RXIVG,RxInterruptHandler,dev);
		adi_int_CECUnhook(dev->TXIVG,TxInterruptHandler,dev);
		adi_int_CECUnhook(dev->EtherIntIVG,EtherInterruptHandler,dev);
		adi_int_CECUnhook(7,DmaErrorInterruptHandler,dev);
	}

	// return
	return(Result);
}


//
//		Queue new frames
//
static void QueueNewFrames(ADI_ETHER_BF537_DATA *dev, FRAME_QUEUE *q,ADI_ETHER_BUFFER *bfs)
{
	ADI_ETHER_BUFFER *lstq,*buf;
	void *CriticalResult;
	int no_frames=0;



	// mark all the buffers as unprocessed
	buf = bfs;
	while (buf != NULL) {
		no_frames++;
		buf->StatusWord = 0;
		buf->ProcessedFlag = 0;
		buf->ProcessedElementCount = 0;
		buf = buf->pNext;
	}

	CriticalResult = EnterCriticalRegion(dev->CriticalData);

	lstq = NULL;
	buf = q->Queued;
	while (buf != NULL) {
		lstq = buf;
		buf = buf->pNext;
	}

	// now append on the Queued queue
	if (lstq == NULL) {
		q->Queued = bfs;
	} else {
		lstq->pNext = bfs;
	}
#ifdef ADI_ETHER_BF537_DEBUG
	bfs->usage = Queued;
	bfs->rxusage = q->Rcve;
#endif
	if (dev->Started) QueueFrames(dev,q);
	q->UnProcessed += no_frames;

	ExitCriticalRegion(CriticalResult);



}




/*********************************************************************
*
*	Function:		adi_pdd_Read
*
*	Description:	Provides buffers to store data when data is received
*					from the EMAC
*
*********************************************************************/


static u32 adi_pdd_Read(			// Reads data or queues an inbound buffer to a device
	ADI_DEV_PDD_HANDLE 	PDDHandle,		// PDD handle
	ADI_DEV_BUFFER_TYPE	BufferType,		// buffer type
	ADI_DEV_BUFFER 		*pBuffer		// pointer to buffer
)
{

	u32 		Result;				// return value
	ADI_ETHER_BF537_DATA *dev = (ADI_ETHER_BF537_DATA *)PDDHandle;

	
//	UART_DEBUG_MESSAGE("ERead\n");	
	/* if the driver has be shut down already, return error immediately */
	if(dev->bEnableGracefulShutdown == TRUE)
	{
		UART_DEBUG_MESSAGE("ERead FAILED\n");			
		return (ADI_DEV_RESULT_FAILED);
	}
	
	// check for errors if required
#if defined(ADI_ETHER_DEBUG)
	if ((Result = ValidatePDDHandle(PDDHandle)) != ADI_DEV_RESULT_SUCCESS) return (Result);
	if (BufferType != ADI_DEV_1D) {
		return (ADI_DEV_RESULT_BUFFER_TYPE_INCOMPATIBLE);
	}
	ADI_DEV_1D_BUFFER *pDebugBuffer = (ADI_DEV_1D_BUFFER *)pBuffer;
	if (((u32)pDebugBuffer->Data & 0x03) || ((u32)((ADI_ETHER_BUFFER *)pBuffer)->PayLoad & 0x03) ) {
		return ADI_ETHER_RESULT_MEMORY_NOT_ALIGNED;
	}
#endif

	QueueNewFrames(dev,&dev->Rx,(ADI_ETHER_BUFFER *)pBuffer);
	
//	UART_DEBUG_MESSAGE("ERead OK\n");			
	return(ADI_DEV_RESULT_SUCCESS);
}





/*********************************************************************
*
*	Function:		adi_pdd_Write
*
*	Description:	Provides buffers containing data to be transmitted
*					out through the EMAC
*
*********************************************************************/


static u32 adi_pdd_Write(		// Writes data or queues an outbound buffer to a device
	ADI_DEV_PDD_HANDLE 	PDDHandle,			// PDD handle
	ADI_DEV_BUFFER_TYPE	BufferType,			// buffer type
	ADI_DEV_BUFFER 		*pBuffer			// pointer to buffer
)

{

	u32 		Result;				// return value
	ADI_ETHER_BF537_DATA *dev = (ADI_ETHER_BF537_DATA *)PDDHandle;
	char *src;
	ADI_ETHER_BUFFER *act = (ADI_ETHER_BUFFER *)pBuffer;

	
	/* if the driver has be shut down already, return error immediately */
	if(dev->bEnableGracefulShutdown == TRUE)
		return (ADI_DEV_RESULT_FAILED);

	// check for errors if required
#if defined(ADI_ETHER_DEBUG)
	if ((Result = ValidatePDDHandle(PDDHandle)) != ADI_DEV_RESULT_SUCCESS) return (Result);
	if (BufferType != ADI_DEV_1D) {
		return (ADI_DEV_RESULT_BUFFER_TYPE_INCOMPATIBLE);
	}
	if (((u32)((ADI_DEV_1D_BUFFER *)pBuffer)->Data & 0x03) || ((u32)((ADI_ETHER_BUFFER *)pBuffer)->PayLoad & 0x03) ) {
		return ADI_ETHER_RESULT_MEMORY_NOT_ALIGNED;
	}
#endif /* ADI_ETHER_DEBUG */


	while (act !=  NULL) {
		// Plant the source MAC address
		src = 8+(char *)(act->Data);
		memcpy(src,dev->Mac,6);
		FlushArea(src,src+6);
		act = act->pNext;
	}


	QueueNewFrames(dev,&dev->Tx,(ADI_ETHER_BUFFER *)pBuffer);

	return(ADI_DEV_RESULT_SUCCESS);
}




//
//		Set MICREL Swich MAC address
//

static void SetMicrelMacAddr(unsigned char *mac)
{		
	WrMicrelSMI(MICREL_MAC3, mac[3]);
	WrMicrelSMI(MICREL_MAC4, mac[4]);	
	WrMicrelSMI(MICREL_MAC5, mac[5]);		
}

//
//		Get MICREL Swich MAC address
//

static void GetMicrelMacAddr(unsigned char *mac)
{

	int i;
	for(i=0; i<6; ++i) mac[i] = RdMicrelSMI(MICREL_MAC0  + i);	
}


//
//		Set MAC address
//

static void SetupMacAddr(unsigned char *mac)
{
	unsigned int lo;
	int i;

	lo = 0;
	for (i=3;i>=0; i--) {
		lo = (lo<<8) | mac[i];
	}
	*pEMAC_ADDRLO = lo;

	lo = 0;
	for (i=5;i>=4; i--) {
		lo = (lo<<8) | mac[i];
	}
	*pEMAC_ADDRHI = lo;
}

//
//		Get MAC address
//

static void GetMacAddr(unsigned char *mac)
{
	unsigned int lo;
	int i;

	lo = *pEMAC_ADDRLO;
	for (i=0;i<4; i++) {
		mac[i] = lo&0xff;
		lo = lo >> 8;
	}

	lo = *pEMAC_ADDRHI;
	for (i=4;i<6; i++) {
		mac[i] = lo&0xff;
		lo = lo >> 8;
	}
}

//
//		Update statistics
//

static void UpdateStatistics(u64 *stats)
{
	volatile unsigned long *cnts = pEMAC_RXC_OK;
	int ncnts = sizeof(ADI_ETHER_STATISTICS_COUNTS)/sizeof(u64);
	int rxcnts = offsetof(ADI_ETHER_STATISTICS_COUNTS,cEMAC_TX_CNT_OK)/sizeof(u64);

	ncnts -= rxcnts;	// no. of TX counts

	while (rxcnts>0) {
		*stats++ += *cnts++;
		rxcnts--;
	}
	cnts = pEMAC_TXC_OK;
	while (ncnts>0) {
		*stats++ += *cnts++;
		ncnts--;
	}

}


//
//		Configure and start the MAC
//
static int StartMac(ADI_ETHER_BF537_DATA *dev)
{
	int res;
	u32 opmode;
	u16 sysctl;
	void *CriticalResult;
	static DMA_DESCRIPTOR txfirst,txlast;
	u16 vrctl;
	unsigned int vco,cmsel,msk;
	u16 fer_val;


	memset(dev->Stats,0,sizeof(ADI_ETHER_STATISTICS_COUNTS));
	UpdateStatistics((u64*)dev->Stats);

	vrctl = *pVR_CTL;
	if ((vrctl&0x4000)==0) {
		*pVR_CTL = 0x4000|vrctl; //enable CLKBFOE to enable clock for the PHY
		msk = cli();
		ssync();
		idle();
		sti(msk);
	}


	//		Set FER regs to MUX in Ethernet pins
	// FER reg bug work-around
	// read it once
	fer_val = *pPORTH_FER;
	// MUX all of these pins to Ethernet
	fer_val = 0xFFFF;

	// write it twice to the same value
	*pPORTH_FER = fer_val;
	*pPORTH_FER = fer_val;



	// enable the interrupts in the SIC
	res       = adi_int_SICSetIVG(ADI_INT_DMA1_ETHERNET_RX_PORTH_A,dev->RXIVG);
	CHECK_RES = adi_int_SICEnable(ADI_INT_DMA1_ETHERNET_RX_PORTH_A);

	CHECK_RES = adi_int_SICSetIVG(ADI_INT_DMA2_ETHERNET_TX_PORTH_B,dev->TXIVG);
	CHECK_RES = adi_int_SICEnable(ADI_INT_DMA2_ETHERNET_TX_PORTH_B);

	CHECK_RES = adi_int_SICSetIVG(ADI_INT_PERIPHERAL_ERROR,dev->EtherIntIVG);
	CHECK_RES = adi_int_SICEnable(ADI_INT_PERIPHERAL_ERROR);

	CHECK_RES = adi_int_SICSetIVG(ADI_INT_DMA_ERROR,7);
	CHECK_RES = adi_int_SICEnable(ADI_INT_DMA_ERROR);

	// hook the DMA error interrupt
	CHECK_RES = adi_int_CECHook(7,DmaErrorInterruptHandler,dev,true);
	// hook the ether interrupt
	CHECK_RES = adi_int_CECHook(dev->EtherIntIVG,EtherInterruptHandler,dev,true);

	if (res == 0) {

		if (dev->Rx.Dma == NULL ) {
			dev->Rx.Dma = (DMA_REGISTERS *)(DMA0_NEXT_DESC_PTR+0x40*dev->Rx.Channel);
			//dev->Rx.Dma->CONFIG = 0;
			dev->Rx.Dma->X_COUNT = 0;
			dev->Rx.Dma->X_MODIFY = 4;
			dev->Rx.Dma->Y_COUNT = 0;
		}

		if (dev->Tx.Dma == NULL) {
			dev->Tx.Dma = (DMA_REGISTERS *)(DMA0_NEXT_DESC_PTR+0x40*dev->Tx.Channel);
			//dev->Tx.Dma->CONFIG = 0;
			dev->Tx.Dma->X_COUNT = 0;
			dev->Tx.Dma->X_MODIFY = 4;
			dev->Tx.Dma->Y_COUNT = 0;
		}
		// set the imask
		dev->IntMask = ~((1<<dev->RXIVG) | (1<<dev->TXIVG) | (1<<dev->EtherIntIVG) );

		// hook the RX,TX complete interrupts
		res = adi_int_CECHook(dev->RXIVG,RxInterruptHandler,dev,true);
		if (res == 0) {
			res = adi_int_CECHook(dev->TXIVG,TxInterruptHandler,dev,true);
			if (res != 0) {
				adi_int_CECUnhook(dev->RXIVG,RxInterruptHandler,NULL);
			}
		}
		if (res != 0)  {
			adi_int_CECUnhook(7,DmaErrorInterruptHandler,NULL);
			adi_int_CECUnhook(dev->EtherIntIVG,EtherInterruptHandler,NULL);
		}

		if (res == 0) {

			u32 fcclk,fsclk,fvco;
			u32 N,mdc;


			vco = ((*pPLL_CTL>>9)&0x3f) * dev->CLKIN;
			if (*pPLL_CTL&1) {
				vco = vco>>1; 	// divide by 2
			}
			cmsel = (*pPLL_DIV>>4)&0x3;
			fcclk = vco/(1<<cmsel);

			cmsel = *pPLL_DIV&0x0f;
			fsclk = vco/cmsel;

			N= (((fsclk+4)/5)-1)&0x3f;

			mdc = (fsclk*5)/(N+1);

			//set up the EMAC controller

			// reset counters, clear on read, saturate , enable counters
			*pEMAC_MMC_CTL = 0x0d;


			// dont enable RX and TX until we have a buffer to start the DMA with
			//Fixted TAR 32762 ethernet driver may drop packets.
			opmode = LCTRE;	// Enable retransmit on late collission
			if (dev->StripPads) {
				opmode |= 0x0002;
			}
			if (dev->FullDuplex) {
				opmode |= 0x04000000;
			}
			opmode |= (1 << 5);
			opmode |= dev->OpMode;
			*pEMAC_OPMODE = opmode;
			

			sysctl = 0;
			if (dev->GenChksums) {
				sysctl |= 0x0004;
			}
			if (dev->NoRcveLnth) {
				sysctl &= (~0x0002);
			} else {
				sysctl |= 0x0002;
			}
			sysctl = (N<<8) | sysctl;	// or in the SCLK:MDC divisor
			
			#ifdef UART_DEBUG		
				sprintf (buffer, "Mdio Div =: 0x%X\n\0", N);
				UART_DEBUG_MESSAGE((u8*)buffer);			
			#endif						
			

			*pEMAC_SYSCTL = sysctl;

			// set up the PHY
			SetPhy(dev);


			// now actually enable the interrupts
			*pEMAC_MMC_RIRQE = 0x00ffffff;		// RX counter interrupts
			*pEMAC_MMC_TIRQE = 0x00ffffff;		// RX counter interrupts

			dev->Started = true;

			CriticalResult = EnterCriticalRegion(dev->CriticalData);
			QueueFrames(dev,&dev->Tx);
			QueueFrames(dev,&dev->Rx);
			ExitCriticalRegion(CriticalResult);
		}
	}

	return res;
}


/************************************************************
	void ShutDownDriver( void )
	Description: Shut down the driver more gracefully than adi_pdd_Close()
				It will wait until all send and recevied queue be processed
				before return
***************************************************************/
				
static u32 ShutDownDriver( 
					ADI_DEV_PDD_HANDLE 	PDDHandle			// PDD handle
 					)
{
	ADI_ETHER_BF537_DATA *dev = (ADI_ETHER_BF537_DATA *)PDDHandle;
	u32 		Result = ADI_DEV_RESULT_SUCCESS;				// return value
	bool active=true;
	FRAME_QUEUE *q;
	u32 uInterruptStatus = cli();

	// check for errors if required
#if defined(ADI_ETHER_DEBUG)
	if ((Result = ValidatePDDHandle(PDDHandle)) != ADI_DEV_RESULT_SUCCESS) return (Result);
#endif

	dev->Closing = true;
	dev->Open = false;
	if (dev->Started) {
		dev->Started = false;
		
		/* set up the flag to block read/write command */
		dev->bEnableGracefulShutdown = true;
		
		// disable all LAN IRQs
		adi_int_SICDisable(ADI_INT_DMA1_ETHERNET_RX_PORTH_A);
		adi_int_SICDisable(ADI_INT_DMA2_ETHERNET_TX_PORTH_B);
		adi_int_SICDisable(ADI_INT_PERIPHERAL_ERROR);
		adi_int_SICDisable(ADI_INT_DMA_ERROR);
		
		if(active)
			active = false;
		
		// wait for the current frames to complete
		while (active) {
			active = false;
			q = &dev->Tx;
			q->Active = 0;
			if ((q->Active!=NULL) || (q->Pending!=NULL) || (q->Completed !=NULL) || (q->Queued!=NULL)) active = true;
			q = &dev->Rx;
			if ((q->Active!=NULL) || (q->Pending!=NULL) || (q->Completed !=NULL) || (q->Queued!=NULL)) active = true;
		}
		// unhook interrupts
		adi_int_CECUnhook(dev->RXIVG,RxInterruptHandler,dev);
		adi_int_CECUnhook(dev->TXIVG,TxInterruptHandler,dev);
		adi_int_CECUnhook(dev->EtherIntIVG,EtherInterruptHandler,dev);
		adi_int_CECUnhook(7,DmaErrorInterruptHandler,dev);
		
	}
	
	memset(&dev->Rx, 0, sizeof(FRAME_QUEUE));
	memset(&dev->Tx, 0, sizeof(FRAME_QUEUE));
	
	*pEMAC_OPMODE = 0;
	sti(uInterruptStatus);
	// return
	return(Result);
	
}


/*********************************************************************
*
*	Function:		GetMultiCastMAC
*
*	Description: 
*                 Creates the multicast mac address given the group ip 
*                 address the incoming pMultiCastMac must be greater than 
*                 or equal to 6 bytes
*
*********************************************************************/
static void GetMultiCastMAC(u32 GroupIpAddress, char *pMultiCastMac)
{
char *p=(char*)&GroupIpAddress;

	// compute the destination multi cast mac address
	//
	memset(pMultiCastMac,0,6);

	// copy the ip address
	memcpy((pMultiCastMac+2),p,4);

	// convert the incoming group ip address to multi cast mac address
	// 0-23 bits are fixed we get the rest from the group ipaddress
	//
	*(pMultiCastMac + 0) = 0x01;
	*(pMultiCastMac + 1) = 0x00;
	*(pMultiCastMac + 2) = 0x5E;
	*(pMultiCastMac + 3) &= 0x7F;

	return;
}

/*********************************************************************
*
*	Function:		GetMultiCastHashBin
*
*	Description:	Returns the hash bin index given the Multicast MAC
*
*********************************************************************/
#define CRC32_POLYNOMIAL 0xEDB88320
static unsigned int GetMultiCastHashBin(char *pMultiCastMac,const int length)
{    
s32  i,j;
bool isXorTrue;
u32  crc32 = ~0L;
s32  binPosition=-1;
u8   data;
   
     for(i=0; i < length; i++)
     {     
     	// get next byte from the data
        data = (u8)pMultiCastMac[i];      
        
        // check all bits in a byte
        for(j=8; j >0;j--)
        {
            isXorTrue = (crc32 ^ data)& 0x1;
            crc32 >>= 1;
               
             if(isXorTrue) 
            	crc32 ^= CRC32_POLYNOMIAL;
            	
            // get the next bit
            data >>=1;
		}           
     }
     
     // swap and get 6 bits
     binPosition = ((crc32&0x01)<<5)| ((crc32&0x02)<<3) |
      			   ((crc32&0x04)<<1)| ((crc32&0x08)>>1) |
            	   ((crc32&0x10)>>3)| ((crc32&0x20)>>5);
     
     return (binPosition);
}

/*********************************************************************
*
*	Function:		AddMultiCastMACFilter
*
*	Description:	Computes the multicast hash bit position and sets
*                   or clears for a given multicast group address.
*
*********************************************************************/

static u32 AddMultiCastMACfilter(u32 GroupIpAddress,bool bAddAddress)
{
char MultiCastMac[6];
char *p = (char*)&GroupIpAddress;
int HashBinIndex;

 	// Get the multicast MAC address for the given address
	GetMultiCastMAC(GroupIpAddress,&MultiCastMac[0]);

	// Get the hash index for the multicast address 
	HashBinIndex = GetMultiCastHashBin((char*)&MultiCastMac[0],6);
	
	if((HashBinIndex == -1) || (HashBinIndex > 63))
	   return ADI_DEV_RESULT_FAILED;

	if(bAddAddress)
	{
		// hash index is in EMAC_HASHHI register
		if(HashBinIndex > 31)
		{
			HashBinIndex -= 32;
			*pEMAC_HASHHI |= (1 << HashBinIndex);
		}
		else
			*pEMAC_HASHLO |= (1 << HashBinIndex);					
	}
	else // remove the address
	{
		/* EMAC_HASHHI bit */
		if(HashBinIndex > 31)
		{
			HashBinIndex -= 32;
			*pEMAC_HASHHI &= ~(1 << HashBinIndex);
		}
		else
			*pEMAC_HASHLO &= ~(1 << HashBinIndex);				
	}

	return(ADI_DEV_RESULT_SUCCESS);
}

/*********************************************************************
*
*	Function:		adi_pdd_Control
*
*	Description:	Configures the EMAC
*
*********************************************************************/


static u32 adi_pdd_Control(		// Sets or senses a device specific parameter
	ADI_DEV_PDD_HANDLE 	PDDHandle,			// PDD handle
	u32 				Command,			// command ID
	void 				*pArg				// pointer to argument
)

{

	u32 		Result;				// return value
	ADI_ETHER_BF537_DATA *dev = (ADI_ETHER_BF537_DATA *)PDDHandle;
	ADI_ETHER_MEM_SIZES *msizes;
	ADI_ETHER_SUPPLY_MEM *memsup;
	int maxbuf,i;
	DMA_DESCRIPTOR *nxt,*lst;
	u32 *prefix;
	ADI_ETHER_BUFFER_COUNTS *bufcnts;
	ADI_ETHER_IVG_MAPPING *ivgs;
	ADI_ETHER_DMA_MAPPING *dmas;
	char *EndXmit;
	ADI_ETHER_BF537_TRACE_INFO *trcinfo;
	char *basemem,*obasemem;
	u32 lnthmem;
	volatile STATUS_AREA *sts,*lsts;

	u32 ii = ADI_DEV_CMD_GET_PERIPHERAL_DMA_SUPPORT;
	ii = ADI_ETHER_CMD_MEM_SIZES;
			
	
	// check for errors if required
#if defined(ADI_ETHER_DEBUG)
	if ((Result = ValidatePDDHandle(PDDHandle)) != ADI_DEV_RESULT_SUCCESS) return (Result);
#endif

	// avoid casts
	Result = ADI_DEV_RESULT_SUCCESS;


	// CASEOF (Command ID)
	switch (Command) {
		case ADI_DEV_CMD_SET_DATAFLOW:
			UART_DEBUG_MESSAGE("CMD_SET_DATAFLOW\n");		
			// enable or disable accordingly
			dev->FlowEnabled = (int)pArg;
			break;

		case ADI_DEV_CMD_SET_DATAFLOW_METHOD:
			break;

		case ADI_DEV_CMD_GET_PERIPHERAL_DMA_SUPPORT:
			// no we dont want peripheral DMA support
			(*(int *)pArg) = false;
			break;

		case ADI_ETHER_CMD_MEM_SIZES:
			UART_DEBUG_MESSAGE("CMD_MEM_SIZES\n");				
			msizes = (ADI_ETHER_MEM_SIZES *)(pArg);
			msizes->BaseMemSize = sizeof(ADI_ETHER_STATISTICS_COUNTS);
			msizes->MemPerRecv = 36+sizeof(DMA_DESCRIPTOR[2]);
			msizes->MemPerXmit = 36+sizeof(DMA_DESCRIPTOR[3]);
			break;

		case ADI_ETHER_CMD_SUPPLY_MEM:
			UART_DEBUG_MESSAGE("CMD_SUPPLY_MEM\n");
			memsup = (ADI_ETHER_SUPPLY_MEM *)(pArg);

			if (memsup->BaseMemLength < sizeof(ADI_ETHER_STATISTICS_COUNTS)) {
				Result = ADI_DEV_RESULT_NO_MEMORY;
			} else {
				dev->Stats = memsup->BaseMem;
				memset(dev->Stats,0,sizeof(ADI_ETHER_STATISTICS_COUNTS));
			}

#if 1
			// layout the available RX desciptors
			lnthmem = memsup->RcveMemLength;
			obasemem = basemem = (char *)memsup->RcveMem;
			// round up base mem to be a multiple of 32
			basemem = (char *)((((u32)basemem)+31)&(~0x1F));
			// adjust the length remaining
			lnthmem = lnthmem - (basemem - obasemem);
			memsup->MaxRcveFrames = maxbuf = lnthmem/(32+sizeof(DMA_DESCRIPTOR[2]));
			if (maxbuf>=1) {
				// layout the status words
				lsts = NULL;
				dev->Rx.AvailStatus = (STATUS_AREA *)basemem;
				for (i=0;i<maxbuf;i++) {
					sts = (STATUS_AREA *)basemem;
					if (lsts!=NULL) lsts->Next = sts;
					sts->Next = NULL;
					lsts = sts;
					basemem += 32;
				}

				dev->Rx.Avail = (DMA_DESCRIPTOR *)basemem;
				dev->Rx.NoAvail = 2*maxbuf;
				nxt = dev->Rx.Avail;
				for (i=0;i<maxbuf;i++) {
					lst = nxt+1;
					memset(nxt,0,sizeof(DMA_DESCRIPTOR));
					nxt[0].NEXT_DESC_PTR = lst;
					lst->NEXT_DESC_PTR = nxt+2;
					nxt += 2;
				}
				lst->NEXT_DESC_PTR = NULL;
			} else {
				memsup->MaxRcveFrames = 0;
				Result = ADI_DEV_RESULT_NO_MEMORY;
			}

			CheckQueues(&dev->Rx);

			// layout the available TX desciptors
			lnthmem = memsup->XmitMemLength;
			obasemem = basemem = (char *)memsup->XmitMem;
			// round up base mem to be a multiple of 32
			basemem = (char *)((((u32)basemem)+31)&(~0x1F));
			// adjust the length remaining
			lnthmem = lnthmem - (basemem - obasemem);
			memsup->MaxXmitFrames = maxbuf = lnthmem/(32+sizeof(DMA_DESCRIPTOR[3]));
			if (maxbuf>=1) {
				// layout the status words
				lsts = NULL;
				dev->Tx.AvailStatus = (STATUS_AREA *)basemem;
				for (i=0;i<maxbuf;i++) {
					sts = (STATUS_AREA *)basemem;
					if (lsts!=NULL) lsts->Next = sts;
					sts->Next = NULL;
					lsts = sts;
					basemem += 32;
				}

				dev->Tx.Avail = (DMA_DESCRIPTOR *)basemem;
				dev->Tx.NoAvail = 3*maxbuf;
				nxt = dev->Tx.Avail;
				for (i=0;i<maxbuf;i++) {
					memset(nxt,0,sizeof(DMA_DESCRIPTOR));
					nxt[0].NEXT_DESC_PTR = nxt+1;
					nxt[1].NEXT_DESC_PTR = nxt+2;
					lst = &nxt[2];
					lst->NEXT_DESC_PTR = nxt+3;
					nxt += 3;
				}
				lst->NEXT_DESC_PTR = NULL;
			} else {
				memsup->MaxXmitFrames = 0;
				Result = ADI_DEV_RESULT_NO_MEMORY;
			}

			CheckQueues(&dev->Tx);
#else
			// layout the available RX desciptors
			dev->Rx.Avail = memsup->RcveMem;
			memsup->MaxRcveFrames = maxbuf = memsup->RcveMemLength/sizeof(DMA_DESCRIPTOR[2]);
			dev->Rx.NoAvail = 2*maxbuf;
			if (maxbuf>=1) {
				nxt = dev->Rx.Avail;
				for (i=0;i<maxbuf;i++) {
					lst = nxt+1;
					memset(nxt,0,sizeof(DMA_DESCRIPTOR));
					nxt[0].NEXT_DESC_PTR = lst;
					lst->NEXT_DESC_PTR = nxt+2;
					nxt += 2;
				}
				lst->NEXT_DESC_PTR = NULL;
			} else {
				memsup->MaxRcveFrames = 0;
				Result = ADI_DEV_RESULT_NO_MEMORY;
			}

			CheckQueues(&dev->Rx);

			// layout the available TX desciptors
			dev->Tx.Avail = memsup->XmitMem;
			EndXmit = (char *)dev->Tx.Avail+memsup->XmitMemLength;
			memsup->MaxXmitFrames = maxbuf = memsup->XmitMemLength/sizeof(DMA_DESCRIPTOR[3]);
			dev->Tx.NoAvail = 3*maxbuf;
			if (maxbuf>=1) {
				nxt = dev->Tx.Avail;
				for (i=0;i<maxbuf;i++) {
					memset(nxt,0,sizeof(DMA_DESCRIPTOR));
					nxt[0].NEXT_DESC_PTR = nxt+1;
					nxt[1].NEXT_DESC_PTR = nxt+2;
					lst = &nxt[2];
					lst->NEXT_DESC_PTR = nxt+3;
					nxt += 3;
				}
				lst->NEXT_DESC_PTR = NULL;
			} else {
				memsup->MaxXmitFrames = 0;
				Result = ADI_DEV_RESULT_NO_MEMORY;
			}

			CheckQueues(&dev->Tx);

#endif



			break;
		case ADI_ETHER_CMD_GET_MAC_ADDR:
			UART_DEBUG_MESSAGE("CMD_GET_MAC_ADDR\n");		
			GetMacAddr((unsigned char *)(pArg));
			break;
		case ADI_ETHER_CMD_SET_MAC_ADDR:
			if (*pEMAC_OPMODE&0x00010001) 
			{
				Result = ADI_DEV_RESULT_INVALID_SEQUENCE;
				UART_DEBUG_MESSAGE("CMD_SET_MAC_ADDR_INVALID\n");
			} else 
			{
//				UART_DEBUG_MESSAGE("CMD_SET_MAC_ADDR_OK\n");				
				memcpy(dev->Mac,pArg,6);
				#ifdef UART_DEBUG
					sprintf (buffer, "Set Mac: 0x%X:0x%X:0x%X:0x%X:0x%X:0x%X:\n\0",
							dev->Mac[0], dev->Mac[1], dev->Mac[2], dev->Mac[3],dev->Mac[4],dev->Mac[5]);
					UART_DEBUG_MESSAGE((u8*)buffer);
				#endif				
				SetupMacAddr((unsigned char *)(pArg));
				SetMicrelMacAddr((unsigned char *)(pArg));
				#ifdef UART_DEBUG
					u8 MicMac[6];
					GetMicrelMacAddr(MicMac);
					sprintf (buffer, "New Micrel Mac: 0x%X:0x%X:0x%X:0x%X:0x%X:0x%X:\n\0",
							MicMac[0], MicMac[1], MicMac[2], MicMac[3], MicMac[4], MicMac[5]);
					UART_DEBUG_MESSAGE((u8*)buffer);
				#endif								
			}
			break;
		case ADI_ETHER_CMD_GET_STATISTICS:
			UART_DEBUG_MESSAGE("CMD_GET_STATISTICS\n");		
			UpdateStatistics((u64*)dev->Stats);
			memcpy((void *)(pArg),dev->Stats,sizeof(ADI_ETHER_STATISTICS_COUNTS));
			break;
		case ADI_ETHER_CMD_GET_BUFFER_PREFIX:
				UART_DEBUG_MESSAGE("CMD_GET_BUFFER_PREFIX\n");
			prefix = (u32 *)(pArg);
			*prefix = 0;
			break;
		case ADI_ETHER_CMD_UNPROCESSED_BUFFER_COUNTS:
			UART_DEBUG_MESSAGE("CMD_UNPROCESSED_BUFFER_COUNTS\n");		
			bufcnts = (ADI_ETHER_BUFFER_COUNTS *)(pArg);
			bufcnts->RcvrBufferCnt = dev->Rx.UnProcessed;
			bufcnts->XmitBufferCnt = dev->Tx.UnProcessed;
			break;
		case ADI_ETHER_CMD_GET_MIN_RECV_BUFSIZE:
			UART_DEBUG_MESSAGE("CMD_GET_MIN_RECV_BUFSIZE\n");		
			prefix = (u32 *)(pArg);
			*prefix = MAX_RCVE_FRAME;
			break;
		case ADI_ETHER_CMD_SET_SPEED:
			i = (int)(pArg);
			if ((i <= 0) || (i > 2)) 
			{
				UART_DEBUG_MESSAGE("CMD_SET_SPEED_NOT_SUPPORTED\n");
				Result = ADI_DEV_RESULT_NOT_SUPPORTED;
			} 
			else 
			{
				UART_DEBUG_MESSAGE("CMD_SET_SPEED_OK\n");				
				dev->Port10 = (i == 1);
				dev->Negotiate = false;
			}
			break;
		case ADI_ETHER_CMD_SET_FULL_DUPLEX:
			UART_DEBUG_MESSAGE("CMD_SET_FULL_DUPLEX\n");		
			i = (int)(pArg);
			dev->FullDuplex = (i!=0);
			dev->Negotiate = false;
			break;
		case ADI_ETHER_CMD_SET_NEGOTIATE:
			UART_DEBUG_MESSAGE("CMD_SET_NEGOTIATE\n");		
			i = (int)(pArg);
			dev->Negotiate = (i!=0);
			break;
		case ADI_ETHER_CMD_START:
			UART_DEBUG_MESSAGE("CMD_START\n");		
			Result = StartMac(dev);
			break;
		case ADI_ETHER_CMD_GET_PHY_REGS:
			UART_DEBUG_MESSAGE("CMD_GET_PHY_REGS\n");		
			GetPhyRegs(dev->PhyAddr,(void *)(pArg));
			break;
		case ADI_ETHER_CMD_SET_LOOPBACK:
			UART_DEBUG_MESSAGE("CMD_SET_LOOPBACK\n");		
			i = (int)(pArg);
			dev->Loopback = (i!=0);
			if (dev->Started) {
				// change the phy
				u16 cur;

				cur = RdPHYReg(dev->PhyAddr, PHYREG_MODECTL);
				if (dev->Loopback) {
					cur |= (1 << 14);	// enable TX->RX loopback
				} else {
					cur &= (~(1 << 14));
				}
				WrPHYReg(dev->PhyAddr, PHYREG_MODECTL,cur);
			}
			break;
		case ADI_ETHER_CMD_BUFFERS_IN_CACHE:
			UART_DEBUG_MESSAGE("CMD_BUFFERS_IN_CACHE\n");		
			i = (int)(pArg);
			dev->Cache = (i!=0);
			break;

		case ADI_ETHER_CMD_BF537_NO_RCVE_LNTH:
			UART_DEBUG_MESSAGE("CMD_BF537_NO_RCVE_LNTH\n");		
			i = (int)(pArg);
			dev->NoRcveLnth = (i!=0);
			break;
		case ADI_ETHER_CMD_BF537_STRIP_PAD:
			UART_DEBUG_MESSAGE("CMD_BF537_STRIP_PAD\n");		
			i = (int)(pArg);
			dev->StripPads = (i!=0);
			break;
		case ADI_ETHER_CMD_BF537_CLKIN:
			UART_DEBUG_MESSAGE("CMD_BF537_CLKIN\n");		
			i = (int)(pArg);
			dev->CLKIN = i;
			break;
		case ADI_ETHER_CMD_BF537_USE_IVG:
			UART_DEBUG_MESSAGE("CMD_BF537_USE_IVG\n");		
			ivgs = (ADI_ETHER_IVG_MAPPING *)(pArg);
			dev->EtherIntIVG = ivgs->ErrIVG;
			dev->RXIVG = ivgs->RxIVG;
			dev->TXIVG = ivgs->TxIVG;
			break;
		case ADI_ETHER_CMD_BF537_USE_DMA:
			UART_DEBUG_MESSAGE("CMD_BF537_USE_DMA\n");		
			dmas = (ADI_ETHER_DMA_MAPPING *)(pArg);
			dev->Rx.Channel = dmas->RxChannel;
			dev->Tx.Channel = dmas->TxChannel;
			break;
		case ADI_ETHER_CMD_BF537_SET_PHY_ADDR:
			UART_DEBUG_MESSAGE("CMD_BF537_SET_PHY_ADDR\n");		
			i = (int)(pArg);
			dev->PhyAddr = i &0xffff;
			break;
		case ADI_ETHER_CMD_GEN_CHKSUMS:
			UART_DEBUG_MESSAGE("CMD_GEN_CHKSUMS\n");		
			dev->GenChksums = true;
			break;

		case ADI_ETHER_CMD_BF537_SET_PHY_INT_MASK:
			 UART_DEBUG_MESSAGE("CMD_BF537_SET_PHY_INT_MASK\n");
			 dev->PhyInterruptMask = (int)pArg;
			 break;

		case ADI_ETHER_CMD_BF537_GET_PHY_INT_MASK:
			 UART_DEBUG_MESSAGE("CMD_BF537_GET_PHY_INT_MASK\n");				
			 (*(int *)pArg) = dev->PhyInterruptMask;
			 break;

		case ADI_ETHER_CMD_BF537_SET_TRACE:
			UART_DEBUG_MESSAGE("CMD_BF537_SET_TRACE\n");				
			trcinfo = (ADI_ETHER_BF537_TRACE_INFO *)(pArg);
			dev->Trc.BaseEntry = (ADI_ETHER_BF537_TRACE_ENTRY *)trcinfo->Mem;
			dev->Trc.EntryLnth = (sizeof(ADI_ETHER_BF537_TRACE_ENTRY)+trcinfo->MaxBytes+3)&(~0x3); // round up to multiple of 4
			dev->Trc.NoOfEntries = trcinfo->LnthMem/dev->Trc.EntryLnth;
			if (dev->Trc.NoOfEntries <=0) {
				dev->Trc.BaseEntry = NULL;
				dev->MaxTraceEntries = 0;
				Result = ADI_DEV_RESULT_NO_MEMORY;
			} else {
				dev->Trc.EndOfData = (ADI_ETHER_BF537_TRACE_ENTRY *)(((char *)dev->Trc.BaseEntry) + dev->Trc.EntryLnth*dev->Trc.NoOfEntries);
				dev->MaxTraceEntries = dev->Trc.NoOfEntries;
				dev->Trc.OldestEntry = dev->Trc.BaseEntry;
				dev->Trc.NoOfEntries = 0;
				dev->TraceMaxBytes = dev->Trc.EntryLnth - sizeof(ADI_ETHER_BF537_TRACE_ENTRY);
				dev->TraceFirstByte = trcinfo->FirstByte;
			}

			break;
		case ADI_ETHER_CMD_BF537_GET_TRACE:
			UART_DEBUG_MESSAGE("CMD_BF537_GET_TRACE\n");				
			memcpy(pArg,&dev->Trc,sizeof(ADI_ETHER_BF537_TRACE_DATA));
			break;

		case ADI_ETHER_CMD_SHUTDOWN_DRIVER:
			UART_DEBUG_MESSAGE("CMD_SHUTDOWN_DRIVER\n");				
			ShutDownDriver(PDDHandle);
			break;


		case ADI_DEV_CMD_TABLE:
			break;


		case ADI_ETHER_ADD_MULTICAST_MAC_FILTER:
			{
			  UART_DEBUG_MESSAGE("ADD_MULTICAST_MAC_FILTER\n");						
			  u32 MultiCastGroupAddr = (u32)pArg;

			  AddMultiCastMACfilter(MultiCastGroupAddr,true);
			}
			break;

		case ADI_ETHER_DEL_MULTICAST_MAC_FILTER:
			{
			  UART_DEBUG_MESSAGE("DEL_MULTICAST_MAC_FILTER\n");										
			  u32 MultiCastGroupAddr = (u32)pArg;

			  AddMultiCastMACfilter(MultiCastGroupAddr,false);
			}
			break;

		default:

			UART_DEBUG_MESSAGE("CMD_NOT_SUPPORTED\n");				
			// we don't understand this command
			Result = ADI_DEV_RESULT_NOT_SUPPORTED;

	}

	// return
	return(Result);
}


/*********************************************************************
*
*	Function:		InterruptHandler
*
*	Description:	Processes events in response to EMAC interrupts
*
*********************************************************************/
static void DmaError(FRAME_QUEUE *q)
{
	int k=10;

	k=q->Channel;
	k = q->Dma->IRQ_STATUS;
}
static ADI_INT_HANDLER(DmaErrorInterruptHandler)
{
	ADI_INT_HANDLER_RESULT result = ADI_INT_RESULT_NOT_PROCESSED;
	ADI_ETHER_BF537_DATA *dev = (ADI_ETHER_BF537_DATA *)ClientArg;
	BUFFER_INFO *bi;
	int addr;

	breakpoint(2);
	// ensure a 32 bit access to the MMRs is issued before the 16 bit access in case
	// the interrupt killed a 32 bit access - anomaly ED(03-00-0048)
	addr = dev->Tx.Dma->START_ADDR;
	// check our two DMA channels
	if (dev->Tx.Dma->IRQ_STATUS & 0x02) {
		// error on TX channel
		bi = (BUFFER_INFO *)dev->Tx.Active;
		DmaError(&dev->Tx);
		result = ADI_INT_RESULT_PROCESSED;
		// acknowedge the interrupt
		dev->Tx.Dma->IRQ_STATUS = 0x02;
	}
	if (dev->Rx.Dma->IRQ_STATUS & 0x02) {
		// error on RX channel
		bi = (BUFFER_INFO *)dev->Rx.Active;
		DmaError(&dev->Rx);
		result = ADI_INT_RESULT_PROCESSED;
		// acknowedge the interrupt
		dev->Rx.Dma->IRQ_STATUS = 0x02;
	}
	return 	result;

}

//
//  Process PHY interrupts, More than one PHY interrupt can be asserted.   
//  Currently entire status is passed to the callback handler for it to take
//  action. driver itself do not perform any actions. upper layers(stack) or 
//  application has to issue approprirate i/o controls to handle the events.
//
static int ProcessPhyInterrupts(ADI_ETHER_BF537_DATA *pDev,u32 uPhyInterruptStatus)
{
ADI_LAN983C185_PHY_INT_COUNTERS *pPhyInterruptCounters = &pDev->PhyInterruptCounters;

	UART_DEBUG_MESSAGE("ProcessPhyInterrupts\n");

	if(uPhyInterruptStatus & MII_LAN83C185_ISF_INT1)
		pPhyInterruptCounters->cPhyIntAutoNegotiationPageRcvd++;
	if(uPhyInterruptStatus & MII_LAN83C185_ISF_INT2)
		pPhyInterruptCounters->cPhyIntParallelDetectionFault++;
	if(uPhyInterruptStatus & MII_LAN83C185_ISF_INT3)
		pPhyInterruptCounters->cPhyIntAutoNegotiationLPAck++;
	if(uPhyInterruptStatus & MII_LAN83C185_ISF_INT4)
		pPhyInterruptCounters->cPhyIntLinkDown++;
	if(uPhyInterruptStatus & MII_LAN83C185_ISF_INT5)
		pPhyInterruptCounters->cPhyIntRemoteFaultDetected++;
	if(uPhyInterruptStatus & MII_LAN83C185_ISF_INT6)
		pPhyInterruptCounters->cPhyIntAutoNegotiationComplete++;
	if(uPhyInterruptStatus & MII_LAN83C185_ISF_INT7)
		pPhyInterruptCounters->cPhyIntEnergyOn++;

  return(1);
}

//
//		EMAC ethernet event interrupt handler
//

static  ADI_INT_HANDLER(EtherInterruptHandler)			// EMAC ethernet event interrupt handler
{
	ADI_INT_HANDLER_RESULT result = ADI_INT_RESULT_NOT_PROCESSED;
	ADI_ETHER_BF537_DATA *dev = (ADI_ETHER_BF537_DATA *)ClientArg;
	u32 systat,mask, rirqs,tirqs,event;

	event = ADI_ETHER_EVENT_INTERRUPT;
	// Ethernet event interrupt
	systat = *pEMAC_SYSTAT;
	breakpoint(2);
	// ack W1C bits
	mask = systat&0xe1;
	if (mask) *pEMAC_SYSTAT = mask;
	// now to process individual bits
	if (systat&0x02) {
		// MMC counter interrupt
		rirqs = *pEMAC_MMC_RIRQS;
		tirqs = *pEMAC_MMC_TIRQS;
		UpdateStatistics((u64*)dev->Stats);
		// clear the interrupt
		*pEMAC_MMC_RIRQS = rirqs;
		*pEMAC_MMC_TIRQS = tirqs;
		result = ADI_INT_RESULT_PROCESSED;
	}
	if (systat&0x01) {
		//PHY_INT
		int full=0;
		u32 opmode;
		u16 reg3,reg2,reg,phydat;

//		reg2 = RdPHYReg(dev->PhyAddr,PHYREG_PHYID1); // read PHY id 1
//		reg3 = RdPHYReg(dev->PhyAddr,PHYREG_PHYID2); // read PHY id 2
//		if ((reg2 == 0x07) && ((reg3>>4) == 0xc0a)) 
		UART_DEBUG_MESSAGE("INT_HANDLER CheckPhyDev Exec\n");
		if(PhyDevType)
		{					
			reg = RdPHYReg(dev->PhyAddr,PHYREG_SPEED_IND); // read special status			
			if(PhyDevType == PHY_MICREL_8895) full = (reg & 0x400);
			if(PhyDevType == PHY_LAN83) full = (reg & 0x10);
			
			if (full) {
				// does remote link support flow control
				phydat = RdPHYReg(dev->PhyAddr,PHYREG_ANLPAR);
				dev->FlowControl = (phydat &0x0400);
				if (dev->FlowControl) {
					// we enable flow control
					*pEMAC_FLC = FLCE;		/* flow control enabled */
				// advertize flow control supported
				}
			}
		}
		opmode = *pEMAC_OPMODE;
		if (full) {
			opmode |= 0x04000000;
		} else {
			opmode &= 0xfbffffff;
		}
		*pEMAC_OPMODE = opmode;

		systat = AckPhyInterrupts(dev);

		ProcessPhyInterrupts(dev,systat);

		event = ADI_ETHER_EVENT_INTERRUPT_PHY;
		result = ADI_INT_RESULT_PROCESSED;

	}
	//invoke the callback function

	if (dev->DCBHandle) {
#ifdef ADI_ETHER_BF537_DEBUG
		ADI_ETHER_BF537_OutstandingPosts++;
#endif
		adi_dcb_Post(dev->DCBHandle,0,dev->DMCallback, dev->DeviceHandle, event, (void *)systat);
	} else {
		(dev->DMCallback)(dev->DeviceHandle, event,(void *)systat);
	}

	return result;
}

//
//		Append Pending queue to the active queue
//
static AppendPending(FRAME_QUEUE *q, ADI_ETHER_BUFFER *lstact)
{
	BUFFER_INFO *bi,*bipend;


	CheckQueues(q);

	{
		ADI_ETHER_BUFFER *pnd = q->Active;
		while (pnd && pnd->pNext) {
			pnd = pnd->pNext;
		}
		if (lstact != pnd) {
			int k=4;
		}
	}

	// update the Active and Pending queues
	if (lstact == NULL) {
		if (q->Active != NULL) {
			int k=4;
		}
		CheckQueues(q);
		q->Active = q->Pending;
	} else {
		CheckQueues(q);
		lstact->pNext = q->Pending;
		// chain on the deswcriptors
		bi = (BUFFER_INFO*)lstact;
		bipend = (BUFFER_INFO*)q->Pending;
		bi->Last->NEXT_DESC_PTR = bipend->First;
		// update the config in the last desc
		if ((q->Rcve==0) && (q->Pending->PayLoad!=NULL)) {
			bi->Last->CONFIG &= 0xf0ff;		// remove the current ndsize
			bi->Last->CONFIG |= 0x7600;		// or in  flow as 7 and ndsize as 6
		} else {
			// the previous value will have been 8b
#ifdef 	USE_SYNC
			bi->Last->CONFIG |= 0x7520;		// or in  flow as 7 and ndsize as 5 as SYNC
#else
			bi->Last->CONFIG |= 0x7500;		// or in  flow as 7 and ndsize as 5
#endif
		}
		FlushArea(bi->Last, ((char *)bi->Last)+sizeof(DMA_DESCRIPTOR));
	}
#ifdef ADI_ETHER_BF537_DEBUG
	{
		ADI_ETHER_BUFFER *pnd = q->Pending;
		while (pnd) {
			pnd->usage = Active;
			pnd = pnd->pNext;
		}
	}
#endif
	q->Pending = NULL;

	CheckQueues(q);
}
//
//		Queue Frames
//		this function assumes that it is running within a critical region
//


static void QueueFrames(ADI_ETHER_BF537_DATA *dev, FRAME_QUEUE *q)
{
	ADI_ETHER_BUFFER *lstpnd = q->Pending, *buf,*lstact,*nonact;
	int no_needed;
	BUFFER_INFO *bi,*bipend;
	DMA_DESCRIPTOR *dmr,*lstdmr,*remdmr;
	bool append=false;
	int NoActive=0;
	int i;
	ADI_ETHER_BUFFER  *iQ = q->Queued, *iQN=NULL, *iP= q->Pending,*iA = q->Active;
	int iNoa = q->NoAvail;
	int ists;
	STATUS_AREA *sts;


	CheckQueues(q);


	// find the end of the pendinq queue
	if (lstpnd) {
		while (lstpnd->pNext != NULL) lstpnd = lstpnd->pNext;
	}

	// step through the queued buffers allocating descriptors and appending them to the pending queue
	buf = q->Queued;
	while (buf!=NULL) {
		no_needed = 2;
		if ((q->Rcve == 0) && (buf->PayLoad != NULL)) {
			no_needed = 3;
		}
		if ((no_needed > q->NoAvail) || (q->AvailStatus==NULL)) {
			break;
		}

		if (((unsigned int)q->Avail)&3) {
			int k=1;//##avail
		}

		dmr = q->Avail;

		// update the CONFIG for the last descriptor to make it flow
		if (lstpnd!=NULL) {
			bi = (BUFFER_INFO *)lstpnd;
			bi->Last->NEXT_DESC_PTR = dmr;
			if (no_needed == 3) {
				bi->Last->CONFIG &= 0xf0ff;		// remove the current ndsize
				bi->Last->CONFIG |= 0x7600;		// or in  flow as 7 and ndsize as 6
			} else {
				// the previous value will have been 8b
#ifdef 	USE_SYNC
				bi->Last->CONFIG |= 0x7520;		// or in  flow as 7 and ndsize as 5 as SYNC
#else
				bi->Last->CONFIG |= 0x7500;		// or in  flow as 7 and ndsize as 5
#endif
			}
			FlushArea(bi->Last, ((char *)bi->Last)+sizeof(DMA_DESCRIPTOR));
		}

		// layout the descriptors for the buffer
		bi = (BUFFER_INFO *)buf;
		bi->NoDesc = no_needed;

		// set up the status word area
		bi->Status = sts = q->AvailStatus;
		*((long long *)sts) = 0;
		q->AvailStatus = sts->Next;

		bi->First = dmr;
		// dmr NEXT_DESC_PTR is already set
		dmr->START_ADDR = (unsigned long)buf->Data;
		dmr->X_COUNT = (no_needed==3?(buf->ElementCount*buf->ElementWidth+3)>>2:0);
		if (q->Rcve == 0) {
			// we need to set the length half word
			unsigned short *lnth = (unsigned short *)buf->Data;
			buf->ProcessedElementCount = (*lnth +2+buf->ElementWidth-1)/buf->ElementWidth;
			if (no_needed == 3) {
				// we need to get X_COUNT set to zero by next descriptor
#ifdef 	USE_SYNC
				dmr->CONFIG = 0x7629;		// flow=7,ndsize=6,wdsize=4,enable, mem read, use sync
#else
				dmr->CONFIG = 0x7609;		// flow=7,ndsize=6,wdsize=4,enable, mem read
#endif
			} else {
#ifdef 	USE_SYNC
				dmr->CONFIG = 0x7529;		// flow=7,ndsize=5,wdsize=4,enable, mem read, use sync
#else
				dmr->CONFIG = 0x7509;		// flow=7,ndsize=5,wdsize=4,enable, mem read
#endif
			}

		} else {
#ifdef 	USE_SYNC
			dmr->CONFIG = 0x752b;		// flow=7,ndsize=5,wdsize=4,enable, mem write, use sync
#else
			dmr->CONFIG = 0x750b;		// flow=7,ndsize=5,wdsize=4,enable, mem write
#endif
		}
		FlushArea(dmr,((char *)dmr)+sizeof(DMA_DESCRIPTOR));
		dmr = dmr->NEXT_DESC_PTR;
		if (no_needed == 3)	{
			// set up payload descriptor
			dmr->START_ADDR = (unsigned long)buf->PayLoad;
#ifdef USE_SYNC
			dmr->CONFIG = 0x7529;		// flow=7,ndsize=5,wdsize=4,enable, mem read, use sync
#else
			dmr->CONFIG = 0x7509;		// flow=7,ndsize=5,wdsize=4,enable, mem read
#endif
			dmr->X_COUNT = 0;
			FlushArea(dmr,((char *)dmr)+sizeof(DMA_DESCRIPTOR));
			dmr = dmr->NEXT_DESC_PTR;
		}
		if (dev->Cache) {
			char *data = (char *)buf->Data;

			if (q->Rcve==0) {
				// we need to flush the buffer
				unsigned short lnth = *((unsigned short *)data);
				if (no_needed == 3) {
					// two data buffers
					int no_bytes = buf->ElementCount*buf->ElementWidth;
					FlushArea(data,data+no_bytes);
					FlushArea(buf->PayLoad,((char *)buf->PayLoad)+lnth-no_bytes);
				} else {
					FlushArea(data,data+2+lnth);
				}
			} else {
				// we need to invalidate the cache lines
				FlushInvArea(data,data+MAX_RCVE_FRAME);
			}
		}
		// now set the descriptor for the status word
#if 1
		dmr->START_ADDR = ((q->Rcve!=0) && (dev->GenChksums!=0)?(unsigned long)&sts->IPHdrChksum:(unsigned long)&sts->StatusWord);
#else
		dmr->START_ADDR = ((q->Rcve!=0) && (dev->GenChksums!=0)?(unsigned long)&buf->IPHdrChksum:(unsigned long)&buf->StatusWord);
#endif
		// we also need to invalidate the status word space
		SIMPLEFLUSHINV(sts);	// flush and invalidate the status word
		// status word is always written
#ifdef USE_SYNC
		dmr->CONFIG = 0x00AB;		// flow=0,ndsize=0,wdsize=4,int enable, ensable, mem write, use sync
#else
		dmr->CONFIG = 0x008B;		// flow=0,ndsize=0,wdsize=4,int enable, ensable, mem write
#endif
		bi->Last = dmr;
		remdmr = dmr->NEXT_DESC_PTR;
		// terminate the list of dmr's
		dmr->NEXT_DESC_PTR = 0;
		FlushArea(dmr,((char *)dmr)+sizeof(DMA_DESCRIPTOR));

		if (lstpnd == NULL) {
			// form the pending queue
			q->Pending = buf;
		} else {
			lstpnd->pNext = buf;
		}
		// update last pending buffer
		lstpnd = buf;
#ifdef ADI_ETHER_BF537_DEBUG
		buf->usage = Pending;
#endif
		// step buf onto the next buffer if any in the queued queue
		buf = buf->pNext;
		// now terminate the pending queue
		lstpnd->pNext = NULL;

		q->NoAvail -= no_needed;
		q->Avail = remdmr;
		if (((unsigned int)q->Avail)&3) {
			int k=1; //##avail
		}

		//## this only needed to allow CheckQueues to be invoked
		q->Queued = buf;	// remove frames from Queued list
		CheckQueues(q);
		//##

	}

	q->Queued = buf;	// remove frames from Queued list

	CheckQueues(q);

	// check to see if there is anything in the pending queue
	if (q->Pending) {

		append = false;

		// check to see if we have at least two uncompleted entries in active queue
		NoActive = 0;
		lstact = NULL;
		buf = q->Active;
		lstact = buf;
		while (buf != NULL) {
			lstact = buf;
			bi = (BUFFER_INFO *)buf->Reserved;
			sts = bi->Status;
			SIMPLEFLUSHINV(sts);	// flush and invalidate the status word

			if ((sts->StatusWord&q->CompletedStatus)==0) {
				NoActive++;
			}
			buf = buf->pNext;
		}
		CheckQueues(q);
		if ((NoActive>=2) || (q->EnableMac!=0))	 {
			// if two non completed frames or DMA not yet started
			CheckQueues(q);
			AppendPending(q,lstact);
			CheckQueues(q);
		}
		CheckQueues(q);
		// check to see if the DMA is running
		if (q->Dma == NULL) {
			q->Dma = (DMA_REGISTERS *)(DMA0_NEXT_DESC_PTR+0x40*q->Channel);
			//q->Dma->CONFIG = 0;
			q->Dma->X_COUNT = 0;
			q->Dma->X_MODIFY = 4;


		}

		iP = q->Pending; iA = q->Active;
		ists = (q->Dma != NULL?q->Dma->IRQ_STATUS: -1);

		if ((q->Dma != NULL) && ((q->Dma->IRQ_STATUS &0x8) == 0)) {
			// DMA assigned but not running
			// anything in the pending queue
			if (q->Pending) {
				AppendPending(q,lstact);
			}
			// start the DMA if anything in the active queue
			nonact = q->Active;
			while (nonact!=NULL) {
				bi = (BUFFER_INFO *)nonact->Reserved;
				sts = bi->Status;
				SIMPLEFLUSHINV(sts);	// flush and invalidate the status word

				if ((sts->StatusWord&q->CompletedStatus)!=0) {
					nonact = nonact->pNext;
				} else {
					break;
				}
			}
			if (nonact) {
				if (q->Rcve==0) {
					NoTxStarts++;
				} else {
					NoRxStarts++;
				}
				// we need to trigger the DMA
				bi = (BUFFER_INFO *)nonact;
				q->Enabled = true;
				q->Dma->NEXT_DESC_PTR = bi->First;
				q->Dma->CONFIG = bi->First->CONFIG;

				// now the DMA is running, we can enable the MAC
				if (q->EnableMac!=0) {
					u32 opmode = *pEMAC_OPMODE|q->EnableMac;
					*pEMAC_OPMODE = opmode;
					q->EnableMac = 0;
				}
			}
		} else {
			if (q->Rcve==0) {
				int sts = q->Dma->IRQ_STATUS;
				int k=2;
			}

		}
	}

	CheckQueues(q);

}

//
//		Process DMA completion
//

static ADI_INT_HANDLER_RESULT ProcessCompletion(ADI_ETHER_BF537_DATA *dev, FRAME_QUEUE *q)
{
	ADI_INT_HANDLER_RESULT result = ADI_INT_RESULT_NOT_PROCESSED;
	ADI_ETHER_BUFFER *act,*lst,*fst;
	BUFFER_INFO *bi;
	int noposted=0;
	STATUS_AREA *sts;


	void *xit = EnterCriticalRegion(NULL);
	breakpoint(2);

	CheckQueues(q);

	if (q->Dma->IRQ_STATUS&0x01) {
		// interrupt asserted
		result = ADI_INT_RESULT_PROCESSED;
		// acknowedge the interrupt
		q->Dma->IRQ_STATUS = 0x01;
		if (q->Rcve) {
			NoRxInts++;
		} else {
			NoTxInts++;
}

	}
	// check to see if any frames have completed
	fst = act = q->Active;
	lst = NULL;
	while (act!=NULL) {
		bi = (BUFFER_INFO *)act->Reserved;
		sts = bi->Status;
		SIMPLEFLUSHINV(sts);	// flush and invalidate the status word

		if ((sts->StatusWord&q->CompletedStatus)==0) break;
		noposted++;

		*((long long *)&act->IPHdrChksum) = *((long long *)sts);
		// we can add the used descriptors to the Avail queue
		q->NoAvail += bi->NoDesc;
		bi->Last->NEXT_DESC_PTR = q->Avail;
		q->Avail = bi->First;
		if (((unsigned int)q->Avail)&3) {
			int k=1;//##avail
		}
		// return the status area
		sts->Next = q->AvailStatus;
		q->AvailStatus = sts;

#ifdef ADI_ETHER_BF537_DEBUG
		act->usage = Completed;
#endif
		act->ProcessedFlag = TRUE;
		// we need to set the processed element count for a received frame
		// in the case of a transmit it is set up before transmission
		if (q->Rcve) {
			int nobytes = act->StatusWord&0x7ff;
			if (!dev->NoRcveLnth) {
				ADI_ETHER_FRAME_BUFFER *frm = act->Data;
				frm->NoBytes = nobytes;
				act->ProcessedElementCount = (nobytes+2+3)/act->ElementWidth;
			} else {
				act->ProcessedElementCount = (nobytes+3)/act->ElementWidth;
			}
		}

		if (dev->MaxTraceEntries>0) {
			// we need to trace the frame if it is OK
			ADI_ETHER_BF537_TRACE_ENTRY *te = dev->Trc.OldestEntry;

			if (q->Rcve) {
				// received
				if ((act->StatusWord&0x3000) == 0x3000) {
					// valid frame received
					int nobytes = act->StatusWord&0x7ff-dev->TraceFirstByte;
					if (nobytes>0) {
						if (nobytes>dev->TraceMaxBytes) nobytes = dev->TraceMaxBytes;
						te->NoBytes = nobytes;
						te->Dirn = 'R';
						te->Seqn = (u8)(dev->TraceSequence++);
						memcpy(te->Data,((char *)act->Data)+(dev->NoRcveLnth?0:2)+dev->TraceFirstByte,nobytes);
						te = (ADI_ETHER_BF537_TRACE_ENTRY *)(((char *)te) + dev->Trc.EntryLnth);
						if (dev->Trc.NoOfEntries<dev->MaxTraceEntries) {
							dev->Trc.NoOfEntries++;
						} else {
							if (te>=dev->Trc.EndOfData) {
								te = dev->Trc.BaseEntry;
							}
						}
						dev->Trc.OldestEntry = te;
					}
				}
			} else {
				// transmitted
				if ((act->StatusWord&0x3) == 0x3) {
					// valid frame trasmitted
					int nobytes = (act->StatusWord>>16)&0x7ff-dev->TraceFirstByte;
					if (nobytes>0) {
						if (nobytes>dev->TraceMaxBytes) nobytes = dev->TraceMaxBytes;
						te->NoBytes = nobytes;
						te->Dirn = 'T';
						te->Seqn = (u8)(dev->TraceSequence++);
						if (act->PayLoad!=NULL) {
							int first = act->ElementCount*act->ElementWidth;
							int pa = first - 2 - dev->TraceFirstByte;

							if (nobytes<= pa) {
								memcpy(te->Data,((char *)act->Data)+2+dev->TraceFirstByte,nobytes);
							} else {
								u8 *nxt = te->Data;
								if (pa>0) {
									memcpy(nxt,((char *)act->Data)+2+dev->TraceFirstByte,pa);
									nobytes -= pa;
									nxt += pa;
								}
								memcpy(nxt,((u8 *)act->PayLoad)-pa,nobytes);
							}
						} else {
							memcpy(te->Data,((char *)act->Data)+2+dev->TraceFirstByte,nobytes);
						}
						te = (ADI_ETHER_BF537_TRACE_ENTRY *)(((char *)te) + dev->Trc.EntryLnth);
						if (dev->Trc.NoOfEntries<dev->MaxTraceEntries) {
							dev->Trc.NoOfEntries++;
						} else {
							if (te>=dev->Trc.EndOfData) {
								te = dev->Trc.BaseEntry;
							}
						}
						dev->Trc.OldestEntry = te;
					}
				} else {
					TxErr++;
				}

			}
		}
		// step onto the next active element
		q->NoCompletions++;
		lst = act;
		act = act->pNext;
	}

	// fst points to first active buffer
	// lst if non NULL points to last completed buffer
	// act points to the new head of the Active list
	if (lst!=NULL) {
		u32 event = (q->Rcve?ADI_ETHER_EVENT_FRAME_RCVD:ADI_ETHER_EVENT_FRAME_XMIT);

		// at least one buffer has completed
		q->UnProcessed -= noposted;
		// detach the completed frames
		q->Active = act;

		// append the completed buffers to the Completed queue
		lst->pNext = NULL;
		if (q->Completed!=NULL) {
			lst = q->Completed;
			while (lst->pNext != NULL) {
				noposted++;
				lst = lst->pNext;
			}
			lst->pNext = fst;
		} else {
			q->Completed = fst;
		}

		// can we add any queued frames to the pending list and then to the active list
		QueueFrames(dev,q);
		if (q->Rcve && (q->Active == NULL) && (dev->FlowControl)) {
			// the control to send pause frame
			if ((*pEMAC_FLC & FLCBUSY) == 0) {
				// if we're not currently sending a previous PAUSE Frame...
				// send a PAUSE Frame for (almost) two max-length frame times
				*pEMAC_FLC = SET_FLCPAUSE(48) | FLCBUSY | (dev->FlowControl?FLCE:0);
			}
		}

		// q->Completed must be non-null
		act = q->Completed;
		q->Completed = NULL;
#ifdef ADI_ETHER_BF537_DEBUG
		{
			ADI_ETHER_BUFFER *pst = act;
			noposted = 0;
			while (pst != NULL) {
				noposted++;
				pst->usage = Posted;
				pst = pst->pNext;
			}
		}
#endif

		// finally invoke the user call back
		if (dev->DMCallback!= NULL) {
			int res;


			if (dev->DCBHandle) {
#ifdef ADI_ETHER_BF537_DEBUG
				ADI_ETHER_BF537_PostedBuffer = act->CallbackParameter;
				ADI_ETHER_BF537_OutstandingPosts++;
#endif

				if (ADI_DEV_RESULT_SUCCESS!=(res=adi_dcb_Post(dev->DCBHandle,0,dev->DMCallback, dev->DeviceHandle, event, act->CallbackParameter))) {
#ifdef ADI_ETHER_BF537_DEBUG
					ADI_ETHER_BF537_OutstandingPosts--;
					{
						ADI_ETHER_BUFFER *pst = act;
						while (pst != NULL) {
							pst->usage = Completed;
							pst = pst->pNext;
						}
					}
#endif
					q->Completed = act;
					FailedPosts++;
				} else {
#ifdef ADI_ETHER_BF537_DEBUG
					ADI_ETHER_BF537_NoPosted += noposted;
#endif
					if (q->Rcve) {
						NoRxPosts++;
					} else {
						NoTxPosts++;
					}
				}
			} else {
				ExitCriticalRegion(xit);
				(dev->DMCallback)(dev->DeviceHandle, event,act->CallbackParameter);
				xit = EnterCriticalRegion(NULL);
			}
		}
	} else {
		QueueFrames(dev,q);
	}

	CheckQueues(q);
	ExitCriticalRegion(xit);


	return result;
}



//
//		EMAC RX complete event interrupt handler
//

static ADI_INT_HANDLER(RxInterruptHandler)			// RX complete interrupt handler
{
	ADI_INT_HANDLER_RESULT result,result1;
	ADI_ETHER_BF537_DATA *dev =  (ADI_ETHER_BF537_DATA *)ClientArg;

	result = ProcessCompletion(dev,&dev->Rx);

	return result;
}



//
//		EMAC TX complete event interrupt handler
//

static ADI_INT_HANDLER(TxInterruptHandler)			// TX complete interrupt handler
{
	ADI_INT_HANDLER_RESULT result;
	ADI_ETHER_BF537_DATA *dev =  (ADI_ETHER_BF537_DATA *)ClientArg;

	result = ProcessCompletion(dev,&dev->Tx);
	return result;
}



#if defined(ADI_ETHER_DEBUG)

/*********************************************************************

	Function:		ValidatePDDHandle

	Description:	Validates a PDD handle

*********************************************************************/

static int ValidatePDDHandle(ADI_DEV_PDD_HANDLE PDDHandle) {
	if (PDDHandle == (ADI_DEV_PDD_HANDLE)&EtherDev) {
		if (EtherDev.Open) {
			return (ADI_DEV_RESULT_SUCCESS);
		}
	}
	return (ADI_DEV_RESULT_BAD_PDD_HANDLE);
}


#endif


