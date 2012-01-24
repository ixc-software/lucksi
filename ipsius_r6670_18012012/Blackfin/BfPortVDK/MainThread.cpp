#include "stdafx.h"
#include "MainThread.h"
#include <new>

#include "MainBody.h"

#pragma file_attr("OS_Component=Threads")
#pragma file_attr("Threads")


// ----------------------------------------------------------

extern "C" void KernelPanic(VDK::PanicCode code, VDK::SystemError err, const int value)
{
    int i = 0;
	
	if (err == VDK::kNewFailure)
	{
		i++;
	}
	while(1);
}

// ----------------------------------------------------------
 
void MainThread::Run()
{
 	MainBody();
}

// ----------------------------------------------------------

 
int MainThread::ErrorHandler()
{
	int i = 0;
	
    return (VDK::Thread::ErrorHandler());
}

/******************************************************************************
 *  MainThread Constructor
 */
 
MainThread::MainThread(VDK::Thread::ThreadCreationBlock &tcb)
    : VDK::Thread(tcb)
{
    /* This routine does NOT run in new thread's context.  Any non-static thread
     *   initialization should be performed at the beginning of "Run()."
     */

    // TODO - Put code to be executed when this thread has just been created HERE

}

/******************************************************************************
 *  MainThread Destructor
 */
 
MainThread::~MainThread()
{
    /* This routine does NOT run in the thread's context.  Any VDK API calls
     *   should be performed at the end of "Run()."
     */

    // TODO - Put code to be executed just before this thread is destroyed HERE

}

/******************************************************************************
 *  MainThread Externally Accessible, Pre-Constructor Create Function
 */
 
VDK::Thread*
MainThread::Create(VDK::Thread::ThreadCreationBlock &tcb)
{
    /* This routine does NOT run in new thread's context.  Any non-static thread
     *   initialization should be performed at the beginning of "Run()."
     */

    	return new (tcb) MainThread(tcb);
}

/* ========================================================================== */
