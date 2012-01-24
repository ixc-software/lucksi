//
// etharp_timer.c
//
// A small thread that runs once every ARP_TMR_INTERVAL to update the
// ARP table.
//

#include "lwip/opt.h"
#include "lwip/sys.h"

#include "netif/etharp.h"
#include "netif/etharp_timer.h"

static void etharp_timer_thread(void* arg)
{
  sys_sem_t sem = sys_sem_new(0);
  
  while (1) {
    etharp_tmr();
    sys_sem_wait_timeout(sem, ARP_TMR_INTERVAL);
  }
}

void etharp_timer_init()
{
  sys_thread_new(TCPIP_THREAD_NAME, etharp_timer_thread, NULL, TCPIP_THREAD_STACKSIZE, TCPIP_THREAD_PRIO);
}
