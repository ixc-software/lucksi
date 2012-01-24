/*
 * Descripton:
 *  Contains packet callbacks, dhcp, buffer handling functionlity
 */
#include "lwip/debug.h"
#include "lwip/ip.h"
#include "lwip/tcpip.h"
#include "lwip/netif.h"
#include "lwip/dhcp.h"
#include "netif/etharp.h"

#include <cglobals.h>
#include <kernel_abs.h>
#include <sys/exception.h>
#include <services/services.h>
#include "netif/nifce_driver.h"
#include <ADI_ETHER.h>

// Enable it to print the obtained IP address.
//#define LWIP_DEBUG

// check for four byte alignment
#define CHECK_ALIGNMENT(x) (((x+3)&(~0x3)) == x)


// Maximum number of supported interfaces.
#define MAX_NETWORK_IF 2 

// struct to hold network driver instance specific information.
//
struct network_info
{
    int                       num_if;
    int                       imask[MAX_NETWORK_IF]; 
    struct nifce_info          vinfo[MAX_NETWORK_IF];
    struct netif*             netif[MAX_NETWORK_IF];
    struct ip_addr            ipaddr[MAX_NETWORK_IF];
    struct ip_addr            netmask[MAX_NETWORK_IF];
    struct ip_addr            gateway[MAX_NETWORK_IF];
};

// The below two external variables will be  defined in the generated user 
// configuration file
//
extern net_config_info user_net_config_info[];
extern int user_net_num_ifces;

// global statics
static struct network_info  netinfo = {0};
static ADI_DEV_DEVICE_HANDLE pli_services[MAX_NETWORK_IF];

// loopback stuff
static struct ip_addr lb_ipaddr, lb_netmask, lb_gw;
struct netif loopif;

// proto
void lwip_init(u8_t *lwip_memory, int memSize);
void dhcp_timer_init(void);
void   etharp_timer_init(void);
int    dhcp_configure(void);
err_t  loopif_init(struct netif *netif);


/****************************************************************
 *
 *  Append the packet the passed list. list could be recv list
 *  or transmit list.must be called in critical region.
 *  list could be rcv_list or xmt_list
 ****************************************************************/
static void append_list(ADI_ETHER_BUFFER **list,ADI_ETHER_BUFFER *b)
{
    ADI_ETHER_BUFFER *t;

    if (*list == NULL)
        *list = b;
    else
    {
        t = *list;
        while (t->pNext != NULL)
            t= t->pNext;
        t->pNext = b;
    }
}


/****************************************************************
 *
 * Defered callback handler. Called by an ethernet device driver with
 * list of transmitted or received buffers. Appends them to network
 * interface's lists and posts messages to stack's main processing 
 * thread to deal with them.
 ****************************************************************/
void stack_callback_handler(void* arg1,unsigned int event,void* arg3)
{
    ADI_ETHER_BUFFER* pack_list = (ADI_ETHER_BUFFER*)arg3;

    if (pack_list != NULL)
    {
        struct netif* ni;
        struct nifce_info* nip;

        /* packet list may get modified from the TCP thread and all
         * instances of it were protected using the critical regions
         * Here we are in a critical region because we do not want to be 
         * preempted with a high priority interrupt.
         */
        int int_sts = cli();

        switch (event)
        {
        case ADI_ETHER_EVENT_FRAME_RCVD:
            ni = (struct netif*)pack_list->x;
            nip = (struct nifce_info*)ni->state;
            append_list(&nip->rcv_list, pack_list);

            // trigger callback thread to process packet if its not already
            if (!nip->rxmsg_processed)
            {
                tcpip_ISR_callback(&nip->rxmsg);
                nip->rxmsg_processed = 1;
            }
            break;

        case ADI_ETHER_EVENT_FRAME_XMIT:
            ni = (struct netif*)pack_list->x;
            nip = (struct nifce_info*)ni->state;
            append_list(&nip->xmt_list, pack_list);

            // trigger callback thread to process packet if its not already
            if (!nip->txmsg_processed)
            {
                tcpip_ISR_callback(&nip->txmsg);
                nip->txmsg_processed = 1;
            }
            break;

        case ADI_ETHER_EVENT_INTERRUPT: 
            break;
        }

        sti(int_sts);
    }
}

// ----------------------------------------------------------------------

static const bool CEnableLoopback = false;

void RegisterLoopbackIntf(void)
{
    IP4_ADDR(&lb_gw,      127,0,0,1);
    IP4_ADDR(&lb_ipaddr,  127,0,0,1);
    IP4_ADDR(&lb_netmask, 255,0,0,0);

    netif_add(&loopif, &lb_ipaddr, &lb_netmask, &lb_gw, NULL, loopif_init, ip_input /* tcpip_input */);   
}




/****************************************************************
 * Initialises LWIP modules, assigns Tx and Rx buffers to each 
 * network interface and starts ARP update thread.
 *
 * Buffer structure
 * 
 *   ---------  ----------       ---------------   ------------
 *   buffer_t   buffer overhead  actual buffer    buffer_info
 *   ---------  ----------       ---------------  -------------
 ****************************************************************/

int init_stack(int poll_thread_pri, int poll_thread_per, int inMemSize, char *MemArea)
{
    int i,overhead=0;
    int nw_ifce_buffer_length = 0;
    int total_buffer_length =0;
    unsigned int LWIP_AREA_START=0;
    unsigned int BUFF_AREA_START=0;
    unsigned int BUFF_AREA_LEN=0; 
    unsigned int buff_overhead[MAX_NETWORK_IF] = {0};

    BUFF_AREA_START = (unsigned int)(((unsigned int)(MemArea+32)) & ~31);
    BUFF_AREA_LEN   = (unsigned int)inMemSize;

    LWIP_ASSERT("Supplied memory is not aligned",CHECK_ALIGNMENT((unsigned int)MemArea)); 

    // defined in the user configuration file.
    netinfo.num_if =    user_net_num_ifces;

    // get driver specific overhead
    //
    for (i=0;i<netinfo.num_if;i++)
    {

        adi_dev_Control(pli_services[i],ADI_ETHER_CMD_GET_BUFFER_PREFIX,(void*)&overhead);
        buff_overhead[i] = overhead;

#if defined(__ADSPBF537__)
        // On BF537 buffer overhead has to be 4 byte aligned
        // Bf5337 DMA requires the first two bytes to hold the length.
        //
        /*           <-- must be 4 byte aligned 
         *  ----------------------------------------------
         *   prefix | 2bytes| ethernet header| data
         *  ----------------------------------------------
         */
        if (overhead > 0)
            LWIP_ASSERT("Buffer overhead is not four byte aligned",CHECK_ALIGNMENT(overhead));
#elif (defined(__ADSPBF533__) || defined(__ADSPBF561__))
        // first two bytes are used to store the size of buffer
        // itself. So the actual Data must be 4 byte aligned.
        //
        /*                   <-- must be 4 byte aligned 
         *  ----------------------------------------------
         *   prefix | 2bytes| ethernet header| data
         *  ----------------------------------------------
         */

        if (overhead > 0)
            LWIP_ASSERT("Improper alignment [for BF533 and Bf561 overhead+2 has to be four byte aligned]",CHECK_ALIGNMENT((overhead+2)));
#endif
    } 

    // TODO: if multiple n/w interfaces were used then currently the buff_overhead is assumed
    // to be same for both network interfaces. Change the below code if its different.
    //
    overhead  = sizeof(ADI_ETHER_BUFFER) + sizeof(struct buffer_info) + buff_overhead[0];

    nw_ifce_buffer_length = 0;


    // Caluculate the total buffer space for rx/tx and for all n/w interfaces.
    //
    for (i = 0; i < netinfo.num_if; i += 1)
    {
        LWIP_ASSERT("Rx packet length is not four byte aligned",CHECK_ALIGNMENT(user_net_config_info[i].rx_buff_datalen)); 
        LWIP_ASSERT("Tx packet length is not four byte aligned",CHECK_ALIGNMENT(user_net_config_info[i].tx_buff_datalen)); 
        nw_ifce_buffer_length += user_net_config_info[i].rx_buffs * user_net_config_info[i].rx_buff_datalen;
        nw_ifce_buffer_length += user_net_config_info[i].tx_buffs * user_net_config_info[i].tx_buff_datalen;
        nw_ifce_buffer_length  += user_net_config_info[i].rx_buffs * overhead;
        nw_ifce_buffer_length  += user_net_config_info[i].tx_buffs * overhead;
    }

    // base address for major lwip data areas
    //
    LWIP_AREA_START = BUFF_AREA_START + nw_ifce_buffer_length;
    LWIP_ASSERT("RX/TX buffs too big", (inMemSize > nw_ifce_buffer_length));

    // Initialize lwip stack modules.
    //
    lwip_init( (u8_t*)LWIP_AREA_START, (inMemSize - nw_ifce_buffer_length));

    // Setup interfaces and their associated buffers.
    //
    for (i = 0; i < netinfo.num_if; i += 1)
    {
        struct nifce_info* nip = &netinfo.vinfo[i];
        struct netif*  ni;
        void *handle = pli_services[i];

        nip->handle = handle;

        // Compute the size of the memory area with the user given pli buffer
        // size and its length.
        nw_ifce_buffer_length = 0;

        // Add rx buffer area
        nw_ifce_buffer_length += user_net_config_info[i].rx_buffs * user_net_config_info[i].rx_buff_datalen;

        // Add Tx buffer area.
        nw_ifce_buffer_length += user_net_config_info[i].tx_buffs * user_net_config_info[i].tx_buff_datalen;

        // reserve space for overhead also.
        nw_ifce_buffer_length  += user_net_config_info[i].rx_buffs * overhead;
        nw_ifce_buffer_length  += user_net_config_info[i].tx_buffs * overhead;

        nip->rx_buffs = user_net_config_info[i].rx_buffs;
        nip->tx_buffs = user_net_config_info[i].tx_buffs;
        nip->rx_buff_datalen = user_net_config_info[i].rx_buff_datalen;
        nip->tx_buff_datalen = user_net_config_info[i].tx_buff_datalen;
        nip->buff_area = (char*)(BUFF_AREA_START + total_buffer_length);
        nip->buff_area_size = nw_ifce_buffer_length;
        nip->use_DHCP = user_net_config_info[i].use_dhcp;

        // add it to the total length
        total_buffer_length += nw_ifce_buffer_length;

        // If DHCP is not enabled then we have user specified IP address,
        // Subnet mask, Gateway
        //
        if (nip->use_DHCP == 0)
        {
            IP4_ADDR(&netinfo.ipaddr[i],
                     (user_net_config_info[i].ipaddr >> 24) & 0x000000ff , 
                     (user_net_config_info[i].ipaddr >> 16) & 0x000000ff, 
                     ( user_net_config_info[i].ipaddr>> 8) & 0x000000ff,
                     ( user_net_config_info[i].ipaddr >> 0) & 0x000000ff);

            IP4_ADDR(&netinfo.netmask[i],
                     (user_net_config_info[i].netmask >> 24) & 0x000000ff , 
                     (user_net_config_info[i].netmask >> 16) & 0x000000ff, 
                     ( user_net_config_info[i].netmask>> 8) & 0x000000ff,
                     ( user_net_config_info[i].netmask >> 0) & 0x000000ff);

            IP4_ADDR(&netinfo.gateway[i],
                     (user_net_config_info[i].gateway >> 24) & 0x000000ff , 
                     (user_net_config_info[i].gateway >> 16) & 0x000000ff, 
                     ( user_net_config_info[i].gateway>> 8) & 0x000000ff,
                     ( user_net_config_info[i].gateway >> 0) & 0x000000ff);
        }

        // get if Mac address is specified. If user specified is 0 then
        // we will query the supllied IPLI to get the actual address.
        //
        if (memcmp(user_net_config_info[i].mac_addr,"\x00\x00\x00\x00\x00\x00",6))
        {
            memcpy(&netinfo.vinfo[i].ia[0], user_net_config_info[i].mac_addr, 6);
            adi_dev_Control(pli_services[i],ADI_ETHER_CMD_SET_MAC_ADDR,(void*)&user_net_config_info[i].mac_addr);
        }
        else
        {
            // Applicatons must set up the physical address of the interface
            // before.
            char m_addr[] = { 0,0,0,0,0,0};
            adi_dev_Control(pli_services[i],ADI_ETHER_CMD_GET_MAC_ADDR,(void*)&m_addr);
            memcpy(&netinfo.vinfo[i].ia[0],m_addr,6);
        }


        nip->buff_overhead = (size_t)buff_overhead[0];

        // initialise lists of completed buffers awaiting processing/disposal
        nip->rcv_list = NULL;
        nip->xmt_list = NULL;

        netinfo.netif[i] = (struct netif*)malloc(sizeof(struct netif));

        // set the initial state of txmsg_processed and rxmsg_processed to zero
        nip->txmsg_processed  = 0;
        nip->rxmsg_processed  = 0;

        ni = netinfo.netif[i] = netif_add(
                                         netinfo.netif[i],
                                         &netinfo.ipaddr[i],
                                         &netinfo.netmask[i],
                                         &netinfo.gateway[i],
                                         &netinfo.vinfo[i],
                                         nifce_driver_init,
                                         ip_input
                                         );


        LWIP_ASSERT("boot thread: failed to add network interface", ni != NULL);

#if LWIP_IGMP
        ni->igmp_mac_filter = nifce_igmp_mac_filter;
        ni->flags = NETIF_FLAG_IGMP;
        igmp_start( ni);     
#endif


#ifdef LWIP_DEBUG
        printf(
              "Network Interface %c%c%d: %d.%d.%d.%d : "
              "%02X-%02X-%02X-%02X-%02X-%02X : RX buffs = %d : TX buffs = %d\n",
              ni->name[0], ni->name[1], ni->num,
              ip4_addr1(&ni->ip_addr), ip4_addr2(&ni->ip_addr),
              ip4_addr3(&ni->ip_addr), ip4_addr4(&ni->ip_addr),
              ni->hwaddr[0], ni->hwaddr[1], ni->hwaddr[2],
              ni->hwaddr[3], ni->hwaddr[4], ni->hwaddr[5],
              ((struct nifce_info*)ni->state)->rx_buffs,
              ((struct nifce_info*)ni->state)->tx_buffs
              );
#endif /* LWIP_DEBUG */

    }

    // register loopback
    if (CEnableLoopback) RegisterLoopbackIntf();

    // set the default network interface
    //
    netif_set_default(CEnableLoopback ? &loopif : netinfo.netif[0]);

    // start the ARP update thread
    etharp_timer_init();

    return 1;
}

// ----------------------------------------------------------------------

int InitStackEx(int poll_thread_pri, int poll_thread_per, int inMemSize, char *MemArea, net_config_info *pCfg)
{
    // overwite global config -- lame, but minimal changes to with stupid spaggetti code
    LWIP_ASSERT("Config non-empty", (pCfg != 0));

    user_net_config_info[0] = *pCfg;
    user_net_num_ifces = 1;

    // do it
    return init_stack(poll_thread_pri, poll_thread_per, inMemSize, MemArea);
}

/****************************************************************
 * Invoked when users calls StartStack on the TCP/IP component.
 * Currently only DHCP configuration is checked. If DHCP is enabled
 * stack checks the DHCP server and gets the addres.
 ****************************************************************/
int start_stack()
{
    return dhcp_configure();
}

/****************************************************************
 * Gets the address from the DHCP server. DHCP is checked for only
 * the interface on which the DHCP is enabled in the UI.
 ****************************************************************/
int dhcp_configure(void)
{
    int i;
    long time_slept=0;
    int  success_flag = 1;
    dhcp_timer_init();

    for (i = 0; i < netinfo.num_if; i += 1)
    {
        struct netif* ni = netinfo.netif[i];

        if (user_net_config_info[i].use_dhcp == 1)
        {
            dhcp_start(ni);

            // Wait for 50ms and check for the DHCP status.            
            ker_sleep(50);
            
            
            
            /*
            //Original VDSP code:
            while (*(volatile u8_t*)&ni->dhcp->state != DHCP_BOUND)
            {
                // sleep for 500ms and check again, for 2 min..
                ker_sleep(500);
                time_slept += 500;

                if (time_slept > 120*1000)
                {
                    success_flag = 0;
                    break;
                }
            }
            */
                        
            // replace with
            if (*(volatile u8_t*)&ni->dhcp->state != DHCP_BOUND) 
            {
            	success_flag = 0;
            }            
            
            

#ifdef _LWIP_DEBUG_
            printf(
                  "Network Interface %c%c%d: %d.%d.%d.%d : "
                  "%02X-%02X-%02X-%02X-%02X-%02X : RX buffs = %d : TX buffs = %d\n",
                  ni->name[0], ni->name[1], ni->num,
                  ip4_addr1(&ni->ip_addr), ip4_addr2(&ni->ip_addr),
                  ip4_addr3(&ni->ip_addr), ip4_addr4(&ni->ip_addr),
                  ni->hwaddr[0], ni->hwaddr[1], ni->hwaddr[2],
                  ni->hwaddr[3], ni->hwaddr[4], ni->hwaddr[5],
                  ((struct nifce_info*)ni->state)->rx_buffs,
                  ((struct nifce_info*)ni->state)->tx_buffs
                  );
#endif /* _LWIP_DEBUG_ */
        }
        // If user configures with a static IP address then activate the link. 
        // 
        else
        {
            // activate link
            netif_set_up(ni);
        }

    }
    return success_flag;
}

/****************************************************************
 * gethostaddr() gets the primary network interfaces IP address
 * in dot notation form.
 ****************************************************************/
int gethostaddr(int nwifce_no,char *host_addr)
{

    if (nwifce_no >= 0 && nwifce_no < netinfo.num_if)
    {
        struct netif* ni = netinfo.netif[nwifce_no];
        //	u32_t ipaddr = ni->ip_addr.addr;
        struct in_addr ipaddr;
        ipaddr.s_addr = ni->ip_addr.addr;
        char *ip_in_dot = inet_ntoa(ipaddr);
        memcpy(host_addr,ip_in_dot,16);
        return 1;
    }
    return -1;
}
/****************************************************************
 * sethostaddr() sets the hostaddress for the specified interface.
 ****************************************************************/

int sethostaddr(int nwifce_no,char *host_addr)
{
    if (nwifce_no >= 0 && nwifce_no < netinfo.num_if)
    {
        struct netif* ni = netinfo.netif[nwifce_no];

        struct ip_addr ip;
        ip.addr = (u32)inet_addr(host_addr);
        ni->ip_addr.addr = ip.addr;
        return 1;
    }
    return -1;
}

// sets the pli services for stack to use
int set_pli_services(int num_services, ADI_DEV_DEVICE_HANDLE *pservices)
{
    int i=0;
    for (i=0; i<num_services; i++)
        pli_services[i]= (ADI_DEV_DEVICE_HANDLE)pservices[i];
    return 1;
}
// return default gateway
struct ip_addr get_default_gateway()
{
    struct netif* ni = netinfo.netif[0];
    return(ni->gw);
}

// shutdown the network interface
err_t nifce_shutdown(int nwifce_no)
{
    if (nwifce_no >= 0 && nwifce_no < netinfo.num_if)
    {
        struct netif* ni = netinfo.netif[nwifce_no];        
        netif_remove(ni);
        free(ni);
    }

    return(0);
}
