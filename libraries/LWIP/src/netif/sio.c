/* Author: Magnus Ivarsson <magnus.ivarsson@volvo.com> */

#include "netif/sio.h"
#include "netif/fifo.h"
#include "lwip/debug.h"
#include "lwip/def.h"
#include "lwip/sys.h"
#include "lwip/arch.h"

/* Following #undefs are here to keep compiler from issuing warnings
   about them being double defined. (They are defined in lwip/inet.h
   as well as the Unix #includes below.) */
#undef htonl
#undef ntohl
#undef htons
#undef ntohs
#undef HTONL
#undef NTOHL
#undef HTONS
#undef NTOHS

#include <stdlib.h>

#define BAUDRATE B9600
/*#define BAUDRATE B19200 */
/*#define BAUDRATE B57600 */
/*#define BAUDRATE B115200*/

#ifndef TRUE
#define TRUE  1
#endif
#ifndef FALSE
#define FALSE 0
#endif

/* for all of you who dont define SIO_DEBUG in debug.h */
#ifndef SIO_DEBUG
#define SIO_DEBUG 0
#endif

void UART1StdioInit(unsigned long ulPortNum);



/*  typedef struct siostruct_t */
/*  {  */
/*  	sio_status_t *sio; */
/*  } siostruct_t; */

/** array of ((siostruct*)netif->state)->sio structs */
static sio_status_t statusar[3];

/**
* Initiation of serial device
* @param device : string with the device name and path, eg. "/dev/ttyS0"
* @param netif  : netinterface struct, contains interface instance data
* @return file handle to serial dev.
*/
static int sio_init( char * device, int devnum, sio_status_t * siostat )
{
	UART1StdioInit(1);
}

/**
*
*/
static void sio_speed( int fd, int speed )
{
}

/* --public-functions----------------------------------------------------------------------------- */
void sio_send( u8_t c, sio_status_t * siostat )
{

}

void sio_send_string( u8_t *str, sio_status_t * siostat )
{

}


void sio_flush( sio_status_t * siostat )
{
	/* not implemented in FreeRtos as it is not needed */
 	/*sio_status_t * siostat = ((siostruct_t*)netif->state)->sio; */
}


#if PPP_SUPPORT

void sio_expect_string(u8_t* str, sio_status_t * siostat)
{
/*
	u8_t ch;
	u16_t pos=0;
	int ret;


	do
	{
	    ret=read(siostat->fd, &ch, 1);
	    /* failure */
	    if (ret==0) return;

	    if (str[pos]==ch) pos++;
	    else pos=0;
	}
	while(str[pos]!=0);

	/* success */
	return;
*/
}


u32_t sio_write(sio_status_t * siostat, u8_t *buf, u32_t size)
{
    return 0;
}

u32_t sio_read(sio_status_t * siostat, u8_t *buf, u32_t size)
{
    return 0;
}

void sio_read_abort(sio_status_t * siostat)
{
	UARTprintf("sio_read_abort: not yet implemented for FreeRtos\n");
}
#endif /* PPP_SUPPORT */

sio_status_t * sio_open( int devnum )
{

	/* would be nice with dynamic memory alloc */
	sio_status_t * siostate;

	return siostate;
}

/**
*
*/
void sio_change_baud( sioBaudrates baud, sio_status_t * siostat )
{
    /*	sio_status_t * siostat = ((siostruct_t*)netif->state)->sio;*/

	LWIP_DEBUGF( 1,("sio_change_baud\n" ));

	switch ( baud )
	{
		case SIO_BAUD_9600:
			sio_speed( siostat->fd, B9600 );
			break;
		case SIO_BAUD_19200:
			sio_speed( siostat->fd, B19200 );
			break;
		case SIO_BAUD_38400:
			sio_speed( siostat->fd, B38400 );
			break;
		case SIO_BAUD_57600:
			sio_speed( siostat->fd, B57600 );
			break;
		case SIO_BAUD_115200:
			sio_speed( siostat->fd, B115200 );
			break;

		default:
			LWIP_DEBUGF( 1,("sio_change_baud: Unknown baudrate, code:%d\n", baud ));
			break;
	}
}

