/* =============================================================================
 *
 *  Description: This is a C++ to C Thread Header file for Thread RawCThread
 *
 * -----------------------------------------------------------------------------
 *  Comments:
 *
 * ===========================================================================*/

#ifndef _RawCThread_H_
#define _RawCThread_H_

#ifndef _MISRA_RULES
#pragma diag(push)
#pragma diag(suppress: 177,401,451,826,831,1462)
#endif

#include "VDK.h"
#ifndef _MISRA_RULES
#pragma diag(pop)
#endif

#ifdef __ECC__	/* for C/C++ access */
#ifdef __cplusplus
extern "C" void RawCThread_InitFunction(void**, VDK::Thread::ThreadCreationBlock const *);
#else
extern "C" void RawCThread_InitFunction(void** inPtr, VDK_ThreadCreationBlock const * pTCB);
#endif
extern "C" void RawCThread_DestroyFunction(void** inPtr);
extern "C" int  RawCThread_ErrorFunction(void** inPtr);
extern "C" void RawCThread_RunFunction(void** inPtr);
#endif /* __ECC__ */

#ifdef __cplusplus
#include <new>

class RawCThread_Wrapper : public VDK::Thread
{
public:
    RawCThread_Wrapper(VDK::ThreadCreationBlock &t)
        : VDK::Thread(t)
    { RawCThread_InitFunction(&m_DataPtr, &t); }

    ~RawCThread_Wrapper()
    { RawCThread_DestroyFunction(&m_DataPtr); }

    int ErrorHandler()
    { 
      return RawCThread_ErrorFunction(&m_DataPtr);
     }

    void Run()
    { RawCThread_RunFunction(&m_DataPtr); }

    static VDK::Thread* Create(VDK::Thread::ThreadCreationBlock &t)
    { return new (t) RawCThread_Wrapper(t); }
};

#endif /* __cplusplus */
#endif /* _RawCThread_H_ */

/* ========================================================================== */
