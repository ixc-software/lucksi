#include "stdafx.h"
#include "MainThread.h"
#include <new>

#include "MainBody.h"

#pragma file_attr("OS_Component=Threads")
#pragma file_attr("Threads")


//-----------------------------------------------------------------------------

 
void MainThread::Run()
{
    try
    {
 	    MainBody();
    }
    catch (std::exception &e)
    {
        ESS_UNEXPECTED_EXCEPTION(e);
    }
}


//-----------------------------------------------------------------------------

 
int MainThread::ErrorHandler()
{
    /* TODO - Put this thread's error handling code HERE */

    /* The default ErrorHandler (called below)  raises
     * a kernel panic and stops the system */
    return (VDK::Thread::ErrorHandler());
}


//-----------------------------------------------------------------------------

 
MainThread::MainThread(VDK::Thread::ThreadCreationBlock &tcb)
    : VDK::Thread(tcb)
{
    /* This routine does NOT run in new thread's context.  Any non-static thread
     *   initialization should be performed at the beginning of "Run()."
     */

    // TODO - Put code to be executed when this thread has just been created HERE

}


//-----------------------------------------------------------------------------

 
MainThread::~MainThread()
{
    /* This routine does NOT run in the thread's context.  Any VDK API calls
     *   should be performed at the end of "Run()."
     */

    // TODO - Put code to be executed just before this thread is destroyed HERE

}


//-----------------------------------------------------------------------------

 
VDK::Thread* MainThread::Create(VDK::Thread::ThreadCreationBlock &tcb)
{
    /* This routine does NOT run in new thread's context.  Any non-static thread
     *   initialization should be performed at the beginning of "Run()."
     */

    	return new (tcb) MainThread(tcb);
}
