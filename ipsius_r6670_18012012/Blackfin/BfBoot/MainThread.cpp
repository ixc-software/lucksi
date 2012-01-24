/* =============================================================================
 *
 *  Description: This is a C++ implementation for Thread MainThread
 *
 * -----------------------------------------------------------------------------
 *  Comments:
 *
 * ===========================================================================*/

#include "stdafx.h"
#include "Utils/ErrorsSubsystem.h"
#include "MainBody.h"

 
#include "MainThread.h"
#include <new>

#pragma file_attr("OS_Component=Threads")
#pragma file_attr("Threads")

// ---------------------------------------------------------------
 
void MainThread::Run()
{
	try
	{
		MainBody();		
	}
	catch(std::exception &e)
    {
    	ESS_UNEXPECTED_EXCEPTION(e);
    }
    catch(...)
    {
    	std::cout << "Unhandled exception!" << std::endl;
    }
    
    
	ESS_HALT("MainThread::Run()");
	
    while (true);
}

// ---------------------------------------------------------------

int MainThread::ErrorHandler()
{
    return (VDK::Thread::ErrorHandler());
}

// ---------------------------------------------------------------
 
MainThread::MainThread(VDK::Thread::ThreadCreationBlock &tcb)
    : VDK::Thread(tcb)
{
}

// ---------------------------------------------------------------

MainThread::~MainThread()
{
}

// ---------------------------------------------------------------
 
VDK::Thread* MainThread::Create(VDK::Thread::ThreadCreationBlock &tcb)
{
	return new (tcb) MainThread(tcb);
}
