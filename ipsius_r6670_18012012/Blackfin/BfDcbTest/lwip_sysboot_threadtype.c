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

#include "DcbTest.h"

/* Size of the memory block to allocate to the stack */
#define ETHER_STACK_SIZE    1024*1024*1

#define DEBUG_PRINT(msg,result)		{									\
										if(result)						\
											{							\
												printf(msg);			\
												return -1;				\
										}								\
									}

#if ( defined(__ADSPBF537__) || defined(__ADSPBF536__) )
// define the following preprocessor macro in your project options
// if the MAC address should be obtained from a non-default location
#ifndef ADDRESS_OF_MAC_ADDRESS
#define ADDRESS_OF_MAC_ADDRESS 			0x203F0000
#endif
#endif



/* Initializes the Stack */
int system_init(bool skipMacStart);

/*
 *  Global static data for various SSL managers
 */
static ADI_INT_IMASK    imask_storage;

/*
 *  ADSP-BF533 EZ-KIT Lite w/ USBLAN specific settings
 */

#if ( defined(__ADSPBF533__) || defined(__ADSPBF561__) || defined(USB_LAN) || defined(__ADSPBF538__ ) )
#include <drivers/ethernet/ADI_ETHER_USBLAN.h>
#define NO_RCVES 1
#define NO_XMITS 1
#pragma alignment_region (4)
char BaseMemSize[ADI_ETHER_MEM_USBLAN_BASE_SIZE];
#pragma alignment_region_end
static int p_period =10;
#endif /* __ADSPBF533__ */

/*
 *  ADSP-BF526 EZ-KIT specific settings
 */
#ifdef __ADSPBF526__
#include <drivers/ethernet/ADI_ETHER_BF526.h>

#define ADDRESS_OF_MAC_ADDRESS 			0x203F0096

static char hwaddr[6] = { 0x00, 0xE6, 0x56, 0x78, 0x90, 0x00 };
#define NO_RCVES 8
#define NO_XMITS 10
#pragma alignment_region (32)
char BaseMemSize[ADI_ETHER_MEM_BF526_BASE_SIZE];
char MemRcve[NO_RCVES*ADI_ETHER_MEM_BF526_PER_RECV];
char MemXmit[NO_XMITS*ADI_ETHER_MEM_BF526_PER_XMIT];
#pragma alignment_region_end
static int p_period =40;
#endif /* __ADSPBF526__ */


/*
 *  ADSP-BF527 EZ-KIT specific settings
 */
#ifdef __ADSPBF527__
#include <drivers/ethernet/ADI_ETHER_BF527.h>
#include <adi_otp.h>
#include <otp_helper_functions.h>
#include "adi_ssl_Init.h"
// MAC address resides in the last un-secured GP space
#define OTP_MAC_ADDRESS_PAGE	0xDF
ADI_DEV_DEVICE_HANDLE DevHandleOTP;
static void OTP_Callback(void *ClientHandle, u32 Event, void *pArg);
int ReadMACAddress(char * cMacAddress);

static char hwaddr[6] = { 0xcd, 0xcd, 0xcd, 0xcd, 0xcd, 0xcd };
#define NO_RCVES 8
#define NO_XMITS 10
#pragma alignment_region (32)
char BaseMemSize[ADI_ETHER_MEM_BF527_BASE_SIZE];
char MemRcve[NO_RCVES*ADI_ETHER_MEM_BF527_PER_RECV];
char MemXmit[NO_XMITS*ADI_ETHER_MEM_BF527_PER_XMIT];
#pragma alignment_region_end
static int p_period =40;
#endif /* __ADSPBF527__ */

/*
 *  ADSP-BF537 EZ-KIT specific settings
 */
#if (!defined(USB_LAN) &&  (defined(__ADSPBF537__) || defined(__ADSPBF536__) ) )
#if(__SILICON_REVISION__<=0x0001)
#warning Because of silicon issue, the LAN example does not work on ADI BF537 silicon revision 0.0 and 0.1.
#endif

#include <drivers/ethernet/ADI_ETHER_BF537.h>
static char hwaddr[6] = { 0xcd, 0xcd, 0xcd, 0xcd, 0xcd, 0xcd };
#define NO_RCVES 8
#define NO_XMITS 10
#pragma alignment_region (32)
char BaseMemSize[ADI_ETHER_MEM_BF537_BASE_SIZE];
char MemRcve[NO_RCVES*ADI_ETHER_MEM_BF537_PER_RECV];
char MemXmit[NO_XMITS*ADI_ETHER_MEM_BF537_PER_XMIT];
#pragma alignment_region_end
static int p_period =40;
#endif /* __ADSPBF537__ */


/*
 *  ADSP-BF548 EZ-KIT Lite specific settings
 */

#if defined(__ADSPBF548__)
#include <drivers/ethernet/ADI_ETHER_LAN9218.h>
#define NO_RCVES 1
#define NO_XMITS 1
#pragma alignment_region (4)
char BaseMemSize[ADI_ETHER_MEM_LAN9218_BASE_SIZE];

char MemRcve[NO_RCVES*ADI_ETHER_MEM_LAN9218_PER_RECV];
char MemXmit[NO_XMITS*ADI_ETHER_MEM_LAN9218_PER_XMIT];

#pragma alignment_region_end
static int p_period =10;
#endif /* __ADSPBF548__ */


ADI_ETHER_SUPPLY_MEM memtable = {
#if ( !defined(USB_LAN) && (defined(__ADSPBF527__) || defined(__ADSPBF526__)|| defined(__ADSPBF537__) || defined(__ADSPBF536__) ) )
    MemRcve,sizeof(MemRcve),0,
    MemXmit,sizeof(MemXmit),0,
#endif
#if (defined(USB_LAN) || defined(__ADSPBF533__) || defined(__ADSPBF561__) || defined(__ADSPBF538__) )
    0,0,0,
    0,0,0,
#endif
#if defined(__ADSPBF548__)
/*
    0,0,0,
    0,0,0,
    */
    MemRcve,sizeof(MemRcve),0,
    MemXmit,sizeof(MemXmit),0,

#endif
    BaseMemSize,sizeof(BaseMemSize)};

/* defined in the basiccrt.s */
extern int __cplb_ctrl;

#define SAMPLE_PERIOD 5000	// The period (in ticks) between each sample reading


void
lwip_sysboot_threadtype_RunFunction(void **inPtr)
{
	char  ip[32];

        // Enable the Periodic Semaphore
	VDK_MakePeriodic(kPeriodic, SAMPLE_PERIOD, SAMPLE_PERIOD);


	/* Initializes the TCP/IP Stack and returns */
	if(system_init(true) == -1)
	{
		printf("Failed to initialize system\n");
		return;
	}

	/* start stack */
	/////start_stack();

	/*
	 * For debug purposes, printf() the IP address to the VisualDSP++
	 * console window.  Likely not needed in final application.
	 */

	////memset(ip,0,sizeof(ip));
	////if(gethostaddr(0,ip))
	{
	////	printf("IP ADDRESS: %s\n",ip);
	}

	/**
	 *  Add Application Code here
	 **/
	 DcbTest();

	/* Put the thread's exit from "main" HERE */
	/* A thread is automatically Destroyed when it exits its run function */
}

int
lwip_sysboot_threadtype_ErrorFunction(void **inPtr)
{

    /* TODO - Put this thread's error handling code HERE */

      /* report error and jump to KernelPanic */

	VDK_CThread_Error(VDK_GetThreadID());
	return 0;
}

void
lwip_sysboot_threadtype_InitFunction(void **inPtr, VDK_ThreadCreationBlock *pTCB)
{
    /* Put code to be executed when this thread has just been created HERE */

    /* This routine does NOT run in new thread's context.  Any non-static thread
     *   initialization should be performed at the beginning of "Run()."
     */
}

void
lwip_sysboot_threadtype_DestroyFunction(void **inPtr)
{
    /* Put code to be executed when this thread is destroyed HERE */

    /* This routine does NOT run in the thread's context.  Any VDK API calls
     *   should be performed at the end of "Run()."
     */
}

/******************************************************************
 *
 *  Function: system_init()
 *  Description:
 *  		Initializes Device Manager, Interrupt
 *  		Manager and the Stack.
 *   Return:
 *   		Returns 1 upon success -1 upon failure.
 *
 ******************************************************************/
int system_init(bool skipMacStart)
{
	unsigned int result;
	ADI_DEV_DEVICE_HANDLE lan_handle;
	char *ether_stack_block;
	u16 phyregs[32];


    /* Initialize interrupt manager and device manager */
    adi_ssl_Init();

#ifdef __ADSPBF526__

	/* Set CCLK = 400 MHz, SCLK = 80 MHz */
	adi_pwr_SetFreq(400000000,80000000, ADI_PWR_DF_NONE);

#endif
	/* Initialize the kernel */

	ker_init((void*)0);

	/* set thread type for the stack threads */

	ker_set_auxdata((void*)kADI_TOOLS_IOEThreadType);



	/* open lan-device */

	result = adi_dev_Open(
                           adi_dev_ManagerHandle,
#if ( defined(USB_LAN) || defined(__ADSPBF533__) || defined(__ADSPBF561__) || defined(__ADSPBF538__) )
                           &ADI_ETHER_USBLAN_Entrypoint,
#elif defined(__ADSPBF526__)
                           &ADI_ETHER_BF526_Entrypoint,
#elif defined(__ADSPBF527__)
                           &ADI_ETHER_BF527_Entrypoint,
#elif ( defined(__ADSPBF537__) || defined(__ADSPBF536__) )
                           &ADI_ETHER_BF537_Entrypoint,
#elif defined(__ADSPBF548__)
                           &ADI_ETHER_LAN9218_Entrypoint,
#endif
                           0,
                           NULL,
                           &lan_handle,
                           ADI_DEV_DIRECTION_BIDIRECTIONAL,
                           NULL,
                           NULL,
                           (ADI_DCB_CALLBACK_FN)stack_callback_handler);

	DEBUG_PRINT("Failed to open the lan-device\n",result != ADI_DEV_RESULT_SUCCESS);

	/* set the services with in stack */

	set_pli_services(1,&lan_handle);

#if (!defined(USB_LAN) && (defined(__ADSPBF527__) || defined(__ADSPBF526__) || defined(__ADSPBF537__) || defined(__ADSPBF536__) ) )

	*pEBIU_AMGCTL = 0x1FF;

#if defined(__ADSPBF527__)
	result = ReadMACAddress(hwaddr);
#endif //BF527

	/*
	 * Read the EZ-KIT Lite's assigned MAC address, found at address 0x203F0000 +- offset.
	 * We need to first set the AMGCTL register to allow access to asynchronous
	 * memory.
	 *
	 * Bit 8 of the EBIU_AMGCTL register is also set to ensure that DMA gets more priority over
	 * the processor while accessing external memory. If this is not done then frequent
	 * DMA under and over runs occur when executing instructions from SDRAM
	 */

#if ( defined(__ADSPBF526__) || defined(__ADSPBF537__) || defined(__ADSPBF536__) )
	memcpy ( &hwaddr, (unsigned char *) ADDRESS_OF_MAC_ADDRESS, sizeof ( hwaddr ) );
#endif
	result = adi_dev_Control(
                              lan_handle,
                              ADI_ETHER_CMD_SET_MAC_ADDR,
                              (void*)&hwaddr);

	DEBUG_PRINT("Failed set MAC address\n",result != ADI_DEV_RESULT_SUCCESS);

#endif
	/* supply some memory for the driver */

	result = adi_dev_Control(
                              lan_handle,
                              ADI_ETHER_CMD_SUPPLY_MEM,
                              &memtable);

	DEBUG_PRINT("Failed to supply memory to driver\n",result != ADI_DEV_RESULT_SUCCESS);

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

	DEBUG_PRINT("Failed to set caching mode in driver\n",result != ADI_DEV_RESULT_SUCCESS);

	/* Initialze the stack with user specified configuration priority -3 and
	 * poll period of p_period msec.  The stack is allocated a memory buffer as well.
	 */

	ether_stack_block = (char *) malloc ( ETHER_STACK_SIZE );

	DEBUG_PRINT("Failed to malloc stack \n",!ether_stack_block);

	init_stack ( 3, p_period, ETHER_STACK_SIZE, ether_stack_block );

	if (skipMacStart) return 1; 
	/* Start the MAC */

	result = adi_dev_Control (
								lan_handle,
								ADI_ETHER_CMD_START,
								NULL);

	DEBUG_PRINT("Failed to start the driver\n",result != ADI_DEV_RESULT_SUCCESS);

	/* read the PHY controller registers */
	adi_dev_Control(lan_handle,ADI_ETHER_CMD_GET_PHY_REGS,phyregs);

	DEBUG_PRINT("PHY Controller has failed and the board needs power cycled\n",phyregs[1]==0xFFFF);

	/* wait for the link to be up */
	if ( (phyregs[1]&0x4) ==0)
	{
		printf("Waiting for the link to be established\n");
		while ( (phyregs[1]&0x4) ==0)
		{
			// wait period of time
			VDK_PendSemaphore(kPeriodic,0);
			adi_dev_Control(lan_handle,ADI_ETHER_CMD_GET_PHY_REGS,phyregs);
		}
	}

	printf("Link established\n");

	return 1;
}

#if defined (__ADSPBF527__)

/*******************************************************************
*   Function:    OTP_Callback
*   Description: Invoked when the driver needs to notify the application
*	             of something
*******************************************************************/
static void OTP_Callback(void *ClientHandle, u32 Event, void *pArg)
{
	// just return
}


int ReadMACAddress(char * cMacAddress)
{
	u32 Result = ADI_DEV_RESULT_SUCCESS;
	u32 access_mode = ADI_OTP_ACCESS_READ;
	u64 macaddress;
	u16 i=0;
	char *ptr=cMacAddress;
	char MACaddr[6] = { 0xcd, 0xcd, 0xcd, 0xcd, 0xcd, 0xcd };

	// open the OTP driver
	Result = adi_dev_Open(	adi_dev_ManagerHandle,			// DevMgr handle
							&ADIOTPEntryPoint,				// pdd entry point
							0,								// device instance
							NULL,							// client handle callback identifier
							&DevHandleOTP,					// DevMgr handle for this device
							ADI_DEV_DIRECTION_BIDIRECTIONAL,// data direction for this device
							NULL,							// handle to DmaMgr for this device
							NULL,							// handle to deferred callback service
							OTP_Callback);					// client's callback function

	if (Result != ADI_DEV_RESULT_SUCCESS)
		return 0;

	access_mode = ADI_OTP_ACCESS_READWRITE;

	Result = adi_dev_Control(DevHandleOTP, ADI_OTP_CMD_SET_ACCESS_MODE, &access_mode );
	if (Result != ADI_DEV_RESULT_SUCCESS)
	{
		adi_dev_Close( DevHandleOTP );
		return 0;
	}

	// Set Dataflow method
	Result = adi_dev_Control(DevHandleOTP, ADI_DEV_CMD_SET_DATAFLOW_METHOD, (void *)ADI_DEV_MODE_CHAINED );
	if(Result != ADI_DEV_RESULT_SUCCESS)
	{
		adi_dev_Close( DevHandleOTP );
		return 0;
    }


	Result = otp_read_page(OTP_MAC_ADDRESS_PAGE, ADI_OTP_LOWER_HALF, (u64*)&MACaddr);
	if( Result != ADI_DEV_RESULT_SUCCESS )
		return 0;

	// The MAC address is in reverse order
	// do byte swape
	for(i=0; i<6; i++)
	{
		*ptr++ = MACaddr[5-i];

	}

    adi_dev_Close( DevHandleOTP );

    return 1;

}
#endif //__ADSPBF527__
/* ========================================================================== */
