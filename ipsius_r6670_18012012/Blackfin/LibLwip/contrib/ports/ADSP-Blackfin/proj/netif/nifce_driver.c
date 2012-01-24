/* 
 *   Description:
 *   Contains process packet functionality
 */
#include "lwip/debug.h"
#include "lwip/opt.h"
#include "lwip/def.h"
#include "lwip/mem.h"
#include "lwip/pbuf.h"
#include "lwip/stats.h"
#include "lwip/sys.h"
#include "lwip/tcpip.h"
#include "netif/etharp.h"

#include <kernel_abs.h>
#include <sys/exception.h>
#include <services/services.h>
#include "netif/nifce_driver.h"
#include  <ADI_ETHER.h>

// base of network interface identifiers
#define IFNAME0 'e'
#define IFNAME1 't'

// check for four byte alignment
#define CHECK_ALIGNMENT(x) (((x+3)&(~0x3)) == x)

// interface MTU size - max size of an ethernet frame 
#define MTU 1518
void eth_input(struct pbuf* p, struct netif* netif);
static void process_rcvd_packets(void *nifce);
static void process_xmtd_packets(void *nifce);

/* Trace function */
int (*Trace_Function)( unsigned char EventId, unsigned short NoOfBytes, const unsigned char *Data)  = NULL;   

// Set trace function
void SetTraceFunction( int (*Trace_Event) ( unsigned char EventId, unsigned short NoOfBytes, const unsigned char *Data)) 
{
	Trace_Function = Trace_Event;
}


// Ethernet header as received from the wire
PACK_STRUCT_BEGIN
struct hw_eth_hdr
{
  PACK_STRUCT_FIELD(struct eth_addr dest);
  PACK_STRUCT_FIELD(struct eth_addr src);
  PACK_STRUCT_FIELD(u16_t type);
}
PACK_STRUCT_STRUCT;
PACK_STRUCT_END


/*##########################################################################
 * 
 * processes the transmitted packets by releasing the memory
 *
 *#########################################################################*/
static void process_xmtd_packets(void *arg_nif)
{
  struct netif* netif = (struct netif*)arg_nif;
  struct nifce_info* nip = (struct nifce_info*)netif->state;
  unsigned int *mangle_ptr;
  void *handle = nip->handle;
  int int_sts;

  ADI_ETHER_BUFFER* buffers_to_reuse;
  ADI_ETHER_BUFFER* buffers_to_process;
  ADI_ETHER_BUFFER* recycle_list = 0;
  ADI_ETHER_BUFFER* bp;
  ADI_ETHER_BUFFER* nbp;
  ADI_ETHER_BUFFER* lbp;

  int_sts = ker_disable_interrupts(ker_kPriorityLevelAll);
  buffers_to_reuse = nip->xmt_list;
  nip->xmt_list = NULL;
  nip->txmsg_processed =0;
 ker_enable_interrupts(int_sts);

  // add transmitted buffers to the available list after resetting length
  lbp = bp = buffers_to_reuse;
  while (bp != 0)
  {
  // this is a hack to use the num_rows and num_rows for the
  // purpose of storing the buffer_info address. 
  // TODO: getrid of using reserved area
  //
    mangle_ptr = ((unsigned int*)&bp->Reserved)+4;
    bp->ElementCount = ((struct buffer_info*)((*(unsigned int*)mangle_ptr)))->max_buf_len;
    bp->CallbackParameter =bp;
    bp->StatusWord = 0;
    bp->PayLoad =0;
    bp->ProcessedElementCount=0;
    bp->ProcessedFlag =0;
    lbp = bp;
    bp = bp->pNext;
  } //while

  bp = buffers_to_reuse;
  if (bp) 
  {
    u32_t old_level;

    old_level = sys_arch_protect();
    lbp->pNext = nip->x;
    nip->x = bp;
    sys_arch_unprotect(old_level);
  }
}
static void process_rcvd_packets(void *arg_nif)
{
  struct netif* netif = (struct netif*)arg_nif;
  struct nifce_info* nip = (struct nifce_info*)netif->state;
  unsigned int *mangle_ptr;
  void *handle = nip->handle;
  int int_sts;

  ADI_ETHER_BUFFER* buffers_to_reuse;
  ADI_ETHER_BUFFER* buffers_to_process;
  ADI_ETHER_BUFFER* recycle_list = 0;
  ADI_ETHER_BUFFER* bp;
  ADI_ETHER_BUFFER* nbp;
  ADI_ETHER_BUFFER* lbp;

  // now check for received buffers
  int_sts = ker_disable_interrupts(ker_kPriorityLevelAll);
  buffers_to_process = nip->rcv_list;
  nip->rcv_list = NULL;
  nip->rxmsg_processed =0;
  ker_enable_interrupts(int_sts);	


  // create a pbuf for each received buffer and call eth_input to process it
  bp = buffers_to_process;
  while (bp != 0) 
  {
    struct hw_eth_hdr* ethhdr = (struct hw_eth_hdr*)((char*)bp->Data+2);

    u16_t length = bp->ProcessedElementCount - 6; // 2 + 4crc byte to store length
    struct pbuf* p;
    int unpack_arp = 0;

    nbp = bp->pNext;
    bp->pNext = 0;
    // (char*)bp->Data + 2 contrain the actual data.
    if (Trace_Function) Trace_Function('R',length,((unsigned char *)bp->Data+2));
    
    // see whether we need two extra u16_t fields for alignment
    if (length >= (sizeof(struct etharp_hdr) - 6) &&
            htons(ethhdr->type) == ETHTYPE_ARP) {
      length += 4; // need fields unused2 and unused3 in struct etharp_hdr
      unpack_arp = 1;
    }
    length += 2;// for field unused1 in struct eth_hdr
	
    p = pbuf_alloc(PBUF_RAW, length, PBUF_RAM);

    if (p != NULL) 
	{
      //u8_t* psrc = (u8_t*)bp->Data;
      u8_t* psrc = (u8_t*)bp->Data+2;
      u8_t* pdst = (u8_t*)p->payload;

      // set field unused1 to 0
      *(u16_t*)pdst = 0;
      pdst += 2;

      // copy in eth_hdr data
      memcpy(pdst, psrc, 14);
      pdst += 14;
      psrc += 14;

		if (unpack_arp) 
		{
			// copy etharp frame into pbuf up to field unused2
			memcpy(pdst, psrc, 14);
			pdst += 14;
			psrc += 14;
			// set field unused2 to 0
			*(u16_t*)pdst = 0;
			pdst += 2;
			// copy more of frame up to field unused3
			memcpy(pdst, psrc, 10);
			pdst += 10;
			psrc += 10;
			// set field unused3 to 0
			*(u16_t*)pdst = 0;
			pdst += 2;
			// copy remainder of frame
			memcpy(pdst, psrc, 4);
		} else 
		{
			  // just copy the rest of the frame into the pbuf in one go
		  	 memcpy(pdst, psrc, length - 16);
		}
#ifdef LINK_STATS
      lwip_stats.link.recv++;
#endif

      mangle_ptr = ((unsigned int*)&bp->Reserved)+4; // TODO: get-rid of Reserved.
      eth_input(p, ((struct buffer_info*)(*((unsigned int*)mangle_ptr)))->netif);
    } // p!=NULL 
	else 
	{
#ifdef LINK_STATS
      lwip_stats.link.memerr++;  // had to drop frame - no memory for pbuf
#endif

    }
	mangle_ptr = ((unsigned int*)&bp->Reserved)+4;
    bp->ElementCount = ((struct buffer_info*)(*((unsigned int*)mangle_ptr)))->max_buf_len;
    bp->CallbackParameter = bp;
    bp->PayLoad =0;
    bp->StatusWord =0;
    bp->ProcessedElementCount=0;
    bp->ProcessedFlag =0;
    bp->pNext = recycle_list;
    recycle_list = bp;

    bp = nbp;
  } // while

  // return the buffers to the driver

  if (recycle_list != 0)
	  adi_dev_Read(nip->handle,ADI_DEV_1D,(ADI_DEV_BUFFER*)recycle_list);

}



/*##########################################################################
* 
* Output pbuf chain to hardware. It is assumed that there is a complete and
* correct ethernet frame in p. The only buffering in this system is in the
* list of tx ADI_ETHER_BUFFER's. If there is no room in it, then drop the frame.
*
*#########################################################################*/
static err_t
low_level_output(struct netif* netif, struct pbuf* p)
{
  struct nifce_info* nip = (struct nifce_info*)netif->state;
  ADI_ETHER_BUFFER* tx;
  struct pbuf *q;
  char* data;
  unsigned short *ps;
  struct hw_eth_hdr* ethhdr;
  u32_t old_level;
  int len;

  if (p->tot_len > nip->tx_buff_datalen)
  {
    // frame too big for our buffers
#ifdef LINK_STATS
    lwip_stats.link.memerr++;
#endif
    return ERR_MEM;
  }

  // No need of data maipulation as we are directly getting from pbufs.
  if (Trace_Function) Trace_Function('T',p->tot_len,((unsigned char *)p->payload)+2);

  // see whether we've got a free transmit buffer
  old_level = sys_arch_protect();

  tx = (ADI_ETHER_BUFFER*)nip->x;
  if (tx == NULL)
  {
#ifdef LINK_STATS
    lwip_stats.link.memerr++;
#endif

    sys_arch_unprotect(old_level);
    return ERR_MEM;
  }

  // remove first free one from the list
  nip->x = tx->pNext;
  tx->pNext = NULL;
  sys_arch_unprotect(old_level);

  // copy data from pbuf(s) into our buffer
  q = p;
  //data = (char*)tx->Data;
  
  //
  // first two bytes reserved for length
  data = (char*)tx->Data+2;

  // ... first pbuf: skip field unused1 in struct eth_hdr
  memcpy(data, ((u8_t*)q->payload) + 2, q->len - 2);
  data += (q->len - 2);
  len = q->len - 2;
  q = q->next;
  // copy any subsequent pbufs
  while (q) {
    memcpy(data, q->payload, q->len);
    data += q->len;
    len += q->len;
    q = q->next;
  }
  LWIP_ASSERT("low_level_output: data length correct", len == (p->tot_len - 2));
  //tx->ElementCount = p->tot_len - 2;
  tx->ElementCount = p->tot_len; // total element count including 2 byte header

 

  // see whether we need to shuffle etharp frame up to account for
  // the alignment fields unused2 and unused3
  //ethhdr  = (struct hw_eth_hdr*)tx->Data;
  //
  ethhdr  = (struct hw_eth_hdr*)((char*)tx->Data+2);//skip the header
  if (htons(ethhdr->type) == ETHTYPE_ARP)
  {
    u8_t* pdst = (u8_t*)tx->Data + 28 +2;
    u8_t* psrc = pdst + 2;  // skip unused2 field

    memmove(pdst, psrc, 10);
    pdst += 10;
    psrc += 12;             // skip unused3 field
    memmove(pdst, psrc, 4);

    //tx->num_elements -= 4;
    tx->ElementCount -= 4;
  }

   ps = (unsigned short*)tx->Data;
  *ps = tx->ElementCount-2; // only the frame size excluding 2 byte header
  tx->PayLoad =  0; // payload is part of the packet
  tx->StatusWord = 0; // changes from 0 to the status info 
  // give it to the physical driver
  adi_dev_Write(nip->handle,ADI_DEV_1D,(ADI_DEV_BUFFER*)tx);

#ifdef LINK_STATS
  lwip_stats.link.xmit++;
#endif

  //sys_arch_unprotect(old_level);

  return ERR_OK;
}

/*##########################################################################
*
* Called by the TCP/IP stack when an IP packet should be sent.
* It calls the function called low_level_output() to actually transmit the
* packet.
*#########################################################################*/
static err_t
nifce_driver_output(struct netif* netif, struct pbuf* p, struct ip_addr* ipaddr)
{
  return(etharp_output(netif, p, ipaddr));
}

/*##########################################################################
*
* Sends an incoming Ethernet frame up the stack to a function that will
* understand it (or just drops the frame).
*
*#########################################################################*/
static void
eth_input(struct pbuf* p, struct netif* netif)
{
  // Ethernet protocol layer
  struct eth_hdr* ethhdr;
  struct pbuf* q = NULL;

  ethhdr = p->payload;

  switch (htons(ethhdr->type))
  {
      case ETHTYPE_IP:
      etharp_ip_input(netif, p);
      pbuf_header(p, -(s16_t)sizeof(struct eth_hdr));
      netif->input(p, netif);
      break;
      case ETHTYPE_ARP:
      etharp_arp_input(netif, (struct eth_addr*)&(netif->hwaddr[0]), p);
      break;
      default:
      pbuf_free(p);
      break;
  }

  if (q != NULL)
  {
    low_level_output(netif, q);
    pbuf_free(q);
  }
}

/*##########################################################################
 *
 * low_level_init
 *
 *#########################################################################*/
static void
low_level_init(struct netif *netif)
{
  ADI_ETHER_BUFFER* p;
  struct nifce_info* nip = (struct nifce_info*)netif->state;
  struct buffer_info* bip;
  int rx_len, tx_len, count;
  int i;
  ADI_ETHER_BUFFER* rx_head = NULL;
  ADI_ETHER_BUFFER* tx_head = NULL;
  unsigned int *mangle_ptr;
  struct tcpip_msg* msg;

  LWIP_ASSERT("low_level_init: nfice info supply failed", nip != NULL);
  LWIP_ASSERT("low_level_init: buffer area supply failed",
              nip->buff_area != NULL && nip->buff_area_size > 0);

  // copy individual (MAC) address into netif struct
  for (i = 0; i < 6; i += 1)
    netif->hwaddr[i] = nip->ia[i];
  netif->hwaddr_len = 6;

  // calculate total requirement for each rx and tx buffer
  rx_len =  sizeof(ADI_ETHER_BUFFER) + sizeof(struct buffer_info);
  tx_len = rx_len;
  rx_len += nip->rx_buff_datalen + nip->buff_overhead;
  tx_len += nip->tx_buff_datalen + nip->buff_overhead;
  rx_len = ((rx_len + 3) / 4) * 4;
  tx_len = ((tx_len + 3) / 4) * 4;

  // allocate buffers in required ratio from supplied memory area
  while (nip->buff_area_size > rx_len || nip->buff_area_size > tx_len)
  {
    int n;
    for (n = 0; n < nip->rx_buffs; n += 1) {
      if (nip->buff_area_size < rx_len)
        break;
      p = (ADI_ETHER_BUFFER*)nip->buff_area;
      nip->buff_area += rx_len;
      nip->buff_area_size -= rx_len;
      p->pNext = rx_head;
      rx_head = p;
    }
    for (n = 0; n < nip->tx_buffs; n += 1) {
      if (nip->buff_area_size < tx_len)
        break;
      p = (ADI_ETHER_BUFFER*)nip->buff_area;
      nip->buff_area += tx_len;
      nip->buff_area_size -= tx_len;
      p->pNext = tx_head;
      tx_head = p;
    }
  }

  // initialise each buffer's ADI_ETHER_BUFFER descriptor
  p = rx_head;
  count = 0;
  while (p)
  {
    p->Data = (char*)p + sizeof(ADI_ETHER_BUFFER) + nip->buff_overhead;
    #if defined(__ADSPBF537__)
    if(p->Data >0) 
	    LWIP_ASSERT("Rx data buffers are not aligned properly",CHECK_ALIGNMENT((unsigned int)p->Data));
    #elif (defined(__ADSPBF533__) || defined(__ADSPBF561__))
    // first two bytes in the data buffer are used to store the length.
    if((p->Data >0) && (nip->buff_overhead > 0)) 
	    LWIP_ASSERT("Rx data buffers are not aligned properly",CHECK_ALIGNMENT((unsigned int)((char*)p->Data+ 2)));
    #endif /* __ADSPBF537__ */

    p->ElementCount = nip->rx_buff_datalen;
    p->ElementWidth = 1;
    p->CallbackParameter =p;
    p->ProcessedElementCount=0;
    p->ProcessedFlag =0;
    p->PayLoad = 0;
    p->x = netif;
   
    mangle_ptr = ((unsigned int*)&p->Reserved)+4;
    *((unsigned int*)mangle_ptr)= (unsigned int)((char*)p->Data + p->ElementCount - nip->buff_overhead);
    bip = (struct buffer_info*)(*(unsigned int*)mangle_ptr);
    bip->netif = netif;
    bip->max_buf_len = nip->rx_buff_datalen;
    count += 1;
    p = p->pNext;
  }
  nip->rx_buffs = count;
  
  p = tx_head;
  count = 0;
  while (p)
  {
    p->Data = (char*)p + sizeof(ADI_ETHER_BUFFER) + nip->buff_overhead;

    #if defined(__ADSPBF537__)
    if(p->Data >0) 
	    LWIP_ASSERT("Tx data buffers are not aligned properly",CHECK_ALIGNMENT((unsigned int)p->Data));
    #elif (defined(__ADSPBF533__) || defined(__ADSPBF561__))
    // first two bytes in the data buffer are used to store the length.
    if((p->Data >0) && (nip->buff_overhead > 0)) 
	    LWIP_ASSERT("Tx data buffers are not aligned properly",CHECK_ALIGNMENT((unsigned int)((char*)p->Data +2)));
    #endif /* __ADSPBF537__ */

    p->ElementCount = nip->tx_buff_datalen;
    p->ElementWidth = 1;
    p->CallbackParameter =p;
    p->x = netif;
    p->PayLoad = 0;

    mangle_ptr = ((unsigned int*)&p->Reserved)+4;
    *((unsigned int*)mangle_ptr)= (unsigned int)((char*)p->Data + p->ElementCount - nip->buff_overhead);

    bip = (struct buffer_info*)(*(unsigned int*)mangle_ptr);
	
    bip->netif = netif;
    bip->max_buf_len = nip->tx_buff_datalen;
    count += 1;
    p = p->pNext;
  }
  nip->tx_buffs = count;
  
  // set up this interface's TCPIP callback messages
  msg = &nip->txmsg;
  msg->type = TCPIP_MSG_CALLBACK;
  msg->flags = TCPIP_MSG_FLAG_STATIC;
  msg->msg.cb.f = process_xmtd_packets;
  msg->msg.cb.ctx = netif;
  msg = &nip->rxmsg;
  msg->type = TCPIP_MSG_CALLBACK;
  msg->flags = TCPIP_MSG_FLAG_STATIC;
  msg->msg.cb.f = process_rcvd_packets;
  msg->msg.cb.ctx = netif;
  
  // give all the rx buffers to the Ethernet device driver
  adi_dev_Read(nip->handle,ADI_DEV_1D,(ADI_DEV_BUFFER*)rx_head);

  // save the list of tx buffers until they are needed
  nip->x = tx_head;
}

/*##########################################################################
*
* nifce_driver_init(struct netif *netif):
*
* Should be called at the beginning of the program to set up a
* network interface. It calls the function low_level_init() to do the
* actual setup of the hardware. It expects the 'status' member of netif
* to be holding a pointer to a nifce_info struct.
*
*#########################################################################*/
err_t
nifce_driver_init(struct netif *netif)
{
  sys_sem_t sem;

  // set up base part of this interface's name
  netif->name[0] = IFNAME0;
  netif->name[1] = IFNAME1;
  // install function that TCP should call to output packets
  netif->output = nifce_driver_output;
  // install function that actually queues output for transmission
  netif->linkoutput = low_level_output;
  // set up this interface's maximum transfer unit and address length
  netif->mtu = MTU - 18;      // MTU without ethernet header and crc
  // Enable Broaccast on this Network interface.			
  netif->flags = NETIF_FLAG_BROADCAST;
	
  // carry out lowest level initialisation and enable the interface
  low_level_init(netif);

  return ERR_OK;
}

/*##########################################################################
* nifce_igmp_mac_filter
* 
* Issues multicast igmp filters to the MAC using the device i/o controls
*
*#########################################################################*/
#if LWIP_IGMP

err_t nifce_igmp_mac_filter(struct netif* netif, struct ip_addr *group_ip,u8_t action)
{
struct nifce_info* nip = (struct nifce_info*)netif->state;
u32 Result=ADI_DEV_RESULT_FAILED;
	// if we have peripheral device handle
	if(nip->handle != NULL)
	{
	    switch(action)
	    {
		case IGMP_ADD_MAC_FILTER:
			Result = adi_dev_Control(nip->handle, ADI_ETHER_ADD_MULTICAST_MAC_FILTER,(void*)group_ip->addr);
			break;
					
		case IGMP_DEL_MAC_FILTER:
			Result = adi_dev_Control(nip->handle, ADI_ETHER_DEL_MULTICAST_MAC_FILTER,(void*)group_ip->addr);
			break;
		default:
			break;
	    }
	}
return((Result == ADI_DEV_RESULT_SUCCESS) ? 0: -1);
}
#endif
