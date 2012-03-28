/**
*******************************************************************************
* @file       iFAT32.h
* @version    V0.3  
* @date       2010.03.01
* @brief      This file including all API functions's declare of iFAT32 kernel.	
*******************************************************************************
* @copy
*
* Author email: piaolingsky@gmail.com
* 
* COPYRIGHT 2010 Tomy.Gu 
*******************************************************************************
*/ 

#ifndef  _IFAT32_H
#define  _IFAT32_H

/*---------------------------- Include ---------------------------------------*/
#include "iType.h"


/*---------------------------- Constant Define -------------------------------*/
#define   FAT_Initialized      0xA5
#define   FAT_Uninitialized    0x95
#define   FAT_InvalidValue     0xFFFFFFFF
#define   FILE_LastCluster     0x0FFFFFFF

// 文件属性
#define   FILE_WR              0x00
#define   FILE_R               0x02
#define   FILE_HIDE            0x04
#define   FILE_SYS             0x08
#define   FILE_DIR             0x10
#define   FILE_ARCH            0x20


// 目录项返回值
#define   Item_END             0x00
#define   Item_Invalid         0x01
#define   Item_Valid           0x02


/**
* @struct  IFat32  iFAT32.h	  	
* @brief   IFat32 control block
* @details This struct is use to manage FAT32 file system.
*/
typedef struct
{
	char  fatStatus;
	char  secNumInCluster;
	char  fatNum;
	char  rootDirCluNum;
	char  PartitionName[11];
	DWORD maxItemNum;    //一个簇里面的最多文件信息项
	DWORD secSize;
	DWORD secNum;
	DWORD diskSize;
	DWORD rootDirSec;
	DWORD hiddenSec;
	DWORD fat1Sec;
	DWORD fat2Sec;
	DWORD fatSecNum;
	DWORD mbpAddr;

	// 文件分配表管理
	DWORD vacantClusters;
	DWORD assignCluster;
	DWORD maxAvailableCluster;
	DWORD assignSize;
	// 缓冲区管理，只有一个缓冲区
	bool  isDirty;
	DWORD fatBuf[128];
	DWORD cluSec;
}IFat32,*P_IFat32;


/*---------------------------- Function declare-------------------------------*/
extern bool    iFAT32_Init(P_IFAT pFat);
extern void    iFAT32_Uninstall(void);
extern bool    iFAT32_SpreadAllOverDir(P_IFAT pFat,P_IFile fp);
extern void    iFAT32_FlushData(P_IFile fp);
extern void    iFAT32_DeleteFile(P_IFile fp);
extern void    iFAT32_ShowCurDirList(P_IFAT pFat);
extern void    iFAT32_CreateShortNameFile(P_IFAT pFat, P_IFile fp);
extern void    iFAT32_CreateLongNameFile(P_IFAT pFat, P_IFile fp);
extern DWORD   iFAT32_FileRead(P_IFile fp,char* buf,DWORD length);
extern DWORD   iFAT32_FileWrite(P_IFile fp,char* buf,DWORD length);
extern P_BulkT iFAT32_BulkWriteFile(P_IFile fp,DWORD size,U8* rc);
extern DWORD   iFAT32_DoBulkWrite(P_BulkT pBulk,char* buf,DWORD size);

#endif
