/********************************************************************************************************
 * TFTP packets 																						*
 ********************************************************************************************************/
#define MAXTFTP     	512
#define TFTPORT     	69      			// Triial File Transfer

#define TFTP_OP_READ   	1       			// Opcodes: read request
#define TFTP_OP_WRITE  	2       			// write request
#define TFTP_OP_DATA	3
#define TFTP_OP_ACK		4
#define TFTP_OP_ERR    	5      				// error

typedef __packed struct tftpr  				// TFTP read/write request
{
    unsigned short op;                		// Opcode
    unsigned char data[MAXTFTP+2];   		// Filename & mode strings
} TFTP_REQ;

typedef __packed struct tftpd
{
	unsigned short op;
	unsigned short block;
	unsigned char data[MAXTFTP];
} TFTP_DATA;

#endif