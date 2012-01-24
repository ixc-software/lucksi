//
// dhcp_timer.c
//
// A small thread that runs once every DHCP_FINE_TIMER_MSECS to call the
// DHCP timer functions
//

#include "lwip/opt.h"
#include "lwip/sys.h"
#include "lwip/dhcp.h"

#include "netif/dhcp_timer.h"

static void dhcp_timer_thread(void* arg)
{
  static int coarse_timer = (DHCP_COARSE_TIMER_SECS * 1000);
  sys_sem_t sem = sys_sem_new(0);
  
  while (1) {
    dhcp_fine_tmr();
    coarse_timer -= DHCP_FINE_TIMER_MSECS;
    if (coarse_timer <= 0) {
      dhcp_coarse_tmr();
      coarse_timer = (DHCP_COARSE_TIMER_SECS * 1000);
    }
    sys_sem_wait_timeout(sem, DHCP_FINE_TIMER_MSECS);
  }
}

void dhcp_timer_init()
{
  sys_thread_new(TCPIP_THREAD_NAME, dhcp_timer_thread, NULL, TCPIP_THREAD_STACKSIZE, TCPIP_THREAD_PRIO);
}
