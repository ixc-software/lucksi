#ifndef __LAN__
#define __LAN__

#include "Timer0.h"

	int LanTest();
	
typedef struct adi_emac_staadd_reg {
	u32 b_STABUSY:1;	//0		Busy
	u32 b_STAOP:1;		//1		Direction
	u32 b_STADISPRE:1;	//2		Disable preamble	
	u32 b_STAIE:1;		//3		Sta. Mgmt. Done Irq Enable 
	u32 :2;				//4:5	reserved
	u32 b_REGAD:5;		//6:10	Sta. Reg. Address
	u32 b_PHYAD:5;		//11:15	Sta. PHY  Address
	u32 :16;			//16:31 reserved
} ADI_EMAC_STAADD_REG;

typedef struct adi_ether_frame_buffer {
	u16		NoBytes;		// the no. of following bytes
	u8		Dest[6];		// destination MAC address
	u8		Srce[6];		// source MAC address
	u16		LTfield;		// length/type field
	u8		Data[0];		// payload bytes
} ADI_ETHER_FRAME_BUFFER; 

typedef struct dma_registers {
	struct dma_registers*	NEXT_DESC_PTR;
	unsigned long int		START_ADDR;
	ADI_DMA_CONFIG_REG		CONFIG;
} DMA_REGISTERS;

typedef struct adi_ether_buffer {	
	DMA_REGISTERS			Dma[2];	// first for the frame, second for the status						
	ADI_ETHER_FRAME_BUFFER	*Data;	// pointer to data
	u32	ElementCount;	// data element count
	u32	ElementWidth;	// data element width (in bytes)
	void*	CallbackParameter;// callback flag/pArg value
	u32	ProcessedFlag;	// processed flag
	u32	ProcessedElementCount;
						// # of bytes read in/out
	struct adi_ether_buffer	*pNext;	// next buffer
	void	*PayLoad;		// pointer to IP Payload
	u32	PayloadCount;	// Length of payload
	u32	PayloadWidth;	// Width of payload (in bytes)
	u16	IPHdrChksum;	// the IP header checksum
	u16	IPPayloadChksum;	// the IP header and payload checksum
	u32	StatusWord;		// the frame status word
} ADI_ETHER_BUFFER;



// -----------------------------------------------------------------------
//                     PHY REGISTER NAMES				//
// -----------------------------------------------------------------------
// PHY Control Register
#define PHY_CNTL_REG        0x00
#define PHY_CNTL_RST        0x8000  // 1=PHY Reset
#define PHY_CNTL_LPBK       0x4000  // 1=PHY Loopback
#define PHY_CNTL_SPEED      0x2000  // 1=100Mbps, 0=10Mpbs
#define PHY_CNTL_ANEG_EN    0x1000 // 1=Enable Auto negotiation
#define PHY_CNTL_PDN        0x0800  // 1=PHY Power Down mode
#define PHY_CNTL_MII_DIS    0x0400  // 1=MII 4 bit interface disabled
#define PHY_CNTL_ANEG_RST   0x0200 // 1=Reset Auto negotiate
#define PHY_CNTL_DPLX       0x0100  // 1=Full Duplex, 0=Half Duplex
#define PHY_CNTL_COLTST     0x0080  // 1= MII Colision Test

// PHY Status Register
#define PHY_STAT_REG        0x01
#define PHY_STAT_CAP_T4     0x8000  // 1=100Base-T4 capable
#define PHY_STAT_CAP_TXF    0x4000  // 1=100Base-X full duplex capable
#define PHY_STAT_CAP_TXH    0x2000  // 1=100Base-X half duplex capable
#define PHY_STAT_CAP_TF     0x1000  // 1=10Mbps full duplex capable
#define PHY_STAT_CAP_TH     0x0800  // 1=10Mbps half duplex capable
#define PHY_STAT_CAP_SUPR   0x0040  // 1=recv mgmt frames with not preamble
#define PHY_STAT_ANEG_ACK   0x0020  // 1=ANEG has completed
#define PHY_STAT_REM_FLT    0x0010  // 1=Remote Fault detected
#define PHY_STAT_CAP_ANEG   0x0008  // 1=Auto negotiate capable
#define PHY_STAT_LINK       0x0004  // 1=valid link
#define PHY_STAT_JAB        0x0002  // 1=10Mbps jabber condition
#define PHY_STAT_EXREG      0x0001  // 1=extended registers implemented
	
	

#endif
