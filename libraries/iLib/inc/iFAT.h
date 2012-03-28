/**
*******************************************************************************
* @file       iFAT.h
* @version    V0.3  
* @date       2010.03.01
* @brief      This file including all API functions's declare of iFAT kernel.	
*******************************************************************************
* @copy
*
* Author email: piaolingsky@gmail.com
* 
* COPYRIGHT 2010 Tomy.Gu 
*******************************************************************************
*/ 
#ifndef  _IFAT_H
#define  _IFAT_H

/*---------------------------- Include ---------------------------------------*/
#include "iType.h"



/*---------------------------- Constant Define -------------------------------*/
#define  LONG_NAME         0x00
#define  SHORT_NAME        0x01
#define  ILLEGAL_NAME      0x02

#define  FILE_END          0x00
#define  FILE_CUR          0x01
#define  FILE_STR          0x02




/*---------------------------- Function declare-------------------------------*/
extern void  iFAT_ShowCurDir(P_IFAT pFat);
extern bool  iFAT_Init(P_IFAT pFat);
extern void  iFAT_Uninstall(void);
extern bool  iFAT_SetCurDir(P_IFAT pFat, char* dirName);
extern bool  iFAT_DeleteFile(P_IFAT pFat,const char* fpath);
extern bool  iFAT_OpenFile(P_IFAT pFat, P_IFile fp,const char* name);
extern bool  iFAT_CreateFile(P_IFAT pFat, P_IFile fp, const char* fname, P_ITIME iTime);
extern bool  iFAT_CloseFile(P_IFile fp);
extern void  iFAT_GetFileCreatTime(P_IFile fp,P_ITIME iTime);
extern void  iFAT_GetFileModifyTime(P_IFile fp,P_ITIME iTime);
extern bool  iFAT_SeekFile(P_IFile fp,DWORD offset,char fromWhere);
extern DWORD iFAT_ReadFile(P_IFile fp,char* buf, DWORD size);
extern DWORD iFAT_WriteFile(P_IFile fp,char* buf, DWORD size);
extern P_BulkT iFAT_BulkWriteFile(P_IFile fp,DWORD size,U8* rc);
extern DWORD iFAT_DoBulkWrite(P_BulkT pBulk,char* buf,DWORD size);

#endif   

