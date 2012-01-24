//
// sys_arch_thread.h
//
// Contains definition of the sys_arch_thread struct which the VDK porting
// layer uses to describe each thread that lwIP creates.
//
// Member 'timeouts' is void* rather than struct timeouts to avoid importing
// lwIP include files into lwIP_ThreadType.[ch].
//
// The lwIP_ThreadType ErrorFunction() sets the 'error_cause' and 'error_value'
// members before returning control to the point just after the failing
// VDK API call. That's how timed-out semaphores are detected.
//
// Note that lwIP_ThreadType's Destroy() function sets the 'id' member to 
// UINT_MAX just before it exits to mark the structure as free.
//
#include <kernel_abs.h>
#ifndef _SYS_ARCH_THREAD_H_
#define _SYS_ARCH_THREAD_H_
//#include "ADI_TOOLS_IThreadedOE.h"
struct sys_arch_thread {
  struct sys_arch_thread* next;         // our linked list of lwIP threads
  void*                   timeouts;     // list of timeouts for this thread
  ker_thread_id_t id;
  int         error_cause;  // VDK error indicator
  int                     error_value;  // VDK error value
  void (*function)(void* arg);          // lwIP entrypoint for this thread
  void* arg;                            // argument for lwIP entrypoint
};
#endif // _SYS_ARCH_THREAD_H_
