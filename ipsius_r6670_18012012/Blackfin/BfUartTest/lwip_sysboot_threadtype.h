/* =============================================================================
 *
 *  Description: This is a C++ to C Thread Header file for Thread lwip_sysboot_threadtype
 *
 * -----------------------------------------------------------------------------
 *  Comments:
 *
 * ===========================================================================*/

#ifndef _lwip_sysboot_threadtype_H_
#define _lwip_sysboot_threadtype_H_

#include "VDK.h"
#include <services/services.h>
#include <drivers/adi_dev.h>
#include <lwip/cglobals.h>

#ifdef __ECC__	/* for C/C++ access */
#ifdef __cplusplus
extern "C" void lwip_sysboot_threadtype_InitFunction(void**, VDK::Thread::ThreadCreationBlock *);
#else
extern "C" void lwip_sysboot_threadtype_InitFunction(void**, VDK_ThreadCreationBlock *);
#endif
extern "C" void lwip_sysboot_threadtype_DestroyFunction(void**);
extern "C" int  lwip_sysboot_threadtype_ErrorFunction(void**);
extern "C" void lwip_sysboot_threadtype_RunFunction(void**);
#endif /* __ECC__ */

#ifdef __cplusplus

	extern "C"   // all functions return bool
	{
		int Lw_Init(net_config_info *pCfg, /* out */ ADI_DEV_DEVICE_HANDLE *pLanHandle);
		
		int Lw_StartMac(ADI_DEV_DEVICE_HANDLE lan_handle);
		int Lw_Established(ADI_DEV_DEVICE_HANDLE lan_handle);
		
		int Lw_StartStack();
		
		int Lw_GetHostAddr(void *pBuff, int buffSize);	
        int Lw_GetMAC(char *pBuff, int buffSize);  // copy MAC to chart[6] buffer
	}

#endif

#ifdef __cplusplus
#include <new>

class lwip_sysboot_threadtype_Wrapper : public VDK::Thread
{
public:
    lwip_sysboot_threadtype_Wrapper(VDK::ThreadCreationBlock &t)
        : VDK::Thread(t)
    { lwip_sysboot_threadtype_InitFunction(&m_DataPtr, &t); }

    ~lwip_sysboot_threadtype_Wrapper()
    { lwip_sysboot_threadtype_DestroyFunction(&m_DataPtr); }

    int ErrorHandler()
    { 
      return lwip_sysboot_threadtype_ErrorFunction(&m_DataPtr);
     }

    void Run()
    { lwip_sysboot_threadtype_RunFunction(&m_DataPtr); }

    static VDK::Thread* Create(VDK::Thread::ThreadCreationBlock &t)
    { return new (t) lwip_sysboot_threadtype_Wrapper(t); }
};

#endif /* __cplusplus */
#endif /* _lwip_sysboot_threadtype_H_ */

/* ========================================================================== */
