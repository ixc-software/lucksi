#include "arch/errno.h"
#include <kernel_abs.h>

#ifdef _DEBUG_
#include <stdio.h>
#define DEBUG_ASSERT(_str,_val) (( (_val) ? (printf("%s\n",_str)) : 0))
#else
#define DEBUG_ASSERT
#endif

int GetSocketError()
{
#ifdef ERRNO
	// single threaded environments
	return errno;
#else
	// mutlithreaded environments
	int last_error;

	last_error =  ker_get_lasterror();
	return last_error;
#endif /* ERRNO */
}

#ifndef ERRNO /* NOT ERRNO */
// For multithreaded environments error is set per thread.
void set_thread_sock_err(int err)
{
	ker_set_lasterror(err);
}
#endif
