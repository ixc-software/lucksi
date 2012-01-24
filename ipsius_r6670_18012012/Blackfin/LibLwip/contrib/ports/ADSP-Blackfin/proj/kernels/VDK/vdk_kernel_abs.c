/*****************************************************************************

Copyright(c) 2004 Analog Devices, Inc. All Rights Reserved.

This software is proprietary and confidential.  By using this software you agree
to the terms of the associated Analog Devices License Agreement.

******************************************************************************/
#define _USE_SSL_LIB_

#include <stdlib.h>

#include "VDK.h"
#include "kernel_abs.h"




#define MASK_BITS      (short)~0

typedef struct _os_data
{
	unsigned short oe_imask;
	float         tick_period;
	int	       error_slot;
	void*   	aux_data;
}os_data;

static os_data g_os_data={0};


/*##########################################################################
 *
 *  ker_init, initializes the kernel, global data.
 *
 ##########################################################################*/
int ker_init(void *args)
{
	g_os_data.tick_period = VDK_GetTickPeriod();
	g_os_data.error_slot  = VDK_kTLSUnallocated;

	if(!VDK_AllocateThreadSlot((int*)&g_os_data.error_slot))
		return ker_err_panic;
	return 0;
}


/*##########################################################################
 *
 *  auxilary data is additional data required by the os to initialize or
 *  for any other purposes supplied by the environment. In VDK this is used
 *  to setup the thread template id for the lwip threads.
 *
 ##########################################################################*/
void ker_set_auxdata(void* in_aux_data)
{
	g_os_data.aux_data = in_aux_data;
}

/*##########################################################################
 *
 * Get already set auxilary data
 *
 ##########################################################################*/
void* ker_get_auxdata()
{
	return (void*)g_os_data.aux_data;
}

/*##########################################################################
 *
 *  Get a semaphore.Clears any pending thread errors.
 *
 ##########################################################################*/
int ker_get_semaphore(unsigned int initial_count,
					  unsigned int max_count)
{
	ker_sem_id_t sem_id=(ker_sem_id_t)-1;
	VDK_ClearThreadError();
	sem_id = (ker_sem_id_t)VDK_CreateSemaphore(initial_count,max_count,0,0);
	if(sem_id != UINT_MAX)  return sem_id;

	return ker_err_sem_cre;
}

/*##########################################################################
 *
 *  Releases the semaphore and destroys them.
 *
 ##########################################################################*/
int ker_rel_semaphore(ker_sem_id_t sem_id)
{
	VDK_DestroySemaphore((VDK_SemaphoreID)sem_id);
	if(VDK_GetLastThreadError() == VDK_kUnknownSemaphore)
         return ker_err_sem_id;
	return 0;
}

/*##########################################################################
 *
 * Pend on a semaphore. The calling thread will be blocked if the semaphore
 * is not available.
 *
 ##########################################################################*/
int ker_pend_semaphore(ker_sem_id_t sem_id, unsigned int timeout)
{
unsigned int numTicks=0;

	if(timeout != 0)
		numTicks = (unsigned int)(timeout / g_os_data.tick_period) | VDK_kNoTimeoutError;

    VDK_ClearThreadError();
	if (!VDK_PendSemaphore((VDK_SemaphoreID)(sem_id),numTicks))
			return ker_err_sem_timeout;

	if(VDK_GetLastThreadError() == VDK_kUnknownSemaphore)
		return ker_err_sem_id;

      return 0;
}

/*##########################################################################
 *
 * Post or signal the semaphore, this will make any threads pending on this
 * semaphore ready.
 *
 ##########################################################################*/
int ker_post_semaphore(ker_sem_id_t sem_id)
{
      VDK_ClearThreadError();
      VDK_PostSemaphore((VDK_SemaphoreID)sem_id);

      if(VDK_GetLastThreadError() == VDK_kUnknownSemaphore)
            return(ker_err_sem_id);

	  return 0;
}

/*##########################################################################
 *
 * Post the semaphore from the interrupt domain. This one and ker_post_semaphore
 * may be same for some operating systems.
 *
 ##########################################################################*/
int ker_isr_post_semaphore(ker_sem_id_t sem_id)
{
      VDK_C_ISR_PostSemaphore((VDK_SemaphoreID)sem_id);
      return 0;
}
/*##########################################################################
 *
 * Disable interrupts. This is done via setting up the IMASK using VDK calls.
 *
 ##########################################################################*/
int ker_disable_interrupts(int pri_level)
{

#if !defined(_USE_SSL_LIB_)
short  cur_imask=0;

    // validate the input priorityLevel
	if( (((pri_level > ker_kPriorityLevel4) &&
	 (pri_level <= ker_kPriorityLevel15)) ||
	 (pri_level == ker_kPriorityLevelAll)))
	{
	// Get existing Interrupt mask
	cur_imask  = VDK_GetInterruptMask();

	// save off current imask in to a global
	g_os_data.oe_imask |= cur_imask;

	// Clear off all interrupt mask
	VDK_ClearInterruptMaskBits((unsigned int)MASK_BITS);

	// Mask off all lower/equal priority interrupts than the passed on
	// priorityLevel.
	if(pri_level != ker_kPriorityLevelAll)
		cur_imask &= ~(MASK_BITS << pri_level);
	else
		cur_imask &= 0;

	// Set the new priority mask
	VDK_SetInterruptMaskBits(cur_imask);

	}
	else
	 return (ker_err_int_priority);
#else
	VDK_PushCriticalRegion();
	return 1;
#endif /* _USE_SSL_LIB_ */
}

/*##########################################################################
 *
 * Enable interrupts.  Restores back the previous IMASK
 *
 ##########################################################################*/
int ker_enable_interrupts(int pri_level)
{
#if !defined(_USE_SSL_LIB_)
  short  cur_imask=0,temp_mask=0;

 // validate the input priorityLevel
 //
  if( (((pri_level > ker_kPriorityLevel4) &&
	 (pri_level <= ker_kPriorityLevel15)) ||
	 (pri_level == ker_kPriorityLevelAll)))

  {
	// Get existing Interrupt mask
	cur_imask = VDK_GetInterruptMask();

	// Clear off all interrupt mask
	VDK_ClearInterruptMaskBits((unsigned int)MASK_BITS);

	temp_mask = cur_imask;
	// get the saved mask
	cur_imask  |= g_os_data.oe_imask;

	// Enable interrupts for the given priority level or higher
	if(pri_level != ker_kPriorityLevelAll)
		cur_imask = ((cur_imask & (MASK_BITS << pri_level) | ( temp_mask & ~(MASK_BITS << pri_level))));
	else
		cur_imask &= MASK_BITS;

	// Set the new mask value
	VDK_SetInterruptMaskBits((unsigned int)cur_imask);

	// Reset the enabled bits in the imask
	g_os_data.oe_imask &= ~(MASK_BITS << pri_level);

}
else
	return ker_err_int_priority;
#else
	VDK_PopCriticalRegion();
	return 1;
#endif /* _USE_SSL_LIB_ */
}

/*##########################################################################
 *
 * Sleep for specified number of milli-seconds.
 * TODO: Optimize by avioding the division on Blackfin
 *
 ##########################################################################*/
int ker_sleep(unsigned int sleep_time)
{
    unsigned int numTicks=0;
    numTicks = sleep_time / g_os_data.tick_period;

    VDK_Sleep(numTicks);
      return (0);
}

/*##########################################################################
 *
 * Set the last error for the thread.
 *
 ##########################################################################*/
int ker_set_lasterror(int last_error)
{
     if(VDK_SetThreadSlotValue(g_os_data.error_slot,(void*)last_error))
		 return 0;
	 else
		 return ker_err_1;
}

/*##########################################################################
 *
 * Get the last error for the thread.
 *
 ##########################################################################*/
int ker_get_lasterror(void)
{
	return (int)VDK_GetThreadSlotValue(g_os_data.error_slot);
}

/*##########################################################################
 *
 * Gets the system time in milli-seconds.
 *
 ##########################################################################*/
int ker_get_systime(void)
{
      return ((int)VDK_GetUptime() * g_os_data.tick_period);
}

/*##########################################################################
 *
 * Creates a thread using the ADI_TOOLS_IThreadOE template. caller can set
 * the thread priority and stack.If they are 0 then the default priority and
 * stack size of the template is used.
 *
 ##########################################################################*/
int ker_create_thread(void *th_entry_point, void *th_args, int th_stack_sz, void* th_stack_area, int th_priority, char th_name[], ker_thread_id_t *th_id)
{
static VDK_Priority VDK_priorities[32] = {
        VDK_kPriority1,  VDK_kPriority1,  VDK_kPriority2,  VDK_kPriority3,
        VDK_kPriority4,  VDK_kPriority5,  VDK_kPriority6,  VDK_kPriority7,
        VDK_kPriority8,  VDK_kPriority9,  VDK_kPriority10, VDK_kPriority11,
        VDK_kPriority12, VDK_kPriority13, VDK_kPriority14, VDK_kPriority15,
        VDK_kPriority16, VDK_kPriority17, VDK_kPriority18, VDK_kPriority19,
        VDK_kPriority20, VDK_kPriority21, VDK_kPriority22, VDK_kPriority23,
        VDK_kPriority24, VDK_kPriority25, VDK_kPriority26, VDK_kPriority27,
        VDK_kPriority28, VDK_kPriority29, VDK_kPriority30, VDK_kPriority30
      };
      // This call assumes to have a generic VDK thread template defined in
      // VDK UI configuration. As VDk configuration is driven by the UI
      // Applicaton users that are running under VDK and creating threads
      // using this mechanism to create thread in VDK environment, must create
      // Generic_VDK_ThreadType, without creating the sources.

      VDK_ThreadCreationBlock tcb;

      static int slotNo=0;
      struct ThreadCtrlBlock *cur_thread = (struct ThreadCtrlBlock *)malloc(sizeof(struct ThreadCtrlBlock));;


      if (cur_thread == NULL) {
      	  *th_id = UINT_MAX;   // thread creation failed
      } else {
	      cur_thread->thread_function =(ThreadEntryPoint)th_entry_point;
	      cur_thread->arg = th_args;

	      tcb.template_id     = (enum ThreadType)ker_get_auxdata();
	      tcb.thread_id       = (VDK_ThreadID)0;
	      tcb.thread_stack_size = th_stack_sz;
	      tcb.user_data_ptr     = cur_thread;
	      tcb.thread_priority   = VDK_priorities[th_priority];

	      *th_id = VDK_CreateThreadEx(&tcb);
      }

      if(*th_id != UINT_MAX)
	  {
         tcb.thread_id = (VDK_ThreadID)*th_id;
	  	 return 0;
	  }

      return ker_err_thread_cre;
}

/*##########################################################################
 *
 *  Kills the thread.
 *
 ##########################################################################*/
int ker_destroy_thread(ker_thread_id_t th_id)
{
      VDK_DestroyThread((VDK_ThreadID)th_id,1);
	  return 0;
}

/*##########################################################################
 *
 * Changes the priority of the running thread, identified its id.
 *
 ##########################################################################*/
int ker_set_thread_priority(ker_thread_id_t th_id,int th_pri)
{
	VDK_SetPriority((VDK_ThreadID)th_id,(VDK_Priority)th_pri);
	return 0;
}

/*##########################################################################
 *
 * Gets the priority of the thread identified by the id.
 *
 ##########################################################################*/
int ker_get_thread_priority(ker_thread_id_t th_id)
{
	return VDK_GetPriority((VDK_ThreadID)th_id);
}

/*##########################################################################
 *
 * Disables the scheduler, this will stop any thread switching. Interrupts
 * are still serviced.
 *
 ##########################################################################*/
int ker_disable_scheduler()
{
	VDK_PushUnscheduledRegion();
	return 1;
}

/*##########################################################################
 *
 * Enables the scheduler, this may result in the context switch.
 *
 ##########################################################################*/
int ker_enable_scheduler()
{
	VDK_PopUnscheduledRegion();
	return 0;
}

/*##########################################################################
 *
 * returns the current running thread's id.
 *
 ##########################################################################*/
int ker_get_current_thread(void)
{
	return((int)VDK_GetThreadID());
}

