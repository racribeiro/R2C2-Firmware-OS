#include <CoOS.h>
#include "system_lpc17xx.h"
#include "Serial.h"

void cli_init()
{
  // activate serial port over USB interface

  // register task
}

void ip_init()
{
  // read ip configuration file

  // activate ethernet

  // activate ip stack
}

int main(void)
{
	// SystemINIT
	SystemInit();

	// Init Debug Serial
	serial_init();

	// Init COOS
	CoInitOS ();

	// Setup tasks

	cli_init();		  // Init Serial Client
	ip_init();        // Init IP Stack
	http_init();	  // Init HTTP Server

	// Start Tasks!
	CoStartOS ();

	// Never to get here!!!
	while(1)
    {
    }
}
