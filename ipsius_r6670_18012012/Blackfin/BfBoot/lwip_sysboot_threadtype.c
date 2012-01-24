/* =============================================================================
 *
 *  Description: This is a C implementation for Thread lwip_sysboot_threadtype
 *
 * -----------------------------------------------------------------------------
 *  Comments:
 *
 * ===========================================================================*/

/* Get access to any of the VDK features & datatypes used */
#include "lwip_sysboot_threadtype.h"
#include <lwip/cglobals.h>
#include <lwip/kernel_abs.h>
#include <lwip/ADI_TOOLS_IOEThreadType.h>
#include <lwip/sockets.h>
#include <lwip/inet.h>
#include <sys/exception.h>
#include <services/services.h>
#include <drivers/adi_dev.h>
#include <stdio.h>
#include <string.h>
#include "adi_ssl_init.h"


// ------------------------

struct net_config_info user_net_config_info[1];
int user_net_num_ifces = 1;

// ------------------------

// our function in pkthandler.c
int InitStackEx(int poll_thread_pri, int poll_thread_per, int inMemSize, char *MemArea, net_config_info *pCfg);

/* Size of the memory block to allocate to the stack */
#define ETHER_STACK_SIZE   (2 * 1024 * 1024)

char GEthernetStack[ETHER_STACK_SIZE];


#ifndef ADDRESS_OF_MAC_ADDRESS
    #define ADDRESS_OF_MAC_ADDRESS 			0x203F0000
#endif



/*
 *  Global static data for various SSL managers
 */
static ADI_INT_IMASK    imask_storage;

/*
 *  ADSP-BF533 EZ-KIT Lite w/ USBLAN specific settings
 */


/*
 *  ADSP-BF537 EZ-KIT specific settings
 */

#if(__SILICON_REVISION__<=0x0001)
#warning Because of silicon issue, the LAN example does not work on ADI BF537 silicon revision 0.0 and 0.1.
#endif

#include <drivers/ethernet/ADI_ETHER_BF537.h>

static char hwaddr[6] = { 0xcd, 0xcd, 0xcd, 0xcd, 0xcd, 0xcd };

#define NO_RCVES 32
#define NO_XMITS 32

#pragma alignment_region (32)
char BaseMemSize[ADI_ETHER_MEM_BF537_BASE_SIZE];
char MemRcve[NO_RCVES*ADI_ETHER_MEM_BF537_PER_RECV];
char MemXmit[NO_XMITS*ADI_ETHER_MEM_BF537_PER_XMIT];
#pragma alignment_region_end

static int p_period = 40;


ADI_ETHER_SUPPLY_MEM memtable = 
{
    MemRcve,sizeof(MemRcve),0,
    MemXmit,sizeof(MemXmit),0,
    BaseMemSize,sizeof(BaseMemSize)
};

/* defined in the basiccrt.s */
extern int __cplb_ctrl;

#define SAMPLE_PERIOD 5000	// The period (in ticks) between each sample reading


void lwip_sysboot_threadtype_RunFunction(void **inPtr)
{
}

int lwip_sysboot_threadtype_ErrorFunction(void **inPtr)
{
	VDK_CThread_Error(VDK_GetThreadID());
	return 0;
}

void lwip_sysboot_threadtype_InitFunction(void **inPtr, VDK_ThreadCreationBlock *pTCB)
{
}

void lwip_sysboot_threadtype_DestroyFunction(void **inPtr)
{
}


// -----------------------------------------------------

int OpenLan(/* out */ ADI_DEV_DEVICE_HANDLE *pRes)  // return bool
{
	ADI_DEV_DEVICE_HANDLE lan_handle;
	unsigned int result;
	
    /* Initialize interrupt manager and device manager */
//    adi_ssl_Init();  // ?? -- already done in VDK startup code

	/* Initialize the kernel */
	ker_init((void*)0);

	/* set thread type for the stack threads */
	ker_set_auxdata((void*)kADI_TOOLS_IOEThreadType);



	/* open lan-device */
	result = adi_dev_Open(
                           adi_dev_ManagerHandle,
                           &ADI_ETHER_BF537_Entrypoint,
                           0,
                           NULL,
                           &lan_handle,
                           ADI_DEV_DIRECTION_BIDIRECTIONAL,
                           NULL,
                           NULL,
                           (ADI_DCB_CALLBACK_FN)stack_callback_handler);
                           
    *pRes = lan_handle;
	
	return (result == ADI_DEV_RESULT_SUCCESS);
}


// -----------------------------------------------------

void UpdateMacAddr(void)  // in hwaddr
{
    unsigned int result;

    /*
     * Read the EZ-KIT Lite's assigned MAC address, found at address 0x203F0000 +- offset.
     * We need to first set the AMGCTL register to allow access to asynchronous
     * memory.
     *
     * Bit 8 of the EBIU_AMGCTL register is also set to ensure that DMA gets more priority over
     * the processor while accessing external memory. If this is not done then frequent
     * DMA under and over runs occur when executing instructions from SDRAM
     */

    *pEBIU_AMGCTL = 0x1FF;

    memcpy ( &hwaddr, (unsigned char *) ADDRESS_OF_MAC_ADDRESS, sizeof ( hwaddr ) );

}

// -----------------------------------------------------

int system_init(ADI_DEV_DEVICE_HANDLE lan_handle, net_config_info *pCfg)  // return -1 if fail
{
	unsigned int result;
	// char *ether_stack_block;
	// u16 phyregs[32];

	/* set the services with in stack */

	set_pli_services(1,&lan_handle);

    UpdateMacAddr();

	result = adi_dev_Control(
                              lan_handle,
                              ADI_ETHER_CMD_SET_MAC_ADDR,
                              (void*)&hwaddr);

	// DEBUG_PRINT("Failed set MAC address\n",result != ADI_DEV_RESULT_SUCCESS);
	if (result != ADI_DEV_RESULT_SUCCESS) return -1;


	/* supply some memory for the driver */

	result = adi_dev_Control(
                              lan_handle,
                              ADI_ETHER_CMD_SUPPLY_MEM,
                              &memtable);

	// DEBUG_PRINT("Failed to supply memory to driver\n",result != ADI_DEV_RESULT_SUCCESS);
    if (result != ADI_DEV_RESULT_SUCCESS) return -1;
	
	result = adi_dev_Control(
                              lan_handle,
                              ADI_DEV_CMD_SET_DATAFLOW_METHOD,
                              (void*)TRUE);

	/* if __cplb_ctrl is defined to non-zero value inform the driver about it */
	if(__cplb_ctrl){
	result = adi_dev_Control(
	                         lan_handle,
	                         ADI_ETHER_CMD_BUFFERS_IN_CACHE,
	                         (void *)TRUE);
        }

	// DEBUG_PRINT("Failed to set caching mode in driver\n",result != ADI_DEV_RESULT_SUCCESS);
	if (result != ADI_DEV_RESULT_SUCCESS) return -1;

	/* Initialze the stack with user specified configuration priority -3 and
	 * poll period of p_period msec.  The stack is allocated a memory buffer as well.
	 */

	// ether_stack_block = (char *) malloc ( ETHER_STACK_SIZE );

	// DEBUG_PRINT("Failed to malloc stack \n",!ether_stack_block);
	// if (ether_stack_block == 0) return -1;

	// init_stack ( 3, p_period, sizeof(GEthernetStack), GEthernetStack);

    int res = InitStackEx(3, p_period, sizeof(GEthernetStack), GEthernetStack, pCfg);

	return 1;
}

// ---------------------------------------------------------------------

int StartMAC(ADI_DEV_DEVICE_HANDLE lan_handle)  // return bool
{
	unsigned int result;
	u16 phyregs[32];

	/* Start the MAC */
	result = adi_dev_Control (
								lan_handle,
								ADI_ETHER_CMD_START,
								NULL);

	// DEBUG_PRINT("Failed to start the driver\n",result != ADI_DEV_RESULT_SUCCESS);
	if (result != ADI_DEV_RESULT_SUCCESS) return 0;
		
	adi_dev_Control(lan_handle,ADI_ETHER_CMD_GET_PHY_REGS,phyregs);

	// DEBUG_PRINT("PHY Controller has failed and the board needs power cycled\n",phyregs[1]==0xFFFF);		
	if (phyregs[1] == 0xFFFF) return 0;
		
	return 1;
}

// ---------------------------------------------------------------------

int LinkEstablished(ADI_DEV_DEVICE_HANDLE lan_handle)  // return bool
{
	u16 phyregs[32];
	
	/* read the PHY controller registers */
	adi_dev_Control(lan_handle,ADI_ETHER_CMD_GET_PHY_REGS,phyregs);
	return ((phyregs[1]&0x4) == 0) ? 0 : 1;
}


// --------------------------------------------------------------

int Lw_Init(net_config_info *pCfg, /* out */ ADI_DEV_DEVICE_HANDLE *pLanHandle)  // return bool
{
	if (!OpenLan(pLanHandle)) return 0;
	
    if (system_init(*pLanHandle, pCfg) == -1) return 0;
    	
    return 1;
}
		
int Lw_StartMac(ADI_DEV_DEVICE_HANDLE lan_handle)
{
	return StartMAC(lan_handle);
}

int Lw_Established(ADI_DEV_DEVICE_HANDLE lan_handle)
{
	return LinkEstablished(lan_handle);
}
		
int Lw_StartStack()
{
	start_stack();
	
	return 1;	
}
		
int Lw_GetHostAddr(void *pBuff, int buffSize)
{
	memset(pBuff, 0, buffSize);
	
	if (gethostaddr(0, pBuff))
	{
		return 1;
	}
	
	return 0;
}

int Lw_GetMAC(char *pBuff, int buffSize)
{
    if ((pBuff == 0) || (buffSize != sizeof(hwaddr))) return 0;

    UpdateMacAddr();

    memcpy(pBuff, hwaddr, sizeof(hwaddr));

    return 1;
}

int Lw_Shutdown(ADI_DEV_DEVICE_HANDLE lan_handle)
{
	// shutdown lan
    int result = adi_dev_Control(lan_handle, ADI_ETHER_CMD_SHUTDOWN_DRIVER, NULL);
	if (result != ADI_DEV_RESULT_SUCCESS) return 0;

	// stop LwIP
	//stop_stack();
	
	// close LAN
	result = adi_dev_Close(lan_handle); 
    if (result != ADI_DEV_RESULT_SUCCESS) return 0;

    return 1;
}




