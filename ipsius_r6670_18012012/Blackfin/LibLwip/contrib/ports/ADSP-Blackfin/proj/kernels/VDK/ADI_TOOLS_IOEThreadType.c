/* =============================================================================
 *
 *  Description: This is a C implementation for Thread ADI_TOOLS_IOEThreadType
 *
 * -----------------------------------------------------------------------------
 *  Comments:
 *
 * ===========================================================================*/

/* Get access to any of the VDK features & datatypes used */

#include <stdlib.h>

#include "ADI_TOOLS_IOEThreadType.h"
#include "kernel_abs.h"

void
ADI_TOOLS_IOEThreadType_RunFunction(void **inPtr)
{
   struct ThreadCtrlBlock t_block;
   t_block.thread_function = ((struct ThreadCtrlBlock*)*inPtr)->thread_function;
   t_block.arg = ((struct ThreadCtrlBlock*)*inPtr)->arg;
   free((struct ThreadCtrlBlock*)*inPtr);
   t_block.thread_function(t_block.arg);
   return;
}

int
ADI_TOOLS_IOEThreadType_ErrorFunction(void **inPtr)
{

    /* TODO - Put this thread's error handling code HERE */

      /* The default ErrorHandler kills the thread */

   /* Destroy thread was commented out, by default VDK terminates the
    * current thread upon error
    */
   //VDK_DestroyThread(VDK_GetThreadID(), false);
   return 0;
}

void
ADI_TOOLS_IOEThreadType_InitFunction(void **inPtr, VDK_ThreadCreationBlock *pTCB)
{
    /* Put code to be executed when this thread has just been created HERE */

    /* This routine does NOT run in new thread's context.  Any non-static thread
     *   initialization should be performed at the beginning of "Run()."
     */
}

void
ADI_TOOLS_IOEThreadType_DestroyFunction(void **inPtr)
{
    /* Put code to be executed when this thread is destroyed HERE */

    /* This routine does NOT run in the thread's context.  Any VDK API calls
     *   should be performed at the end of "Run()."
     */
}

/* ========================================================================== */
