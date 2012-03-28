/*
 * This file is a skeleton for developing Ethernet network interface
 * drivers for lwIP. Add code to the low_level functions and do a
 * search-and-replace for the word "ethernetif" to replace it with
 * something that better describes your network interface.
 */

#include "lwip/opt.h"
#include "lwip/def.h"
#include "lwip/mem.h"
#include "lwip/pbuf.h"
#include "lwip/sys.h"
#include <lwip/stats.h>
#include <lwip/snmp.h>
#include "netif/etharp.h"
#include "netif/ppp_oe.h"
#include "ethernetif.h"
#include "emac.h"

/* Define those to better describe your network interface. */
#define IFNAME0 'e'
#define IFNAME1 'n'

#define ETH_MTU		1500

OS_EventID semEthTx;
OS_EventID semEthRx;

#define sizeEthif  400
OS_STK stkEthif[sizeEthif];
#define prioEthif   (TCPIP_THREAD_PRIO+1)

/**
 * Helper struct to hold private data used to operate your ethernet interface.
 * Keeping the ethernet address of the MAC in this struct is not necessary
 * as it is already kept in the struct netif.
 * But this is only an example, anyway...
 */
struct ethernetif {
  struct eth_addr *ethaddr;
  /* Add whatever per-interface state that is needed here. */
};

/**
 * In this function, the hardware should be initialized.
 * Called from ethernetif_init().
 *
 * @param netif the already initialized lwip network interface structure
 *        for this ethernetif
 */
static void
low_level_init(struct netif *netif)
{
  StatusType result;
//  struct ethernetif *ethernetif = netif->state;
  
  /* set MAC hardware address length */
  netif->hwaddr_len = ETHARP_HWADDR_LEN;

  /* set MAC hardware address */
  netif->hwaddr[5] = MYMAC_1;
  netif->hwaddr[4] = MYMAC_2;
  netif->hwaddr[3] = MYMAC_3;
  netif->hwaddr[2] = MYMAC_4;
  netif->hwaddr[1] = MYMAC_5;
  netif->hwaddr[0] = MYMAC_6;
  
  /* maximum transfer unit */
  netif->mtu = ETH_MTU;
  
  /* device capabilities */
  /* don't set NETIF_FLAG_ETHARP if this device is not an ethernet one */
  netif->flags = NETIF_FLAG_BROADCAST | NETIF_FLAG_ETHARP | NETIF_FLAG_LINK_UP;  
  /* Do whatever else is needed to initialize interface. */  
  semEthTx = CoCreateSem(NUM_TX_FRAG, NUM_TX_FRAG, 1); 
  semEthRx = CoCreateSem(0, NUM_RX_FRAG, 1); 
  
  if(semEthTx == E_CREATE_FAIL) {
    LWIP_DEBUGF(NETIF_DEBUG,("Semaphore for ETH transmit created failed !\n\r"));
  };
  if(semEthRx == E_CREATE_FAIL) {
    LWIP_DEBUGF(NETIF_DEBUG,("Semaphore for ETH recive created failed !\n\r"));  
  };

  result = CoCreateTask((FUNCPtr)ethernetif_input, 
                        (void *)netif, 
                        prioEthif, 
                        &stkEthif[sizeEthif-1], 
                        sizeEthif);
  
  if(result == E_CREATE_FAIL) {
    LWIP_DEBUGF(NETIF_DEBUG,("Task for ETH recive created failed !\n\r "));
  };

  Init_EMAC();
}

/**
 * This function should do the actual transmission of the packet. The packet is
 * contained in the pbuf that is passed to the function. This pbuf
 * might be chained.
 *
 * @param netif the lwip network interface structure for this ethernetif
 * @param p the MAC packet to send (e.g. IP packet including MAC addresses and type)
 * @return ERR_OK if the packet could be sent
 *         an err_t value if the packet couldn't be sent
 *
 * @note Returning ERR_MEM here if a DMA queue of your MAC is full can lead to
 *       strange results. You might consider waiting for space in the DMA queue
 *       to become availale since the stack doesn't retry to send a packet
 *       dropped because of memory failure (except for the TCP timers).
 */

static err_t
low_level_output(struct netif *netif, struct pbuf *p)
{
//  struct ethernetif *ethernetif = netif->state;
  struct pbuf *q;
  
//  initiate transfer();
  
#if ETH_PAD_SIZE
  pbuf_header(p, -ETH_PAD_SIZE); /* drop the padding word */
#endif

  if(CoPendSem(semEthTx, 0) == E_OK) {
    RequestSend(p->tot_len);

    for(q = p; q != NULL; q = q->next) {
      /* Send the data from the pbuf to the interface, one pbuf at a
         time. The size of the data in each pbuf is kept in the ->len
         variable. */
      //send data from(q->payload, q->len);
  	  CopyToFrame_EMAC_Start(q->payload, q->len);
    }
  
    //signal that packet should be sent();
    CopyToFrame_EMAC_End();  
  }
  
  pbuf_free(p);

#if ETH_PAD_SIZE
  pbuf_header(p, ETH_PAD_SIZE); /* reclaim the padding word */
#endif
  
  LINK_STATS_INC(link.xmit);

  return ERR_OK;
}

/**
 * Should allocate a pbuf and transfer the bytes of the incoming
 * packet from the interface into the pbuf.
 *
 * @param netif the lwip network interface structure for this ethernetif
 * @return a pbuf filled with the received packet (including MAC header)
 *         NULL on memory error
 */
static struct pbuf *
low_level_input(struct netif *netif)
{
//  struct ethernetif *ethernetif = netif->state;
  struct pbuf *p, *q;
  u16_t len;

  /* Obtain the size of the packet and put it into the "len"
     variable. */
  
  len = StartReadFrame();

#if ETH_PAD_SIZE
  len += ETH_PAD_SIZE; /* allow room for Ethernet padding */
#endif

  /* We allocate a pbuf chain of pbufs from the pool. */
  p = pbuf_alloc(PBUF_RAW, len, PBUF_POOL);
  
  if (p != NULL) {

#if ETH_PAD_SIZE
    pbuf_header(p, -ETH_PAD_SIZE); /* drop the padding word */
#endif

	
    /* We iterate over the pbuf chain until we have read the entire
     * packet into the pbuf. */
    for(q = p; q != NULL; q = q->next) {
      /* Read enough bytes to fill this pbuf in the chain. The
       * available data in the pbuf is given by the q->len
       * variable. */
	    //read data into(q->payload, q->len);
		  CopyFromFrame_EMAC(q->payload, q->len);
    }
	    // acknowledge that packet has been read();
	  EndReadFrame();

#if ETH_PAD_SIZE
    pbuf_header(p, ETH_PAD_SIZE); /* reclaim the padding word */
#endif

    LINK_STATS_INC(link.recv);
  } else {
//  drop packet();
    EndReadFrame();   // Drop packet
    LINK_STATS_INC(link.memerr);
    LINK_STATS_INC(link.drop);
  } 
  
  return p;  
}

/**
 * This function should be called when a packet is ready to be read
 * from the interface. It uses the function low_level_input() that
 * should handle the actual reception of bytes from the network
 * interface. Then the type of the received packet is determined and
 * the appropriate input function is called.
 *
 * @param netif the lwip network interface structure for this ethernetif
 */
void
ethernetif_input(struct netif *netif)
{
  //struct ethernetif *ethernetif;
  struct eth_hdr *ethhdr;
  struct pbuf *p;

  //ethernetif = netif->state;
 
  for(;;) {
    if(CoPendSem(semEthRx, 0) == E_OK) {
      /* move received packet into a new pbuf */
      p = low_level_input(netif);
      /* no packet could be read, silently ignore this */
      if (p == NULL) continue;
      /* points to packet payload, which starts with an Ethernet header */
      ethhdr = p->payload;
    
      switch (htons(ethhdr->type)) {
        /* IP or ARP packet? */
        case ETHTYPE_IP:
          
        case ETHTYPE_ARP:
  #if PPPOE_SUPPORT
        /* PPPoE packet? */
        case ETHTYPE_PPPOEDISC:
        case ETHTYPE_PPPOE:
  #endif /* PPPOE_SUPPORT */
          /* full packet send to tcpip_thread to process */
          if (netif->input(p, netif)!=ERR_OK) { 
            LWIP_DEBUGF(NETIF_DEBUG, ("ethernetif_input: IP input error\n"));
            pbuf_free(p);
            p = NULL;
          }
          break;
        default:
          pbuf_free(p);
          p = NULL;
          break;
      }
    }    
  }  
}

/**
 * Should be called at the beginning of the program to set up the
 * network interface. It calls the function low_level_init() to do the
 * actual setup of the hardware.
 *
 * This function should be passed as a parameter to netif_add().
 *
 * @param netif the lwip network interface structure for this ethernetif
 * @return ERR_OK if the loopif is initialized
 *         ERR_MEM if private data couldn't be allocated
 *         any other err_t on error
 */
err_t
ethernetif_init(struct netif *netif)
{
  struct ethernetif *ethernetif;

  LWIP_ASSERT("netif != NULL", (netif != NULL));
    
  ethernetif = mem_malloc(sizeof(struct ethernetif));
  if (ethernetif == NULL) {
    LWIP_DEBUGF(NETIF_DEBUG, ("ethernetif_init: out of memory\n"));
    return ERR_MEM;
  }

#if LWIP_NETIF_HOSTNAME
  /* Initialize interface hostname */
  netif->hostname = "lwip";
#endif /* LWIP_NETIF_HOSTNAME */

  /*
   * Initialize the snmp variables and counters inside the struct netif.
   * The last argument should be replaced with your link speed, in units
   * of bits per second.
   */
  //NETIF_INIT_SNMP(netif, snmp_ifType_ethernet_csmacd, ???);

  netif->state = ethernetif;
  netif->name[0] = IFNAME0;
  netif->name[1] = IFNAME1;
  /* We directly use etharp_output() here to save a function call.
   * You can instead declare your own function an call etharp_output()
   * from it if you have to do some checks before sending (e.g. if link
   * is available...) */
  netif->output = etharp_output;
  netif->linkoutput = low_level_output;
  
  ethernetif->ethaddr = (struct eth_addr *)&(netif->hwaddr[0]);
  
  /* initialize the hardware */
  low_level_init(netif);

  return ERR_OK;
}

