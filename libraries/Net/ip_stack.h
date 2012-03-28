#ifndef __IP_STACK__
#define __IP_STACK__

#include "ethernetif/ethernetif.h"
#include "lwip/debug.h"
#include "lwip/opt.h"
#include "lwip/def.h"
#include "lwip/mem.h"
#include "lwip/memp.h"
#include "lwip/pbuf.h"
#include "lwip/sys.h"
#include "lwip/tcpip.h"
#include "lwip/api.h"
#include "lwip/stats.h"
#include "lwip/dhcp.h"
#include "netif/etharp.h"
#include "netif/loopif.h"

static struct ip_addr ip_addr, ip_netmask, ip_gw;
static struct netif eth0;

#endif
