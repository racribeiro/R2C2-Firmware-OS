/**
*******************************************************************************
* @file       iFAT.c
* @version    V0.3  
* @date       2010.03.01
* @brief      interface function code of iFAT kernel.	
*******************************************************************************
* @copy
*
* Author email: piaolingsky@gmail.com
* 
* COPYRIGHT 2010 Tomy.Gu 
*******************************************************************************
*/ 


/*---------------------------- Include ---------------------------------------*/
#include "iFAT.h"
#include "iFAT32.h"
#include <string.h>



/**
*******************************************************************************
* @brief      Get file name	from file path
* @param[in]  inFname         file path
* @param[out] outFname        file name
* @retval     inFname         sub file path
*
* @par Description
* @details    This function is called to get file name	from file path. 
* @note 
*******************************************************************************
*/
static char* iFAT_GetFileName(char* inFname,char* outFname)
{
	while ((*inFname != 0) && (*inFname != '\\') && (*inFname != '/'))
	{
		*(outFname++) = *(inFname++);
	}

	if (*inFname != 0)
	{
		*outFname = 0;
		inFname++;
	}
	return inFname;
}



/**
*******************************************************************************
* @brief      Analyse file name	and return name attribute
* @param[in]  fp              IFile struct pointer
* @param[out] None
* @retval     ILLEGAL_NAME    illegal file name    
* @retval     LONG_NAME       long file name
* @retval     SHORT_NAME      short file name
*
* @par Description
* @details    This function is called to analyse file name and return name attribute. 
* @note       Don't support '.' and ' ' in file name
*******************************************************************************
*/
static char iFAT_AnalyseFileName(P_IFile fp)
{
	unsigned char len;
	unsigned char i;
	fp->fileNameLen = strlen(fp->fileName);
	for (len=(fp->fileNameLen-1);(*(fp->fileName+len)) != '.';len--);

	for (i=0;i<len;i++)
	{
		//若短文件名中有空格和'.'，一样创建长文件名
		if ((*(fp->fileName+i) == '.') || (*(fp->fileName+len) == 0x20))
		{
			return ILLEGAL_NAME;
		}
	}
	if (len>8)
	{
		return LONG_NAME;
	}
	return SHORT_NAME;
}



/**
*******************************************************************************
* @brief      Initialize IFile struct
* @param[in]  pFat     IFAT struct pointer       
* @param[out] fp       IFile struct pointer
* @retval     None
*
* @par Description
* @details    This function is called to initialize IFile struct. 
* @note        
*******************************************************************************
*/
static void iFAT_InitIFile(P_IFAT pFat, P_IFile fp)
{
	fp->pFat           = pFat;
	fp->curLocation    = 0;
	fp->fileSize       = 0;
	fp->fileClusterNum = 0;
	fp->ItemCluster    = 0;
	fp->ItemNum        = 0;
	fp->hasModify      = FALSE;
	pFat->hasModify    = FALSE;
	pFat->bufferCluster= INVALID_VALUE;
}



/**
*******************************************************************************
* @brief      Get Date Timestamp
* @param[in]  year     year  
* @param[in]  month    month
* @param[in]  date     date
* @param[out] None     
* @retval     rc       Timestamp
*
* @par Description
* @details    This function is called to Get Date Timestamp. 
* @note        
*******************************************************************************
*/
U16 iFAT_GetDateTimestamp(U16 year,U16 month,U16 date)
{
	U16 rc;
	rc = (year-1980)*512+month*32+date;
	return rc;
}



/**
*******************************************************************************
* @brief      Get Timestamp
* @param[in]  hour     hour  
* @param[in]  minute   minute
* @param[in]  second   second
* @param[out] None     
* @retval     rc       Timestamp
*
* @par Description
* @details    This function is called to Get Timestamp. 
* @note        
*******************************************************************************
*/
U16 iFAT_GetTimestamp(U16 hour,U16 minute ,U16 second)
{
	U16 rc;
	rc = hour*2048+minute*32+second/2;
	return rc;
}



/**
*******************************************************************************
* @brief      Get time form Timestamp 
* @param[in]  dataTimestamp      dataTimestamp  
* @param[in]  timestamp          timestamp
* @param[out] iTime              time struct pointer     
* @retval     None
*
* @par Description
* @details    This function is called to Get time form Timestamp. 
* @note        
*******************************************************************************
*/
void iFAT_GetTimeFromTimestamp(P_ITIME iTime,U16 dataTimestamp,U16 timestamp)
{
	iTime->date  = dataTimestamp&0x001f;
	iTime->month = (dataTimestamp>>5)&0x000f;
	iTime->year  = (dataTimestamp>>9) + 1980;

	iTime->second = (timestamp&0x001f)*2;
	iTime->minute = (timestamp>>5)&0x003f;
	iTime->hour   = timestamp>>11;
}


/**
*******************************************************************************
* @brief      Initialize iFAT
* @param[in]  None      
* @param[out] pFat        IFAT struct pointer 
* @retval     TRUE        Initialize successfully  
* @retval     FALSE       Initialize fail
*
* @par Description
* @details    This function is called to Initialize iFAT. 
* @note        
*******************************************************************************
*/
bool iFAT_Init(P_IFAT pFat)
{
	return iFAT32_Init(pFat);
}


/**
*******************************************************************************
* @brief      Uninstall iFAT
* @param[in]  None      
* @param[out] None   
* @retval     None
*
* @par Description
* @details    This function is called to Uninstall iFAT. 
* @note        
*******************************************************************************
*/
void iFAT_Uninstall(void)
{
	iFAT32_Uninstall();
}



/**
*******************************************************************************
* @brief      Open file
* @param[in]  pFat     IFAT struct pointer       
* @param[in]  name     file path   
* @param[out] fp       IFile struct pointer
* @retval     TRUE     Find file and open it. 
* @retval     FALSE    No such file
*
* @par Description
* @details    This function is called to open file. 
* @note       If file name don't start with '/' or '\', that show find file
under current directory, else find from root directory.
*******************************************************************************
*/
bool iFAT_OpenFile(P_IFAT pFat, P_IFile fp,const char* name)
{
	char* fname;
	iFAT_InitIFile(pFat, fp);
	strcpy(fp->fileName,name);
	fname = fp->fileName;
	if ((*fname == '\\') || (*fname == '/'))
	{
		pFat->curCluster = pFat->rootDirCluster;
		fname = fname+1;
	}
	else
	{
		pFat->curCluster = pFat->curDirCluster;
	}

	while (*fname != 0)
	{
		fname = iFAT_GetFileName(fname,fp->fileName);
		if(iFAT32_SpreadAllOverDir(pFat,fp) == FALSE)
		{
			return FALSE;
		}
		else
		{
			pFat->curCluster = fp->fileClusterNum;
		}
	}
	return TRUE;
}




/**
*******************************************************************************
* @brief      Create file
* @param[in]  pFat     IFAT struct pointer       
* @param[in]  fname    file name   
* @param[in]  fTime    file create time  
* @param[out] fp       IFile struct pointer
* @retval     TRUE     Create file successfully. 
* @retval     FALSE    Create file fail.
*
* @par Description
* @details    This function is called to Create file. 
* @note       Don't support create folder,and create file under current directory.
*******************************************************************************
*/
bool iFAT_CreateFile(P_IFAT pFat, P_IFile fp, const char* fname, P_ITIME iTime)
{
	char rc;
	U16  date;
	U16  time;
	iFAT_InitIFile(pFat, fp);
	strcpy(fp->fileName,fname);
	pFat->curCluster = pFat->curDirCluster;
	if (iFAT32_SpreadAllOverDir(pFat,fp) == TRUE)
	{
		// 文件已存在，返回创建错误
		return FALSE;
	}

	if ((iTime->hour>23) || (iTime->minute>59) || (iTime->second>59))
	{
		return 0;
	}

	if ((iTime->year>2099) || (iTime->month>12) || (iTime->date>31))
	{
		return 0;
	}

	date = iFAT_GetDateTimestamp(iTime->year,iTime->month,iTime->date);
	time = iFAT_GetTimestamp(iTime->hour,iTime->minute,iTime->second);

	// 创建文件
	fp->fileCreatDate = date;
	fp->fileCreatTime = time;
	fp->fileModifyDate = date;
	fp->fileModifyTime = time; 
	fp->fileAttr      = FILE_ARCH;
	rc = iFAT_AnalyseFileName(fp);
	if (rc == SHORT_NAME)
	{
		iFAT32_CreateShortNameFile(pFat,fp);
	}
	else if(rc == LONG_NAME)
	{
		iFAT32_CreateLongNameFile(pFat,fp);
	}
	else
	{
		return FALSE;
	}
	iFAT32_FlushData(fp);
	return TRUE;
}


/**
*******************************************************************************
* @brief      Close file
* @param[in]  fp       IFile struct pointer      
* @param[out] None
* @retval     TRUE     Close file successfully. 
* @retval     FALSE    Close file fail. 
*
* @par Description
* @details    This function is called to rewrite buffer data. 
* @note       
*******************************************************************************
*/
bool iFAT_CloseFile(P_IFile fp)
{
	iFAT32_FlushData(fp);
	return TRUE;
}


/**
*******************************************************************************
* @brief      Get File CreatTime
* @param[in]  fp       IFile struct pointer   
* @param[in]  iTime    CreatTime struct pointer  
* @param[out] None
* @retval     None
*
* @par Description
* @details    This function is called to Get File CreatTime. 
* @note       
*******************************************************************************
*/
void iFAT_GetFileCreatTime(P_IFile fp,P_ITIME iTime)
 {
	 iFAT_GetTimeFromTimestamp(iTime,fp->fileCreatDate,fp->fileCreatTime);
 }


/**
*******************************************************************************
* @brief      Get File ModifyTime
* @param[in]  fp       IFile struct pointer   
* @param[in]  iTime    ModifyTime struct pointer  
* @param[out] None
* @retval     None
*
* @par Description
* @details    This function is called to Get File ModifyTime. 
* @note       
*******************************************************************************
*/
 void iFAT_GetFileModifyTime(P_IFile fp,P_ITIME iTime)
 {
	 iFAT_GetTimeFromTimestamp(iTime,fp->fileModifyDate,fp->fileModifyTime);
 }



/**
*******************************************************************************
* @brief      Delete file
* @param[in]  pFat     IFAT struct pointer       
* @param[in]  fpath    file path 
* @param[out] None
* @retval     TRUE     Delete file successfully. 
* @retval     FALSE    No such file. 
*
* @par Description
* @details    This function is called to delete file. 
* @note       
*******************************************************************************
*/
bool  iFAT_DeleteFile(P_IFAT pFat,const char* fpath)
{
	IFile file;
	strcpy(file.fileName,fpath);
	if (iFAT_OpenFile(pFat,&file,fpath) == FALSE)
	{
		return FALSE;
	}
	iFAT32_DeleteFile(&file);
	return TRUE;

}



/**
*******************************************************************************
* @brief      Seek file location and set it as current file location.
* @param[in]  fp         IFile struct pointer      
* @param[in]  offset     Offset location
* @param[in]  fromWhere  Start to seek location 
* @param[out] None
* @retval     TRUE       Seek location successfully. 
* @retval     FALSE      No such location.
*
* @par Description
* @details    This function is called to seek file location 
and set it as current file location.
* @note       
*******************************************************************************
*/
bool iFAT_SeekFile(P_IFile fp,DWORD offset,char fromWhere)
{
	if (fp==NULL)
	{
		return FALSE;
	}
	if (fromWhere == FILE_STR)
	{
		if (offset>(fp->fileSize))
		{
			return FALSE;
		}
		else
		{
			fp->curLocation = offset;
		}
	}
	else if (fromWhere == FILE_END)
	{
		if (offset>(fp->fileSize))
		{
			return FALSE;
		}
		else
		{
			fp->curLocation = fp->fileSize - offset;
		}
	}
	else if (fromWhere == FILE_CUR)
	{
		if ((fp->curLocation+offset)>(fp->fileSize))
		{
			return FALSE;
		}
		else
		{
			fp->curLocation = fp->curLocation + offset;
		}
	}
	return TRUE;
}



/**
*******************************************************************************
* @brief      Read file content.
* @param[in]  fp          IFile struct pointer      
* @param[in]  size        Content length
* @param[out] buf         Content buffer
* @retval     hadReadLen  Content length that had read.
*
* @par Description
* @details    This function is called to read file content.
* @note       
*******************************************************************************
*/
DWORD iFAT_ReadFile(P_IFile fp,char* buf, DWORD size)
{
	DWORD hadReadLen;
	hadReadLen =  iFAT32_FileRead(fp,buf,size);
	fp->curLocation = fp->curLocation + hadReadLen;
	return hadReadLen;
}


/**
*******************************************************************************
* @brief      Write file content.
* @param[in]  fp          IFile struct pointer      
* @param[in]  size        Content length
* @param[in]  buf         Content buffer
* @param[out] None
* @retval     hadWriteLen  Content length that had written.
*
* @par Description
* @details    This function is called to write file content.
* @note       Only support to write content at the end of file.
*******************************************************************************
*/
DWORD iFAT_WriteFile(P_IFile fp,char* buf, DWORD size)
{
	DWORD hadWriteLen;
	hadWriteLen = iFAT32_FileWrite(fp,buf,size);
	fp->curLocation = fp->fileSize;
	return hadWriteLen;
}


/**
*******************************************************************************
* @brief      Start to Write Bulk content into File
* @param[in]  fp          IFile struct pointer      
* @param[in]  size        Content length
* @param[out] rc          return value
* @retval     &iBulkT     Bulk write struct pointer
*
* @par Description
* @details    This function is called to start to Write Bulk content into File.
* @note       Only support to write file that size is 0.
*******************************************************************************
*/
P_BulkT iFAT_BulkWriteFile(P_IFile fp,DWORD size,U8* rc)
{
	if (fp == NULL )
	{
		*rc =E_INVALID_PARAMETER;
		return NULL;
	}
	if (size == 0)
	{
		*rc =E_INVALID_PARAMETER;
		return NULL;
	}

	return iFAT32_BulkWriteFile(fp,size,rc);
}


/**
*******************************************************************************
* @brief      Write Bulk content into File
* @param[in]  pBulk       Bulk write struct pointer     
* @param[in]  size        Content length
* @param[in]  buf         Content buffer
* @param[out] None
* @retval     hadWriteLen  Content length that had written.
*
* @par Description
* @details    This function is called to start to Write Bulk content into File.
* @note       If hadWriteLen smaller than size, this show Bulk Write end.
*******************************************************************************
*/
DWORD iFAT_DoBulkWrite(P_BulkT pBulk,char* buf,DWORD size)
{
	if (pBulk == NULL)
	{
		return 0;
	}
	if (buf == NULL)
	{
		return 0;
	}
	if (size == 0)
	{
		return 0;
	}
	return iFAT32_DoBulkWrite(pBulk,buf,size);
}





/**
*******************************************************************************
* @brief      Set current directory
* @param[in]  pFat     IFAT struct pointer       
* @param[in]  dirName  directory path
* @param[out] None
* @retval     TRUE     Successfully
* @retval     FALSE    No such directory
*
* @par Description
* @details    This function is called to set current directory.
* @note       
*******************************************************************************
*/
bool iFAT_SetCurDir(P_IFAT pFat, char* dirPath)
{
	IFile file;
	strcpy(file.fileName,dirPath);
	if (iFAT_OpenFile(pFat,&file,dirPath) == FALSE)
	{
		return FALSE;
	}
	strcpy(pFat->curDir,file.fileName);
	pFat->curDirCluster = file.fileClusterNum;
	return TRUE;

}


/**
*******************************************************************************
* @brief      Show current directory
* @param[in]  pFat     IFAT struct pointer   
* @param[out] None
* @retval     None     
*
* @par Description
* @details    This function is called to show current directory.
* @note       
*******************************************************************************
*/
void iFAT_ShowCurDir(P_IFAT pFat)
{
	pFat->curCluster = pFat->curDirCluster;
	iFAT32_ShowCurDirList(pFat);
}

