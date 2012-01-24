/*********************************************************************************

Copyright(c) 2005 Analog Devices, Inc. All Rights Reserved. 

This software is proprietary and confidential.  By using this software you agree
to the terms of the associated Analog Devices License Agreement.  
			
*********************************************************************************/

/*********************************************************************

This file contains a convenient mechanism to initialize and terminate 
all system services and the device manager.  The application should 
modify the sizings located in the adi_ssl_Init.h file as needed by their
application, then add this file to their list of source files for
their project.  

The application should then make one call to the function adi_ssl_Init(), 
insuring the return value from the function call returns the value 0.  
This function call initialized all services and the device manager 
according to the sizings defined in adi_ssl_init.h.

When no longer needed, the application can then one call to the function
adi_ssl_Terminate(), insuring the return value from the function call
returns the value 0.  This function call terminates all services and 
the device manager.  

The handles to the DMA and Device Manager are stored in the global 
variables adi_dma_ManagerHandle and adi_dev_ManagerHandle, respectively.
These handles can be passed to subsequent adi_dev_Open() calls as
necessary.

DO NOT MODIFY ANYTHING IN THIS FILE

*********************************************************************/


/*********************************************************************

Include files

*********************************************************************/

#include <services/services.h>		// system service includes
#include <drivers/adi_dev.h>        // device driver includes

#include "adi_ssl_init.h"           // initialization sizings



/*********************************************************************

Handles

This section provides storage for handles into the services and device 
manager.  The application may use these handles into calls such as
adi_dev_Open() for opening device drivers, adi_dma_OpenChannel() for
opening DMA channels etc.  


*********************************************************************/

ADI_DMA_MANAGER_HANDLE adi_dma_ManagerHandle;   // handle to the DMA manager
ADI_DEV_MANAGER_HANDLE adi_dev_ManagerHandle;   // handle to the device manager
	

/*********************************************************************

Global storage data

This section provides memory, based on the sizing defined above, for 
each of the services. 

*********************************************************************/

static  u8  InterruptServiceData        [ADI_INT_SECONDARY_MEMORY * ADI_SSL_INT_NUM_SECONDARY_HANDLERS];
static  u8  DeferredCallbackServiceData [ADI_DCB_QUEUE_SIZE * ADI_SSL_DCB_NUM_SERVERS];
static  u8  DMAServiceData              [ADI_DMA_BASE_MEMORY + (ADI_DMA_CHANNEL_MEMORY *  ADI_SSL_DMA_NUM_CHANNELS)];
static  u8  FlagServiceData             [ADI_FLAG_CALLBACK_MEMORY *  ADI_SSL_FLAG_NUM_CALLBACKS];
static  u8  SemaphoreServiceData        [ADI_SEM_SEMAPHORE_MEMORY *  ADI_SSL_SEM_NUM_SEMAPHORES];

static  u8  DevMgrData                  [ADI_DEV_BASE_MEMORY + (ADI_DEV_DEVICE_MEMORY * ADI_SSL_DEV_NUM_DEVICES)];



/*********************************************************************

	Function:		adi_ssl_Init

	Description:	Initializes the system services and device manager.


*********************************************************************/

u32 adi_ssl_Init(void) {
    
    u32 i;
    u32 Result;


	/* definitions for asynch memory controller commands */

	/*  global control register fields */
	/*  clkout enable */
	ADI_EBIU_ASYNCH_CLKOUT clkout_enable = ADI_EBIU_ASYNCH_CLKOUT_ENABLE;
	/*  which banks to enable */
	ADI_EBIU_ASYNCH_BANK_ENABLE banks_enable = ADI_EBIU_ASYNCH_BANK0_1_2_3;
 
	/* bank timing parameters - specified in cycles */
	/* transition times for the four banks */
	ADI_EBIU_ASYNCH_BANK_TIMING asynch_bank_trans_time = {ADI_EBIU_BANK_ALL, { ADI_EBIU_ASYNCH_TT_4_CYCLES, { 0, ADI_EBIU_TIMING_UNIT_NANOSEC } } };
 
	/* time between Read Enable assertion to de-assertion */
	ADI_EBIU_ASYNCH_BANK_TIMING asynch_bank_read_access_time = { ADI_EBIU_BANK_ALL,  { 0xB, { 0, ADI_EBIU_TIMING_UNIT_NANOSEC } } };
 
	/* time between Write Enable  assertion to de-assertion */
	ADI_EBIU_ASYNCH_BANK_TIMING asynch_bank_write_access_time = { ADI_EBIU_BANK_ALL,  { 7, { 0, ADI_EBIU_TIMING_UNIT_NANOSEC } } };
  
	/* time from beginning of memory cycle to R/W-enable */
	ADI_EBIU_ASYNCH_BANK_TIMING asynch_bank_setup_time = { ADI_EBIU_BANK_ALL,  { ADI_EBIU_ASYNCH_ST_3_CYCLES, { 0, ADI_EBIU_TIMING_UNIT_NANOSEC } } };
   
	/* time from de-assertion  to end of memory cycle */
	ADI_EBIU_ASYNCH_BANK_TIMING asynch_bank_hold_time = {ADI_EBIU_BANK_ALL,  { ADI_EBIU_ASYNCH_HT_2_CYCLES, { 0, ADI_EBIU_TIMING_UNIT_NANOSEC } } };

	/* specify whether ARDY enabled (is used to insert extra wait states) */
	ADI_EBIU_ASYNCH_BANK_VALUE asynch_bank_ardy_enable = { ADI_EBIU_BANK_ALL, { ardy_enable: ADI_EBIU_ASYNCH_ARDY_DISABLE } };

    	
	/* specify whether ARDY is sampled low or high */	
	ADI_EBIU_ASYNCH_BANK_VALUE asynch_bank_ardy_polarity = { ADI_EBIU_BANK_ALL, { ardy_polarity: ADI_EBIU_ASYNCH_ARDY_POLARITY_LOW } };



/********************************************************************/
#if defined(__ADSP_EDINBURGH__)		// BF533 EZKit

	ADI_PWR_COMMAND_PAIR ezkit_power[] = { 

#if defined ADI_SSL_BF533_6V 
    	{ ADI_PWR_CMD_SET_PROC_VARIANT,(void*)ADI_PWR_PROC_BF533SKBC600_6V  },  // 600Mhz ADSP-BF533 6V variant 
#else
    	{ ADI_PWR_CMD_SET_PROC_VARIANT,(void*)ADI_PWR_PROC_BF533SKBC600  },     // 600Mhz ADSP-BF533 variant 
#endif
    	{ ADI_PWR_CMD_SET_PACKAGE,     (void*)ADI_PWR_PACKAGE_MBGA       },     // in MBGA packaging, as on all EZ-KITS
    	{ ADI_PWR_CMD_SET_VDDEXT,      (void*)ADI_PWR_VDDEXT_330         },     // external voltage supplied to the voltage regulator is 3.3V

#if defined ADI_SSL_EXT_VOLTAGE_REG
	{ ADI_PWR_CMD_SET_VDDINT,      (void*)ADI_SSL_EXT_VDDINT         },	/* Externally supplied VDDINT (see adi_ssl_Init.h)*/		
#endif

    	{ ADI_PWR_CMD_SET_CLKIN,       (void*)27                         },	// the CLKIN frequency 27 MHz
    	{ ADI_PWR_CMD_END,             0                                 } 
	}; 

	ADI_EBIU_TIMING_VALUE     twrmin       = {1,{7500, ADI_EBIU_TIMING_UNIT_PICOSEC}};   // set min TWR to 1 SCLK cycle + 7.5ns	
	ADI_EBIU_TIMING_VALUE     refresh      = {8192,{64, ADI_EBIU_TIMING_UNIT_MILLISEC}}; // set refresh period to 8192 cycles in 64ms
	ADI_EBIU_TIME             trasmin      = {44, ADI_EBIU_TIMING_UNIT_NANOSEC};         // set min TRAS to 44ns
	ADI_EBIU_TIME             trpmin       = {20, ADI_EBIU_TIMING_UNIT_NANOSEC};	     // set min TRP to 20ns
	ADI_EBIU_TIME             trcdmin      = {20, ADI_EBIU_TIMING_UNIT_NANOSEC}; 	     // set min TRCD to 20ns
	u32                       cl_threshold = 100;                                        // set cl threshold to 100 Mhz
#if defined( ADI_SSL_EDIN_REV_1_7 )
	ADI_EBIU_SDRAM_BANK_VALUE bank_size    = {0, {size: ADI_EBIU_SDRAM_BANK_64MB }};      // set bank size to 64MB
	ADI_EBIU_SDRAM_BANK_VALUE bank_width   = {0, {width: ADI_EBIU_SDRAM_BANK_COL_10BIT}}; // set column address width to 10-Bit
#else
	ADI_EBIU_SDRAM_BANK_VALUE bank_size    = {0, {size: ADI_EBIU_SDRAM_BANK_32MB }};     // set bank size to 64MB
	ADI_EBIU_SDRAM_BANK_VALUE bank_width   = {0, {width: ADI_EBIU_SDRAM_BANK_COL_9BIT}}; // set column address width to 9-Bit
#endif

	ADI_EBIU_COMMAND_PAIR ezkit_ram[] = { 
	{ ADI_EBIU_CMD_SET_SDRAM_BANK_SIZE,     (void*)&bank_size   },
       	{ ADI_EBIU_CMD_SET_SDRAM_BANK_COL_WIDTH,(void*)&bank_width  },
       	{ ADI_EBIU_CMD_SET_SDRAM_CL_THRESHOLD,  (void*)cl_threshold },
      	{ ADI_EBIU_CMD_SET_SDRAM_TRASMIN,       (void*)&trasmin     }, 
       	{ ADI_EBIU_CMD_SET_SDRAM_TRPMIN,        (void*)&trpmin      }, 
       	{ ADI_EBIU_CMD_SET_SDRAM_TRCDMIN,       (void*)&trcdmin     }, 
       	{ ADI_EBIU_CMD_SET_SDRAM_TWRMIN,        (void*)&twrmin      },
       	{ ADI_EBIU_CMD_SET_SDRAM_REFRESH,       (void*)&refresh     },
       	/* Asynch Commands memory controller commands */ 
       	{ ADI_EBIU_CMD_SET_ASYNCH_CLKOUT_ENABLE,          (void*)&clkout_enable },
       	{ ADI_EBIU_CMD_SET_ASYNCH_BANK_ENABLE,            (void*)&banks_enable },                                 
       	{ ADI_EBIU_CMD_SET_ASYNCH_BANK_TRANSITION_TIME,   (void*)&asynch_bank_trans_time  },
       	{ ADI_EBIU_CMD_SET_ASYNCH_BANK_READ_ACCESS_TIME,  (void*)&asynch_bank_read_access_time  }, 
       	{ ADI_EBIU_CMD_SET_ASYNCH_BANK_WRITE_ACCESS_TIME, (void*)&asynch_bank_write_access_time  },
       	{ ADI_EBIU_CMD_SET_ASYNCH_BANK_SETUP_TIME,        (void*)&asynch_bank_setup_time  }, 
       	{ ADI_EBIU_CMD_SET_ASYNCH_BANK_HOLD_TIME,         (void*)&asynch_bank_hold_time  },
       	{ ADI_EBIU_CMD_SET_ASYNCH_BANK_ARDY_ENABLE,       (void*)&asynch_bank_ardy_enable  },  
       	{ ADI_EBIU_CMD_SET_ASYNCH_BANK_ARDY_POLARITY,     (void*)&asynch_bank_ardy_polarity },  
      	{ ADI_EBIU_CMD_END,                     0                   }
	};
	
#endif
/********************************************************************/
#if defined(__ADSP_BRAEMAR__)		// BF537 EZKit

	ADI_PWR_COMMAND_PAIR ezkit_power[] = { 
    	{ ADI_PWR_CMD_SET_PROC_VARIANT,(void*)ADI_PWR_PROC_BF537SKBC1600 }, // 600Mhz ADSP-BF537
    	{ ADI_PWR_CMD_SET_PACKAGE,     (void*)ADI_PWR_PACKAGE_MBGA       }, // in MBGA packaging, as on all EZ-KITS
    	{ ADI_PWR_CMD_SET_VDDEXT,      (void*)ADI_PWR_VDDEXT_330         }, // external voltage supplied to the voltage regulator is 3.3V
#if defined ADI_SSL_EXT_VOLTAGE_REG
	{ ADI_PWR_CMD_SET_VDDINT,      (void*)ADI_SSL_EXT_VDDINT         },	/* Externally supplied VDDINT (see adi_ssl_Init.h)*/		
#endif
    	{ ADI_PWR_CMD_SET_CLKIN,       (void*)25                         },	// the CLKIN frequency 25 MHz
    	{ ADI_PWR_CMD_END,             0                                 } 
	}; 

	ADI_EBIU_TIMING_VALUE     twrmin       = {1,{7500, ADI_EBIU_TIMING_UNIT_PICOSEC}};    // set min TWR to 1 SCLK cycle + 7.5ns	
	ADI_EBIU_TIMING_VALUE     refresh      = {8192,{64, ADI_EBIU_TIMING_UNIT_MILLISEC}};  // set refresh period to 8192 cycles in 64ms
	ADI_EBIU_TIME             trasmin      = {44, ADI_EBIU_TIMING_UNIT_NANOSEC};          // set min TRAS to 44ns
	ADI_EBIU_TIME             trpmin       = {20, ADI_EBIU_TIMING_UNIT_NANOSEC};	      // set min TRP to 20ns
	ADI_EBIU_TIME             trcdmin      = {20, ADI_EBIU_TIMING_UNIT_NANOSEC}; 	      // set min TRCD to 20ns
	u32                       cl_threshold = 100;                                         // set cl threshold to 100 Mhz
	ADI_EBIU_SDRAM_BANK_VALUE bank_size    = {0, {size: ADI_EBIU_SDRAM_BANK_64MB }};      // set bank size to 64MB
	ADI_EBIU_SDRAM_BANK_VALUE bank_width   = {0, {width: ADI_EBIU_SDRAM_BANK_COL_10BIT}}; // set column address width to 10-Bit

	ADI_EBIU_COMMAND_PAIR ezkit_ram[] = { 
		{ ADI_EBIU_CMD_SET_SDRAM_BANK_SIZE,     (void*)&bank_size   },
       	{ ADI_EBIU_CMD_SET_SDRAM_BANK_COL_WIDTH,(void*)&bank_width  },
       	{ ADI_EBIU_CMD_SET_SDRAM_CL_THRESHOLD,  (void*)cl_threshold },
      	{ ADI_EBIU_CMD_SET_SDRAM_TRASMIN,       (void*)&trasmin     }, 
       	{ ADI_EBIU_CMD_SET_SDRAM_TRPMIN,        (void*)&trpmin      }, 
       	{ ADI_EBIU_CMD_SET_SDRAM_TRCDMIN,       (void*)&trcdmin     }, 
       	{ ADI_EBIU_CMD_SET_SDRAM_TWRMIN,        (void*)&twrmin      },
       	{ ADI_EBIU_CMD_SET_SDRAM_REFRESH,       (void*)&refresh     },
       	/* Asynch Commands memory controller commands */ 
       	{ ADI_EBIU_CMD_SET_ASYNCH_CLKOUT_ENABLE,          (void*)&clkout_enable },
       	{ ADI_EBIU_CMD_SET_ASYNCH_BANK_ENABLE,            (void*)&banks_enable },                                 
       	{ ADI_EBIU_CMD_SET_ASYNCH_BANK_TRANSITION_TIME,   (void*)&asynch_bank_trans_time  },
       	{ ADI_EBIU_CMD_SET_ASYNCH_BANK_READ_ACCESS_TIME,  (void*)&asynch_bank_read_access_time  }, 
       	{ ADI_EBIU_CMD_SET_ASYNCH_BANK_WRITE_ACCESS_TIME, (void*)&asynch_bank_write_access_time  },
       	{ ADI_EBIU_CMD_SET_ASYNCH_BANK_SETUP_TIME,        (void*)&asynch_bank_setup_time  }, 
       	{ ADI_EBIU_CMD_SET_ASYNCH_BANK_HOLD_TIME,         (void*)&asynch_bank_hold_time  },
       	{ ADI_EBIU_CMD_SET_ASYNCH_BANK_ARDY_ENABLE,       (void*)&asynch_bank_ardy_enable  },  
       	{ ADI_EBIU_CMD_SET_ASYNCH_BANK_ARDY_POLARITY,     (void*)&asynch_bank_ardy_polarity },  
      	{ ADI_EBIU_CMD_END,                     0                   }
	};
	
#endif
/********************************************************************/
#if defined(__ADSP_TETON__)		// BF561 EZKit
	ADI_PWR_COMMAND_PAIR ezkit_power[] = { 
    	{ ADI_PWR_CMD_SET_PROC_VARIANT,         (void*)ADI_PWR_PROC_BF561SKBCZ500X  },  // 500Mhz ADSP-BF561
    	{ ADI_PWR_CMD_SET_PACKAGE,              (void*)ADI_PWR_PACKAGE_MBGA         },  // in MBGA packaging, as on all EZ-KITS
    	{ ADI_PWR_CMD_SET_VDDEXT,               (void*)ADI_PWR_VDDEXT_330           },  // external voltage supplied to the voltage regulator is 3.3V
#if defined ADI_SSL_EXT_VOLTAGE_REG
	{ ADI_PWR_CMD_SET_VDDINT,      (void*)ADI_SSL_EXT_VDDINT         },	/* Externally supplied VDDINT (see adi_ssl_Init.h)*/		
#endif
    	{ ADI_PWR_CMD_SET_CLKIN,                (void*)30                           },	// the CLKIN frequency 30 MHz

#if defined( ADI_SSL_DUAL_CORE )
    	{ ADI_PWR_CMD_SET_AUTO_SYNC_ENABLED,    NULL                                },	// enable autosync so the core A and B sync up
#endif

    	{ ADI_PWR_CMD_END,                      0                                   } 
	}; 

	ADI_EBIU_TIMING_VALUE     twrmin       = {1,{7500, ADI_EBIU_TIMING_UNIT_PICOSEC}};   // set min TWR to 1 SCLK cycle + 7.5ns	
	ADI_EBIU_TIMING_VALUE     refresh      = {8192,{64, ADI_EBIU_TIMING_UNIT_MILLISEC}}; // set refresh period to 8192 cycles in 64ms
	ADI_EBIU_TIME             trasmin      = {44, ADI_EBIU_TIMING_UNIT_NANOSEC};         // set min TRAS to 44ns
	ADI_EBIU_TIME             trpmin       = {20, ADI_EBIU_TIMING_UNIT_NANOSEC};	     // set min TRP to 20ns
	ADI_EBIU_TIME             trcdmin      = {20, ADI_EBIU_TIMING_UNIT_NANOSEC}; 	     // set min TRCD to 20ns
	u32                       cl_threshold = 100;                                        // set cl threshold to 100 Mhz

	
	// Teton has four separately configurable banks. Only one bank is used on the EZ-Kit
	ADI_EBIU_SDRAM_BANK_VALUE bank0_size   = {0, {size: ADI_EBIU_SDRAM_BANK_64MB }};      // set bank 0 size to 64MB
	ADI_EBIU_SDRAM_BANK_VALUE bank0_width  = {0, {width: ADI_EBIU_SDRAM_BANK_COL_9BIT}}; // set bank 0 column address width to 9-Bit
              

	/* Set 16 bit packing enable for all four asynch banks */
	ADI_EBIU_ASYNCH_BANK_VALUE asynch_bank_packing_enable = { ADI_EBIU_BANK_ALL, { data_path: ADI_EBIU_ASYNCH_BANK_DATA_PATH_16 }};

	/* Teton uses bank 3 for Netchip device.  Overwrite the global "bank_all" configuration of this bank */

	/* time between Read Enable assertion to de-assertion */
	ADI_EBIU_ASYNCH_BANK_TIMING asynch_bank3_read_access_time = { ADI_EBIU_BANK_3,  { 3, { 0, ADI_EBIU_TIMING_UNIT_NANOSEC } } };
 
	/* time between Write Enable  assertion to de-assertion */
	ADI_EBIU_ASYNCH_BANK_TIMING asynch_bank3_write_access_time = { ADI_EBIU_BANK_3,  { 1, { 0, ADI_EBIU_TIMING_UNIT_NANOSEC } } };
  
	/* time from beginning of memory cycle to R/W-enable */
	ADI_EBIU_ASYNCH_BANK_TIMING asynch_bank3_setup_time = { ADI_EBIU_BANK_3,  { ADI_EBIU_ASYNCH_ST_1_CYCLES, { 0, ADI_EBIU_TIMING_UNIT_NANOSEC } } };
   
	/* time from de-assertion  to end of memory cycle */
	ADI_EBIU_ASYNCH_BANK_TIMING asynch_bank3_hold_time = {ADI_EBIU_BANK_3,  { ADI_EBIU_ASYNCH_HT_3_CYCLES, { 0, ADI_EBIU_TIMING_UNIT_NANOSEC } } };


	ADI_EBIU_COMMAND_PAIR ezkit_ram[] = { 
      	{ ADI_EBIU_CMD_SET_SDRAM_BANK_SIZE,     (void*)&bank0_size   },
      	{ ADI_EBIU_CMD_SET_SDRAM_BANK_COL_WIDTH,(void*)&bank0_width  },	       	
       	{ ADI_EBIU_CMD_SET_SDRAM_CL_THRESHOLD,  (void*)cl_threshold  },
      	{ ADI_EBIU_CMD_SET_SDRAM_TRASMIN,       (void*)&trasmin      }, 
       	{ ADI_EBIU_CMD_SET_SDRAM_TRPMIN,        (void*)&trpmin       }, 
       	{ ADI_EBIU_CMD_SET_SDRAM_TRCDMIN,       (void*)&trcdmin      }, 
       	{ ADI_EBIU_CMD_SET_SDRAM_TWRMIN,        (void*)&twrmin       },
       	{ ADI_EBIU_CMD_SET_SDRAM_REFRESH,       (void*)&refresh      }, 
       	/* Asynch Commands memory controller commands */ 
       	{ ADI_EBIU_CMD_SET_ASYNCH_CLKOUT_ENABLE,          (void*)&clkout_enable },
       	{ ADI_EBIU_CMD_SET_ASYNCH_BANK_ENABLE,            (void*)&banks_enable },                                 
       	{ ADI_EBIU_CMD_SET_ASYNCH_BANK_TRANSITION_TIME,   (void*)&asynch_bank_trans_time  },
       	{ ADI_EBIU_CMD_SET_ASYNCH_BANK_READ_ACCESS_TIME,  (void*)&asynch_bank_read_access_time  }, 
       	{ ADI_EBIU_CMD_SET_ASYNCH_BANK_WRITE_ACCESS_TIME, (void*)&asynch_bank_write_access_time  },
       	{ ADI_EBIU_CMD_SET_ASYNCH_BANK_SETUP_TIME,        (void*)&asynch_bank_setup_time  }, 
       	{ ADI_EBIU_CMD_SET_ASYNCH_BANK_HOLD_TIME,         (void*)&asynch_bank_hold_time  },
       	{ ADI_EBIU_CMD_SET_ASYNCH_BANK_ARDY_ENABLE,       (void*)&asynch_bank_ardy_enable  },  
       	{ ADI_EBIU_CMD_SET_ASYNCH_BANK_ARDY_POLARITY,     (void*)&asynch_bank_ardy_polarity },         	
       	{ ADI_EBIU_CMD_SET_ASYNCH_BANK_16_BIT_PACKING_ENABLE, (void*)&asynch_bank_packing_enable },


	/* Teton uses bank 3 for the Netchip device so send these additional commands*/

      	{ ADI_EBIU_CMD_SET_ASYNCH_BANK_READ_ACCESS_TIME,  (void*)&asynch_bank3_read_access_time  }, 
       	{ ADI_EBIU_CMD_SET_ASYNCH_BANK_WRITE_ACCESS_TIME, (void*)&asynch_bank3_write_access_time  },
       	{ ADI_EBIU_CMD_SET_ASYNCH_BANK_SETUP_TIME,        (void*)&asynch_bank3_setup_time  }, 
       	{ ADI_EBIU_CMD_SET_ASYNCH_BANK_HOLD_TIME,         (void*)&asynch_bank3_hold_time  },


      	{ ADI_EBIU_CMD_END,                     0                   }
	};
	
#endif
/********************************************************************/
#if defined(__ADSP_STIRLING__)		// BF538 EZKit

	ADI_PWR_COMMAND_PAIR ezkit_power[] = { 
    	{ ADI_PWR_CMD_SET_PROC_VARIANT,(void*)ADI_PWR_PROC_BF538BBCZ500  }, // 500Mhz ADSP-BF538
    	{ ADI_PWR_CMD_SET_PACKAGE,     (void*)ADI_PWR_PACKAGE_MBGA       }, // in MBGA packaging, as on all EZ-KITS
    	{ ADI_PWR_CMD_SET_VDDEXT,      (void*)ADI_PWR_VDDEXT_330         }, // external voltage supplied to the voltage regulator is 3.3V
#if defined ADI_SSL_EXT_VOLTAGE_REG
	{ ADI_PWR_CMD_SET_VDDINT,      (void*)ADI_SSL_EXT_VDDINT         },	/* Externally supplied VDDINT (see adi_ssl_Init.h)*/		
#endif
    	{ ADI_PWR_CMD_SET_CLKIN,       (void*)25                         },	// the CLKIN frequency 25 MHz
    	{ ADI_PWR_CMD_END,             0                                 } 
	}; 

	ADI_EBIU_TIMING_VALUE     twrmin       = {1,{7500, ADI_EBIU_TIMING_UNIT_PICOSEC}};   // set min TWR to 1 SCLK cycle + 7.5ns	
	ADI_EBIU_TIMING_VALUE     refresh      = {8192,{64, ADI_EBIU_TIMING_UNIT_MILLISEC}}; // set refresh period to 8192 cycles in 64ms
	ADI_EBIU_TIME             trasmin      = {44, ADI_EBIU_TIMING_UNIT_NANOSEC};         // set min TRAS to 44ns
	ADI_EBIU_TIME             trpmin       = {20, ADI_EBIU_TIMING_UNIT_NANOSEC};	     // set min TRP to 20ns
	ADI_EBIU_TIME             trcdmin      = {20, ADI_EBIU_TIMING_UNIT_NANOSEC}; 	     // set min TRCD to 20ns
	u32                       cl_threshold = 100;                                        // set cl threshold to 100 Mhz
	ADI_EBIU_SDRAM_BANK_VALUE bank_size    = {0, {size: ADI_EBIU_SDRAM_BANK_64MB }};      // set bank size to 64MB
	ADI_EBIU_SDRAM_BANK_VALUE bank_width   = {0, {width: ADI_EBIU_SDRAM_BANK_COL_10BIT}}; // set column address width to 10-Bit

	ADI_EBIU_COMMAND_PAIR ezkit_ram[] = { 
		{ ADI_EBIU_CMD_SET_SDRAM_BANK_SIZE,     (void*)&bank_size   },
       	{ ADI_EBIU_CMD_SET_SDRAM_BANK_COL_WIDTH,(void*)&bank_width  },
       	{ ADI_EBIU_CMD_SET_SDRAM_CL_THRESHOLD,  (void*)cl_threshold },
      	{ ADI_EBIU_CMD_SET_SDRAM_TRASMIN,       (void*)&trasmin     }, 
       	{ ADI_EBIU_CMD_SET_SDRAM_TRPMIN,        (void*)&trpmin      }, 
       	{ ADI_EBIU_CMD_SET_SDRAM_TRCDMIN,       (void*)&trcdmin     }, 
       	{ ADI_EBIU_CMD_SET_SDRAM_TWRMIN,        (void*)&twrmin      },
       	{ ADI_EBIU_CMD_SET_SDRAM_REFRESH,       (void*)&refresh     },
       	/* Asynch Commands memory controller commands */ 
       	{ ADI_EBIU_CMD_SET_ASYNCH_CLKOUT_ENABLE,          (void*)&clkout_enable },
       	{ ADI_EBIU_CMD_SET_ASYNCH_BANK_ENABLE,            (void*)&banks_enable },                                 
       	{ ADI_EBIU_CMD_SET_ASYNCH_BANK_TRANSITION_TIME,   (void*)&asynch_bank_trans_time  },
       	{ ADI_EBIU_CMD_SET_ASYNCH_BANK_READ_ACCESS_TIME,  (void*)&asynch_bank_read_access_time  }, 
       	{ ADI_EBIU_CMD_SET_ASYNCH_BANK_WRITE_ACCESS_TIME, (void*)&asynch_bank_write_access_time  },
       	{ ADI_EBIU_CMD_SET_ASYNCH_BANK_SETUP_TIME,        (void*)&asynch_bank_setup_time  }, 
       	{ ADI_EBIU_CMD_SET_ASYNCH_BANK_HOLD_TIME,         (void*)&asynch_bank_hold_time  },
       	{ ADI_EBIU_CMD_SET_ASYNCH_BANK_ARDY_ENABLE,       (void*)&asynch_bank_ardy_enable  },  
       	{ ADI_EBIU_CMD_SET_ASYNCH_BANK_ARDY_POLARITY,     (void*)&asynch_bank_ardy_polarity },         	
      	{ ADI_EBIU_CMD_END,                     0                   }
	};
	
#endif


/*******************************************************************************/
#if defined(__ADSP_MOAB__)                                 /* BF548 EZKit */

	ADI_PWR_COMMAND_PAIR ezkit_power[] = { 
        
    	{ ADI_PWR_CMD_SET_PROC_VARIANT,(void*)ADI_PWR_PROC_BF548SBBC1533 },     /*  533Mhz ADSP-BF548 */   
    	{ ADI_PWR_CMD_SET_PACKAGE,     (void*)ADI_PWR_PACKAGE_MBGA       },     /* in MBGA packaging, as on all EZ-KITS */      
    	{ ADI_PWR_CMD_SET_VDDEXT,      (void*)ADI_PWR_VDDEXT_330         },     /* external voltage supplied to the voltage regulator is 3.3V */  
#if defined ADI_SSL_EXT_VOLTAGE_REG
	{ ADI_PWR_CMD_SET_VDDINT,      (void*)ADI_SSL_EXT_VDDINT         },	/* Externally supplied VDDINT (see adi_ssl_Init.h)*/		
#endif
    	{ ADI_PWR_CMD_SET_CLKIN,       (void*)25                         },     /*  the CLKIN frequency 25 MHz */       
    	{ ADI_PWR_CMD_END,              0                                }      /* indicates end of table */
	};


	ADI_EBIU_TIMING_VALUE RC =   { 8, {60, ADI_EBIU_TIMING_UNIT_NANOSEC  }};     /* cycles between one active command and the next   */	
	ADI_EBIU_TIMING_VALUE RAS =  { 6, {42, ADI_EBIU_TIMING_UNIT_NANOSEC  }};     /* cycles between active command and precharge command  */       
 	ADI_EBIU_TIMING_VALUE RP =   { 2, {15, ADI_EBIU_TIMING_UNIT_NANOSEC  }};     /* cycles between precharge command and active command   */     
	ADI_EBIU_TIMING_VALUE RFC =  { 10,{72, ADI_EBIU_TIMING_UNIT_NANOSEC  }};     /* cycles for SDRAM to recover from REFRESH signal   */     
	ADI_EBIU_TIMING_VALUE WTR =  { 2, {15, ADI_EBIU_TIMING_UNIT_NANOSEC }};     /* cycles from last write data until next read command   */  
	ADI_EBIU_TIMING_VALUE tWR =  { 2, {15, ADI_EBIU_TIMING_UNIT_NANOSEC  }};     /* write recovery time is 2 or 3 cycles   */
	ADI_EBIU_TIMING_VALUE tMRD = { 2, {15, ADI_EBIU_TIMING_UNIT_NANOSEC  }};     /* cycles from setting of mode   */       
	ADI_EBIU_TIMING_VALUE RCD =  { 2, {15, ADI_EBIU_TIMING_UNIT_NANOSEC  }};     /* cycles from active command to next R/W  */             
	ADI_EBIU_TIMING_VALUE REFI = { 1037,{7777, ADI_EBIU_TIMING_UNIT_NANOSEC}};   /* cycles from one REFRESH signal to the next  */    
 	
		
	ADI_EBIU_COMMAND_PAIR ezkit_ram[] = {           
       	{ ADI_EBIU_CMD_SET_DDR_REFI,    (void*)&REFI   },        /* command to set refresh interval */             
       	{ ADI_EBIU_CMD_SET_DDR_RFC,     (void*)&RFC    },        /* command to set auto refresh period */       
       	{ ADI_EBIU_CMD_SET_DDR_RP,      (void*)&RP     },        /* command to set precharge to active time */          
       	{ ADI_EBIU_CMD_SET_DDR_RAS,     (void*)&RAS    },        /* command to set active to precharge time */       
       	{ ADI_EBIU_CMD_SET_DDR_RC,      (void*)&RC     },        /* command to set active to active time */        
       	{ ADI_EBIU_CMD_SET_DDR_WTR,     (void*)&WTR    },        /* command to set write to read time */                   
       	{ ADI_EBIU_CMD_SET_DDR_DEVICE_SIZE, (void*)0   },        /* command to set size of device */           
       	{ ADI_EBIU_CMD_SET_DDR_CAS,     (void*)2       },        /* command to set cycles from assertion of R/W until first valid data */            
        { ADI_EBIU_CMD_SET_DDR_DEVICE_WIDTH, (void*)2  },        /* command to set width of device */     
       	{ ADI_EBIU_CMD_SET_DDR_EXTERNAL_BANKS,(void*)0 },        /* command to set number of external banks */           
       	{ ADI_EBIU_CMD_SET_DDR_DATA_WIDTH,    (void*)2 },        /* command to set data width */           
       	{ ADI_EBIU_CMD_SET_DDR_WR,      (void*)&tWR    },        /* command to set write recovery time  */                    
       	{ ADI_EBIU_CMD_SET_DDR_MRD,     (void*)&tMRD   },        /* command to set cycles from setting mode reg until next command */          
       	{ ADI_EBIU_CMD_SET_DDR_RCD,     (void*)&RCD    },        /* command to set cycles from active command to a read/write assertion */       
       	/* Asynch Commands memory controller commands */ 
       	{ ADI_EBIU_CMD_SET_ASYNCH_CLKOUT_ENABLE,          (void*)&clkout_enable },
       	{ ADI_EBIU_CMD_SET_ASYNCH_BANK_ENABLE,            (void*)&banks_enable },                                 
       	{ ADI_EBIU_CMD_SET_ASYNCH_BANK_TRANSITION_TIME,   (void*)&asynch_bank_trans_time  },
       	{ ADI_EBIU_CMD_SET_ASYNCH_BANK_READ_ACCESS_TIME,  (void*)&asynch_bank_read_access_time  }, 
       	{ ADI_EBIU_CMD_SET_ASYNCH_BANK_WRITE_ACCESS_TIME, (void*)&asynch_bank_write_access_time  },
       	{ ADI_EBIU_CMD_SET_ASYNCH_BANK_SETUP_TIME,        (void*)&asynch_bank_setup_time  }, 
       	{ ADI_EBIU_CMD_SET_ASYNCH_BANK_HOLD_TIME,         (void*)&asynch_bank_hold_time  },
       	{ ADI_EBIU_CMD_SET_ASYNCH_BANK_ARDY_ENABLE,       (void*)&asynch_bank_ardy_enable  },  
       	{ ADI_EBIU_CMD_SET_ASYNCH_BANK_ARDY_POLARITY,     (void*)&asynch_bank_ardy_polarity },         	
      	{ ADI_EBIU_CMD_END, 0                          }         /* indicate the last command of the table */
	};	
	

	
#endif



/*******************************************************************************/
#if defined(__ADSP_KOOKABURRA__)                               /* BF527 EZKit */


ADI_PWR_COMMAND_PAIR ezkit_power[] = {	


    	{ ADI_PWR_CMD_SET_PROC_VARIANT,(void*)ADI_PWR_PROC_BF527SBBC1600  },    /*  600Mhz ADSP-BF527 */   
    	{ ADI_PWR_CMD_SET_PACKAGE,     (void*)ADI_PWR_PACKAGE_MBGA       },     /* in MBGA packaging, as on all EZ-KITS */      
    	{ ADI_PWR_CMD_SET_VDDEXT,      (void*)ADI_PWR_VDDEXT_330         },     /* external voltage supplied to the voltage regulator is 3.3V */  
#if defined ADI_SSL_EXT_VOLTAGE_REG
	{ ADI_PWR_CMD_SET_VDDINT,      (void*)ADI_SSL_EXT_VDDINT         },	/* Externally supplied VDDINT (see adi_ssl_Init.h)*/		
#endif
    	{ ADI_PWR_CMD_SET_CLKIN,       (void*)25                         },     /* the CLKIN frequency 25 MHz */       
    	{ ADI_PWR_CMD_END,              0                                }      /* indicates end of table */
	};

 

	ADI_EBIU_TIMING_VALUE     twrmin       = {1,{7500, ADI_EBIU_TIMING_UNIT_PICOSEC}};   // set min TWR to 1 SCLK cycle + 7.5ns	
	ADI_EBIU_TIMING_VALUE     refresh      = {8192,{64, ADI_EBIU_TIMING_UNIT_MILLISEC}}; // set refresh period to 8192 cycles in 64ms
	ADI_EBIU_TIME             trasmin      = {44, ADI_EBIU_TIMING_UNIT_NANOSEC};         // set min TRAS to 44ns
	ADI_EBIU_TIME             trpmin       = {20, ADI_EBIU_TIMING_UNIT_NANOSEC};	     // set min TRP to 20ns
	ADI_EBIU_TIME             trcdmin      = {20, ADI_EBIU_TIMING_UNIT_NANOSEC}; 	     // set min TRCD to 20ns
	u32                       cl_threshold = 100;                                        // set cl threshold to 100 Mhz
	ADI_EBIU_SDRAM_BANK_VALUE bank_size    = {0, {size: ADI_EBIU_SDRAM_BANK_64MB }};      // set bank size to 64MB
	ADI_EBIU_SDRAM_BANK_VALUE bank_width   = {0, {width: ADI_EBIU_SDRAM_BANK_COL_10BIT}}; // set column address width to 10-Bit

	ADI_EBIU_COMMAND_PAIR ezkit_ram[] = { 
	{ ADI_EBIU_CMD_SET_SDRAM_BANK_SIZE,     (void*)&bank_size   },
       	{ ADI_EBIU_CMD_SET_SDRAM_BANK_COL_WIDTH,(void*)&bank_width  },
       	{ ADI_EBIU_CMD_SET_SDRAM_CL_THRESHOLD,  (void*)cl_threshold },
      	{ ADI_EBIU_CMD_SET_SDRAM_TRASMIN,       (void*)&trasmin     }, 
       	{ ADI_EBIU_CMD_SET_SDRAM_TRPMIN,        (void*)&trpmin      }, 
       	{ ADI_EBIU_CMD_SET_SDRAM_TRCDMIN,       (void*)&trcdmin     }, 
       	{ ADI_EBIU_CMD_SET_SDRAM_TWRMIN,        (void*)&twrmin      },
       	{ ADI_EBIU_CMD_SET_SDRAM_REFRESH,       (void*)&refresh     },
       	/* Asynch Commands memory controller commands */ 
       	{ ADI_EBIU_CMD_SET_ASYNCH_CLKOUT_ENABLE,          (void*)&clkout_enable },
       	{ ADI_EBIU_CMD_SET_ASYNCH_BANK_ENABLE,            (void*)&banks_enable },                                 
       	{ ADI_EBIU_CMD_SET_ASYNCH_BANK_TRANSITION_TIME,   (void*)&asynch_bank_trans_time  },
       	{ ADI_EBIU_CMD_SET_ASYNCH_BANK_READ_ACCESS_TIME,  (void*)&asynch_bank_read_access_time  }, 
       	{ ADI_EBIU_CMD_SET_ASYNCH_BANK_WRITE_ACCESS_TIME, (void*)&asynch_bank_write_access_time  },
       	{ ADI_EBIU_CMD_SET_ASYNCH_BANK_SETUP_TIME,        (void*)&asynch_bank_setup_time  }, 
       	{ ADI_EBIU_CMD_SET_ASYNCH_BANK_HOLD_TIME,         (void*)&asynch_bank_hold_time  },
       	{ ADI_EBIU_CMD_SET_ASYNCH_BANK_ARDY_ENABLE,       (void*)&asynch_bank_ardy_enable  },  
       	{ ADI_EBIU_CMD_SET_ASYNCH_BANK_ARDY_POLARITY,     (void*)&asynch_bank_ardy_polarity },         	
      	{ ADI_EBIU_CMD_END,                     0                   }
	};
	


		
#endif




/*******************************************************************************/
#if defined(__ADSP_MOCKINGBIRD__)                             /* BF526 EZKit */


ADI_PWR_COMMAND_PAIR ezkit_power[] = {	


    	{ ADI_PWR_CMD_SET_PROC_VARIANT,(void*)ADI_PWR_PROC_BF526SBBC1400  },    /* 400Mhz ADSP-BF526 */   
    	{ ADI_PWR_CMD_SET_PACKAGE,     (void*)ADI_PWR_PACKAGE_MBGA       },     /* in MBGA packaging, as on all EZ-KITS */      
    	{ ADI_PWR_CMD_SET_VDDEXT,      (void*)ADI_PWR_VDDEXT_330         },     /* external voltage supplied to the voltage regulator is 3.3V */  
	{ ADI_PWR_CMD_SET_VDDINT,      (void*)ADI_SSL_EXT_VDDINT         },	/* Externally supplied VDDINT (see adi_ssl_Init.h)*/		
    	{ ADI_PWR_CMD_SET_CLKIN,       (void*)25                         },     /* the CLKIN frequency 25 MHz */       
    	{ ADI_PWR_CMD_END,              0                                }      /* indicates end of table */
	};

 

	ADI_EBIU_TIMING_VALUE     twrmin       = {1,{7500, ADI_EBIU_TIMING_UNIT_PICOSEC}};   // set min TWR to 1 SCLK cycle + 7.5ns	
	ADI_EBIU_TIMING_VALUE     refresh      = {8192,{64, ADI_EBIU_TIMING_UNIT_MILLISEC}}; // set refresh period to 8192 cycles in 64ms
	ADI_EBIU_TIME             trasmin      = {44, ADI_EBIU_TIMING_UNIT_NANOSEC};         // set min TRAS to 44ns
	ADI_EBIU_TIME             trpmin       = {20, ADI_EBIU_TIMING_UNIT_NANOSEC};	     // set min TRP to 20ns
	ADI_EBIU_TIME             trcdmin      = {20, ADI_EBIU_TIMING_UNIT_NANOSEC}; 	     // set min TRCD to 20ns
	u32                       cl_threshold = 100;                                        // set cl threshold to 100 Mhz
	ADI_EBIU_SDRAM_BANK_VALUE bank_size    = {0, {size: ADI_EBIU_SDRAM_BANK_64MB }};      // set bank size to 64MB
	ADI_EBIU_SDRAM_BANK_VALUE bank_width   = {0, {width: ADI_EBIU_SDRAM_BANK_COL_10BIT}}; // set column address width to 10-Bit

	ADI_EBIU_COMMAND_PAIR ezkit_ram[] = { 
	{ ADI_EBIU_CMD_SET_SDRAM_BANK_SIZE,     (void*)&bank_size   },
       	{ ADI_EBIU_CMD_SET_SDRAM_BANK_COL_WIDTH,(void*)&bank_width  },
       	{ ADI_EBIU_CMD_SET_SDRAM_CL_THRESHOLD,  (void*)cl_threshold },
      	{ ADI_EBIU_CMD_SET_SDRAM_TRASMIN,       (void*)&trasmin     }, 
       	{ ADI_EBIU_CMD_SET_SDRAM_TRPMIN,        (void*)&trpmin      }, 
       	{ ADI_EBIU_CMD_SET_SDRAM_TRCDMIN,       (void*)&trcdmin     }, 
       	{ ADI_EBIU_CMD_SET_SDRAM_TWRMIN,        (void*)&twrmin      },
       	{ ADI_EBIU_CMD_SET_SDRAM_REFRESH,       (void*)&refresh     },
       	/* Asynch Commands memory controller commands */ 
       	{ ADI_EBIU_CMD_SET_ASYNCH_CLKOUT_ENABLE,          (void*)&clkout_enable },
       	{ ADI_EBIU_CMD_SET_ASYNCH_BANK_ENABLE,            (void*)&banks_enable },                                 
       	{ ADI_EBIU_CMD_SET_ASYNCH_BANK_TRANSITION_TIME,   (void*)&asynch_bank_trans_time  },
       	{ ADI_EBIU_CMD_SET_ASYNCH_BANK_READ_ACCESS_TIME,  (void*)&asynch_bank_read_access_time  }, 
       	{ ADI_EBIU_CMD_SET_ASYNCH_BANK_WRITE_ACCESS_TIME, (void*)&asynch_bank_write_access_time  },
       	{ ADI_EBIU_CMD_SET_ASYNCH_BANK_SETUP_TIME,        (void*)&asynch_bank_setup_time  }, 
       	{ ADI_EBIU_CMD_SET_ASYNCH_BANK_HOLD_TIME,         (void*)&asynch_bank_hold_time  },
       	{ ADI_EBIU_CMD_SET_ASYNCH_BANK_ARDY_ENABLE,       (void*)&asynch_bank_ardy_enable  },  
       	{ ADI_EBIU_CMD_SET_ASYNCH_BANK_ARDY_POLARITY,     (void*)&asynch_bank_ardy_polarity },         	
      	{ ADI_EBIU_CMD_END,                     0                   }
	};
	


		
#endif

/********************************************************************/

	// initialize everything but exit upon the first error
	do {
	
        // initialize the interrupt manager, parameters are
        //      pointer to memory for interrupt manager to use
        //      memory size (in bytes)
        //      location where the number of secondary handlers that can be supported will be stored
       	//      parameter for adi_int_EnterCriticalRegion (always NULL for VDK and standalone systems)
        if ((Result = adi_int_Init(InterruptServiceData, sizeof(InterruptServiceData), &i, ADI_SSL_ENTER_CRITICAL)) != ADI_INT_RESULT_SUCCESS) {
    	    break;
        }
#if defined(__ADSP_TETON__)	&& defined( ADI_SSL_DUAL_CORE ) // BF561 EZKit
	if( adi_core_id() == 0) 	// init EBIU on Core A only
	{
#endif    
	   //  initialize the EBIU, parameters are
	   //  address of table containing RAM parameters
	   //  0 - reserved field, always 0
	   //  Ignores result, so it can keep going if it's already initialized
	Result = adi_ebiu_Init(ezkit_ram, 0);
	if ((Result != ADI_EBIU_RESULT_SUCCESS) && (Result != ADI_EBIU_RESULT_ALREADY_INITIALIZED)) {
		break;
	}
#if defined(__ADSP_TETON__)	&& defined( ADI_SSL_DUAL_CORE ) // BF561 EZKit
	}
#endif
        // initialize power, parameters are
        //      address of table containing processor information
	    // keep going if it's already initialized
        Result = adi_pwr_Init(ezkit_power);
	    if ((Result != ADI_PWR_RESULT_SUCCESS) && (Result != ADI_PWR_RESULT_ALREADY_INITIALIZED)) {
    	    break;
        }
    
#if defined(__ADSP_BRAEMAR__) || defined(__ADSP_STIRLING__) || defined(__ADSP_MOAB__) || defined(__ADSP_KOOKABURRA__) || defined(__ADSP_MOCKINGBIRD__) 
        // initialize port control, parameters are
        //      parameter for adi_int_EnterCriticalRegion (always NULL for VDK and standalone systems)
        if ((Result = adi_ports_Init(ADI_SSL_ENTER_CRITICAL)) != ADI_PORTS_RESULT_SUCCESS) {
        	break;
        }
#endif        

        // initialize deferred callback service if needed, parameters are
        //      pointer to data
        //      size of data
        //      location where number of servers is stored
        //      parameter for adi_int_EnterCriticalRegion (always NULL for VDK and standalone systems)
#if     (ADI_SSL_DCB_NUM_SERVERS != 0)
            if ((Result = adi_dcb_Init(DeferredCallbackServiceData, sizeof(DeferredCallbackServiceData), &i, ADI_SSL_ENTER_CRITICAL)) != ADI_DCB_RESULT_SUCCESS) {
        	    break;
            }
#endif

   	    // initialize the dma manager if needed, parameters are
   	    //      pointer to memory for the DMA manager to use
   	    //      memory size (in bytes)
   	    //      parameter for adi_int_EnterCriticalRegion (always NULL for VDK and standalone systems)
#if     (ADI_SSL_DMA_NUM_CHANNELS != 0)
	        if ((Result = adi_dma_Init(DMAServiceData, sizeof(DMAServiceData), &i, &adi_dma_ManagerHandle, ADI_SSL_ENTER_CRITICAL)) != ADI_DMA_RESULT_SUCCESS) {
        		break;
    	    }
#endif
	
       	// initialize the flag manager, parameters are
       	//      pointer to memory for the flag service to use
       	//      memory size (in bytes)
       	//      location where the number of flag callbacks that can be supported will be stored
       	//      parameter for adi_int_EnterCriticalRegion (always NULL for VDK and standalone systems)
    	if ((Result = adi_flag_Init(FlagServiceData, sizeof(FlagServiceData), &i, ADI_SSL_ENTER_CRITICAL)) != ADI_FLAG_RESULT_SUCCESS) {
		    break;
	    }
	
   	    // initialize the timer manager, parameters are
   	    //      parameter for adi_int_EnterCriticalRegion (always NULL for VDK and standalone systems)
	    if ((Result = adi_tmr_Init(ADI_SSL_ENTER_CRITICAL)) != ADI_TMR_RESULT_SUCCESS) {
    		break;
    	}
	
#if !defined(ADI_SSL_RTC_NO_INIT) && !defined(__ADSP_TETON__)
        // initialize the RTC service
        //      parameter for adi_int_EnterCriticalRegion (always NULL for VDK and standalone systems)
        if ((Result = adi_rtc_Init(ADI_SSL_ENTER_CRITICAL)) != ADI_RTC_RESULT_SUCCESS) {
        	break;
        }
#endif        

   	    // initialize the semaphore service if needed, parameters are
   	    //      pointer to memory for the semaphore service to use
   	    //      memory size (in bytes)
   	    //      parameter for adi_int_EnterCriticalRegion (always NULL for VDK and standalone systems)
#if     (ADI_SSL_SEM_NUM_SEMAPHORES != 0)
	        if ((Result = adi_sem_Init(SemaphoreServiceData, sizeof(SemaphoreServiceData), &i, ADI_SSL_ENTER_CRITICAL)) != ADI_SEM_RESULT_SUCCESS) {
        		break;
    	    }
#endif
	
    	// initialize the device manager if needed, parameters are
    	//      pointer to data for the device manager to use
    	//      size of the data in bytes
    	//      location where the number of devices that can be managed will be stored
    	//      location where the device manager handle will be stored
    	//      parameter for adi_int_EnterCriticalRegion() function (always NULL for standalone and VDK)
#if     (ADI_SSL_DEV_NUM_DEVICES != 0)
    	    if ((Result = adi_dev_Init(DevMgrData, sizeof(DevMgrData), &i, &adi_dev_ManagerHandle, ADI_SSL_ENTER_CRITICAL)) != ADI_DEV_RESULT_SUCCESS) {
    		    break;
	        }
#endif
	    
	 // WHILE (no errors or 1 pass complete)
	 } while (0);
	
    // return
    return (Result);
    
}



/*********************************************************************

	Function:		adi_ssl_Terminate

	Description:	Terminates the system services and device manager

*********************************************************************/

u32 adi_ssl_Terminate(void) {

    u32 Result;
    
	// terminate everything but exit upon the first error
	do {
	
    	// terminate the device manager if needed
#if     (ADI_SSL_DEV_NUM_DEVICES != 0)
    	    if ((Result = adi_dev_Terminate(adi_dev_ManagerHandle)) != ADI_DEV_RESULT_SUCCESS) {
    		    break;
	        }
#endif
	    
   	    // terminate the semaphore service if needed
#if     (ADI_SSL_SEM_NUM_SEMAPHORES != 0)
	        if ((Result = adi_sem_Terminate()) != ADI_SEM_RESULT_SUCCESS) {
        		break;
    	    }
#endif
	
#if !defined(ADI_SSL_RTC_NO_INIT) && !defined(__ADSP_TETON__)
        // terminate the RTC service
        if ((Result = adi_rtc_Terminate()) != ADI_RTC_RESULT_SUCCESS) {
        	break;
        }
#endif

   	    // terminate the timer manager
	    if ((Result = adi_tmr_Terminate()) != ADI_TMR_RESULT_SUCCESS) {
    		break;
    	}
	
       	// terminate the flag manager
    	if ((Result = adi_flag_Terminate()) != ADI_FLAG_RESULT_SUCCESS) {
		    break;
	    }
	
   	    // terminate the dma manager if needed
#if     (ADI_SSL_DMA_NUM_CHANNELS != 0)
	        if ((Result = adi_dma_Terminate(adi_dma_ManagerHandle)) != ADI_DMA_RESULT_SUCCESS) {
        		break;
    	    }
#endif
	
        // terminate the deferred callback service if needed
#if     (ADI_SSL_DCB_NUM_SERVERS != 0)
            if ((Result = adi_dcb_Terminate()) != ADI_DCB_RESULT_SUCCESS) {
        	    break;
            }
#endif

#if defined(__ADSP_BRAEMAR__) || defined(__ADSP_STIRLING__) || defined(__ADSP_MOAB__) || defined(__ADSP_KOOKABURRA__) || defined(__ADSP_MOCKINGBIRD__) 
        // terminate port control
        if ((Result = adi_ports_Terminate()) != ADI_PORTS_RESULT_SUCCESS) {
        	break;
        }
#endif

        // terminate power
        if ((Result = adi_pwr_Terminate()) != ADI_PWR_RESULT_SUCCESS) {
    	    break;
        }
#if defined(__ADSP_TETON__)	&& defined( ADI_SSL_DUAL_CORE ) // BF561 EZKit
	if( adi_core_id() == 0) 	// init EBIU on Core A only
	{
#endif       
	    // terminate the EBIU
	    if ((Result = adi_ebiu_Terminate()) != ADI_EBIU_RESULT_SUCCESS) {
    		break;
    	}
#if defined(__ADSP_TETON__)	&& defined( ADI_SSL_DUAL_CORE ) // BF561 EZKit
	}
#endif       	
	
        // terminate the interrupt manager
        if ((Result = adi_int_Terminate()) != ADI_INT_RESULT_SUCCESS) {
    	    break;
        }
    
	 // WHILE (no errors or 1 pass complete)
	 } while (0);
	
    // return
    return (Result);
    
}

