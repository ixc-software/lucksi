/*********************************************************************************

Copyright(c) 2005 Analog Devices, Inc. All Rights Reserved. 

This software is proprietary and confidential.  By using this software you agree
to the terms of the associated Analog Devices License Agreement.  
			
$RCSfile: adi_ssl_init.h,v $
$Revision: 1.1.2.3 $

*********************************************************************************/

/*********************************************************************

Warn user against using this file in an incompatible VisualDSP release

*********************************************************************/

#if __VISUALDSPVERSION__ < 0x05000100 
#warning ** This version of adi_ssl_Init() is only for VisualDSP 5.0 Update 1 or Later
#endif 


/*********************************************************************

Include files

*********************************************************************/
#include <services/services.h>		// system service includes
#include <drivers/adi_dev.h>        // device driver includes


/*********************************************************************

Definitions/Sizings

The user should modify the values in parenthesis as needed by their
application.

*********************************************************************/

#define ADI_SSL_INT_NUM_SECONDARY_HANDLERS  (8) // number of secondary interrupt handlers
#define ADI_SSL_DCB_NUM_SERVERS             (0) // number of DCB servers
#define ADI_SSL_DMA_NUM_CHANNELS            (0) // number of DMA channels
#define ADI_SSL_FLAG_NUM_CALLBACKS          (0) // number of flag callbacks
#define ADI_SSL_SEM_NUM_SEMAPHORES          (0) // number of semaphores

#define ADI_SSL_DEV_NUM_DEVICES             (2) // number of device drivers


/***********************************************************************

 PLEASE READ THE FOLLOWING SECTIONS CAREFULLY.  If you do not define the following macros 
 according to your hardware, your applications may not work properly.

***********************************************************************/

/***********************************************************************
  By default the Real Time Clock is supported.
  If the application does not require the Real Time Clock uncomment the following.
***********************************************************************/

#define ADI_SSL_RTC_NO_INIT

/***********************************************************************

 IMPORTANT for ADSP-BF561 applications!!  If building a dual core application for ADSP-BF561 
 which uses the SSL power management module on BOTH cores, then uncomment the following.  
 If core B only idles and does not call 'adi_pwr_Init', then leave this line commented out.

***********************************************************************/

// #define ADI_SSL_DUAL_CORE                   


/***********************************************************************

 IMPORTANT for ADSP-BF561 applications, for ADSP-BF522/4/6 applications, and any other
 processors which use an external Voltage Regulator.
 
 *** EXTERNAL VOLTAGE REGULATOR SUPPORT ***

 There is no internal voltage regulator on ADSP-BF522/4/6.  The external voltage regulator 
 must be used on the on ADSP-BF526 EZ-Kit.  'adi_ssl_Init()' will pass the command 
 ADI_PWR_CMD_SET_VDDINT to 'adi_pwr_Init' to inform the power management service of the 
 externally supplied internal voltage level.  For the ADSP-BF526 it is passed automatically, 
 since there is no internal voltage regulator.

 Existing ADSP-BF526 EZ-Kit has external supply of 1.4V.  Future kits have a programmable 
 voltage regulator.  The definition of ADI_SSL_EXT_VDDINT is set here to 1.4V.

 Modify the value of ADI_SSL_EXT_VDDINT as needed.  See enumeration ADI_PWR_VLEV in 'adi_pwr.h'.

 If using an external voltage regulator on ADSP-BF561, (or another Blackfin which supports 
 external VR)uncomment the line #define ADI_SSL_EXT_VOLTAGE_REG to pass the command 
 ADI_PWR_CMD_SET_VDDINT to 'adi_pwr_Init'.  

 A default of 1.3V is defined to ensure that some in-bounds value is chosen.

***********************************************************************/
#if defined(__ADSP_MOCKINGBIRD__)                     /* BF526 EZKit */
#define ADI_SSL_EXT_VDDINT ADI_PWR_VLEV_140           /* modify as needed, if using programmable external VR */
#else
//#define ADI_SSL_EXT_VOLTAGE_REG                     /* uncomment if using external VR */
#define ADI_SSL_EXT_VDDINT ADI_PWR_VLEV_130           /* modify as needed - see ADI_PWR_VLEV in adi_pwr.h */
#endif

/***********************************************************************

 

***********************************************************************/



/***********************************************************************

 IMPORTANT for ADSP-BF533 applications!!  If using the ADSP-BF533 EZ-Kit Rev 1.7 or newer, 
 SDRAM can be configured for 64MB, 10-bit address width, with the definition below: ADI_SSL_EDIN_REV_1_7.

 Beginning with the VisualDSP 5.0 Update 1, this is defined by default.

 In previous VisualDSP releases, it is commented out by default for backward compatibility.

 Explanation: If developing with the VisualDSP IDDE, and "Use XML reset values" is selected in 
 Settings->Target Options, prior to the program being loaded, SDRAM will be configured according 
 to the values in "ADSP-BF533-proc.xml" found in the VisualDSP installation folder: ".../System/ArchDef".  

 The XML reset values should agree with this definition.

 The XML file can be edited or VisualDSP5.0 custom board support can be used to create a custom XML file.  

 If booting the application from flash, be sure that the SDRAM initialization code in the boot stream agrees 
 with the SDRAM configuration in the application.  

 If using a Rev 1.6 EZ-Kit or older, comment out the following line. 

***********************************************************************/
 
#define ADI_SSL_EDIN_REV_1_7                

/***********************************************************************

 CAUTION!! for ADSP-BF533 applications.  ADSP-BF533 part number ending in "6V" have different power management 
 requirements from other part numbers.  If you DO NOT have one of these parts, comment out the following macro,
 which is defined by default. If you DO have one of these parts, leave the following line as it is.

***********************************************************************/
 
#define ADI_SSL_BF533_6V 

/***********************************************************************

Handles to Services

Declare the handles to the various services and device manager.  These
handles can be used after the adi_ssl_Init() call.  

DO NOT MODIFY THIS SECTION

*********************************************************************/

extern ADI_DMA_MANAGER_HANDLE adi_dma_ManagerHandle;   // handle to the DMA manager
extern ADI_DEV_MANAGER_HANDLE adi_dev_ManagerHandle;   // handle to the device manager


/*********************************************************************

Critical Region Parameter

This is the value that is pased to the adi_int_EnterCriticalRegion()
function call.  This value is NULL for all current implementations.

DO NOT MODIFY THIS SECTION

*********************************************************************/

#define ADI_SSL_ENTER_CRITICAL (NULL)




/*********************************************************************

Function Declarations

Declare the intialization and termination functions.

DO NOT MODIFY THIS SECTION

*********************************************************************/

#if defined __cplusplus
    extern "C" {
#endif


u32 adi_ssl_Init(void);
u32 adi_ssl_Terminate(void);


#if defined __cplusplus
    }
#endif



