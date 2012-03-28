#include "ip_stack.h"

void ip_init_stack()
{
  tcpip_init(NULL, NULL);

  // valida se a configuracao tem o DHCP activo
  if (1) {
	  // http://lwip.wikia.com/wiki/DHCP
	  dhcp_start(&eth0);
  } else {
	  // caso contrario
	  // obtem configuração e atribui enderecos
	  IP4_ADDR(&ip_gw, 192,168,2,1);
	  	  IP4_ADDR(&ip_addr, 192,168,2,100);
	  	  IP4_ADDR(&ip_netmask, 255,255,255,0);
  }

  netif_set_default(netif_add(&eth0, &ip_addr, &ip_netmask, &ip_gw, NULL, ethernetif_init, tcpip_input));
  netif_set_up(&eth0);
}
