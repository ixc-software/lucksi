/* =============================================================================
 *
 *  Description: This is a C++ implementation for Thread base
 *
 * -----------------------------------------------------------------------------
 *  Comments:
 *
 * ===========================================================================*/
#include "stdafx.h"
#include <new>
#include "base.h"

#include "MainBody.h"

#pragma file_attr("OS_Component=Threads")
#pragma file_attr("Threads")


// ----------------------------------------------------------
/*
extern "C" void KernelPanic(VDK::PanicCode code, VDK::SystemError err, const int value)
{
    int i = 0;
	
	if (err == VDK::kNewFailure)
	{
		i++;
	}
	while(1);
}
*/
// ----------------------------------------------------------    



/******************************************************************************
 *  base Run Function (base's main{})
 */

void
base::Run()
{      
	       
    MainBody();
    
}

/******************************************************************************
 *  base Error Handler
 */
 
int
base::ErrorHandler()
{
    /* TODO - Put this thread's error handling code HERE */

    /* The default ErrorHandler (called below)  raises
     * a kernel panic and stops the system */
    return (VDK::Thread::ErrorHandler());
}

/******************************************************************************
 *  base Constructor
 */
 
base::base(VDK::Thread::ThreadCreationBlock &tcb)
    : VDK::Thread(tcb)
{
   
    /* This routine does NOT run in new thread's context.  Any non-static thread
     *   initialization should be performed at the beginning of "Run()."
     */

    // TODO - Put code to be executed when this thread has just been created HERE
}

/******************************************************************************
 *  base Destructor
 */
 
base::~base()
{
    /* This routine does NOT run in the thread's context.  Any VDK API calls
     *   should be performed at the end of "Run()."
     */

    // TODO - Put code to be executed just before this thread is destroyed HERE

}

/******************************************************************************
 *  base Externally Accessible, Pre-Constructor Create Function
 */
 
VDK::Thread*
base::Create(VDK::Thread::ThreadCreationBlock &tcb)
{
    
    /* This routine does NOT run in new thread's context.  Any non-static thread
     *   initialization should be performed at the beginning of "Run()."
     */

    	return new (tcb) base(tcb);
}

/* ========================================================================== */
