/**
*******************************************************************************
* @file       iDisk.h
* @version    V0.3  
* @date       2010.03.01
* @brief      This file including all API functions's declare of iDisk kernel.
*******************************************************************************
* @copy
*
* Author email: piaolingsky@gmail.com
* 
* COPYRIGHT 2010 Tomy.Gu 
*******************************************************************************
*/

#ifndef  _MSD_H_
#define  _MSD_H_


//首先只处理一个分区，多分区暂不支持
// typedef struct
// {
// 	DWORD dbrSec;
// 	DWORD partitionSecNum;
// 	DWORD secSize;
// }IDisk,*P_IDisk;


/*---------------------------- Function declare-------------------------------*/
extern char iDisk_Init(void);
extern unsigned int  iDisk_GetBlockSize(void);
extern char iDisk_ReadBlock(int addr,char* buf,int blockSize);
extern char iDisk_WriteBlock(int addr,char* buf,int blockSize);
extern char iDisk_ReadBuffer(int addr,char* buf,int bufSize);
extern char iDisk_WriteBuffer(int addr,char* buf,int bufSize);
extern char iDisk_BeginBulkWrite(int addr);
extern char iDisk_DoBulkWrite(char* buf,int bufSize,int residueLength);

#endif