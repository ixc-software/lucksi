/*************************************************************************
 * nifce_driver.h
 *
 *  This file defines the nifce_info struct that is used to pass interface
 * instances and buffer information from the application into nifce_driver.c 
 * via the 'state' pointer in lwIP's netif struct.
 *
 *************************************************************************/
#ifndef _NIFCE_DRIVER_H_
#define _NIFCE_DRIVER_H_

#include <ADI_ETHER.h>

struct nifce_info {
   	size_t                          buff_overhead;
  
  // before nifce_driver_init():
	//   the ratio of receive buffs to transmit buffs to be used
  //   - set both to 1 if equal numbers of each required
  //   - set rx_buffs to 2 and tx_buffs to 1 if twice as many rx buffs reqd
  //   - etc
  // after nifce_driver_init():
  //   contain the actual number of receive and transmit buffers allocated
  short                     rx_buffs;
  short                     tx_buffs;
  
  // the maximum data size that each receive and transmit buffer must support
  short                     rx_buff_datalen;
  short                     tx_buff_datalen;
  
  // the address and size of the area from which buffers are to be allocated
  // (must be 32-bit aligned, uncached and accessible by the controller)
  char*                     buff_area;
  int                       buff_area_size;
  // (after nifce_driver_init() buff_area_size contains the size of the
  //  unused portion of buff_area, if any)
  
  // how does interface get its IP address?
  int                       use_DHCP;
  
  // for the implementation's use
  void*                     x;

  // interface's individual (MAC) address
  unsigned char             ia[NETIF_MAX_HWADDR_LEN];

  // netif's device driver handle
  void*                     handle;
  
  // lists of received/transmitted buffers awaiting processing/disposal
  ADI_ETHER_BUFFER*         rcv_list;
  ADI_ETHER_BUFFER*         xmt_list;
  
  // static TCPIP mailbox messages for invoking the processing callback functions
  struct tcpip_msg          txmsg;
  struct tcpip_msg          rxmsg;

  // keeps track if there is a already a post. if its zero stack callback handler will
  // post the message, else it will be skipped.
  //
  int                       txmsg_processed;
  int                       rxmsg_processed;
};

// Before this function is called to initialise a hardware interface the
// user must have created a nifce_info struct containing the open device  
// handle and the address and size of a memory
// area, the tx/rx buffer ratio, the IA address and the DHCP flag. The netif 
// struct must contain IP addresses for the network, the network mask and 
// gateway, unless DHCP is to be used. The state field must contain the address
// of the corresponding nifce_info struct.
extern err_t nifce_driver_init(struct netif* netif);
err_t nifce_igmp_mac_filter(struct netif* netif, struct ip_addr *ip,u8_t action); 

struct buffer_info
{
  struct netif* netif;
  short         max_buf_len;
};

#endif // _NIFCE_DRIVER_H_
