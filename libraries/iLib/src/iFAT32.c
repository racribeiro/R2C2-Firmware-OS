/**
*******************************************************************************
* @file       iFAT32.c
* @version    V0.3  
* @date       2010.03.01
* @brief      interface function code of iFAT32 kernel.	
*******************************************************************************
* @copy
*
* Author email: piaolingsky@gmail.com
* 
* COPYRIGHT 2010 Tomy.Gu 
*******************************************************************************
*/ 


/*---------------------------- Include ---------------------------------------*/
#include "iFAT32.h"
#include "iDisk.h"
#include <string.h> 


/*---------------------------- Function declare-------------------------------*/
extern WORD UnicodeToGB2312(WORD unicode);
extern WORD GB2312ToUnicode(WORD data);


/*---------------------------- Variable Define -------------------------------*/
//Disk  MBR±êÖ¾£º 3ÀŽÐ¼
const char MBRFlag[6]  = {0x33, 0xC0, 0x8E, 0xD0, 0xBC, 0x00};
//FAT32 DBR±êÖ¾£º ëX�MSDOS5.0
const char DBRFlag[12] = {0xEB, 0x58, 0x90, 0x4D, 0x53, 0x44, 0x4F, 0x53, 0x35, 0x2E, 0x30, 0x00};


IFat32  iFat;
IFile   iFile;
BulkT   iBulkT;

/**
*******************************************************************************
* @brief      Get capital
* @param[in]  character     original character
* @param[out] None
* @retval     character     capital
*
* @par Description
* @details    This function is called to get capital. 
* @note       If not lowercase,return itself
*******************************************************************************
*/
static char iFat32_GetCapital(char character)
{
	if ((character>='a') && (character<='z'))
	{
		character = character - 0x20;
	}
	return character;
}


/**
*******************************************************************************
* @brief      Compare  character
* @param[in]  character1     character1
* @param[in]  character2     character2
* @param[out] None
* @retval     TURE           Same character
* @retval     FALSE          Different character
*
* @par Description
* @details    This function is called to compare character. 
* @note       
*******************************************************************************
*/
static bool iFat32_IsSameCharacter(char character1,char character2)
{
	if (character1 == character2)
	{
		return TRUE;
	}
	if ((character1>='a') && (character1<='z'))
	{
		if (character1 == (character2+0x20))
		{
			return TRUE;
		}
		else
		{
			return FALSE;
		}
	}
	if ((character1>='A') && (character1<='Z'))
	{
		if (character1 == (character2-0x20))
		{
			return TRUE;
		}
		else
		{
			return FALSE;
		}
	}
	return FALSE;
}


/**
*******************************************************************************
* @brief      Compare file name
* @param[in]  fname1         file name 1
* @param[in]  fname2         file name 2
* @param[out] None
* @retval     TURE           Same file name
* @retval     FALSE          Different file name
*
* @par Description
* @details    This function is called to compare file name. 
* @note       
*******************************************************************************
*/
static bool iFat32_NameCmp(char* fname1,char* fname2)
{
	while((*fname1) !=  0)
	{
		if (iFat32_IsSameCharacter(*fname1,*fname2) == FALSE)
		{
			return FALSE;
		}
		fname1++;
		fname2++;
	}
	if ((*fname2) !=  0)
	{
		return FALSE;
	}
	return TRUE;
}


/**
*******************************************************************************
* @brief      Write FAT section
* @param[in]  secNum      offest section number in FAT
* @param[in]  buf         content buffer
* @param[out] None
* @retval     None
*
* @par Description
* @details    This function is called to write FAT section. 
* @note 
*******************************************************************************
*/
static void iFat32_WriteFAT(DWORD secNum,char* buf)
{
	DWORD addr;
	addr = iFat.secSize * (secNum+iFat.fat1Sec);
	iDisk_WriteBlock(addr,buf,iFat.secSize);
	if (iFat.fatNum == 2)
	{
		addr = iFat.secSize * (secNum+iFat.fat2Sec);
		iDisk_WriteBlock(addr,buf,iFat.secSize);
	}
	iFat.isDirty = FALSE;
}


/**
*******************************************************************************
* @brief      Read FAT section
* @param[in]  secNum      offest section number in FAT
* @param[out] buf         content buffer
* @retval     None
*
* @par Description
* @details    This function is called to read FAT section. 
* @note 
*******************************************************************************
*/
static void iFat32_ReadFAT(DWORD secNum,char* buf)
{
	DWORD addr;
	addr = iFat.secSize * (secNum+iFat.fat1Sec);
	iDisk_ReadBlock(addr,buf,iFat.secSize);
}



/**
*******************************************************************************
* @brief      Get section number from cluster number
* @param[in]  cluNum      cluster number
* @param[out] None
* @retval     secNum      section number
*
* @par Description
* @details    This function is called to get section number. 
* @note 
*******************************************************************************
*/
DWORD iFAT32_GetSecFromCluster(DWORD clusterNum)
{
	DWORD secNum;
	secNum = (clusterNum - iFat.rootDirCluNum)*iFat.secNumInCluster + iFat.rootDirSec;
	return secNum;
}


/**
*******************************************************************************
* @brief      Write disk cluster
* @param[in]  cluNum      cluster number
* @param[in]  buf         content buffer
* @param[out] None
* @retval     None
*
* @par Description
* @details    This function is called to write disk cluster. 
* @note 
*******************************************************************************
*/
void iFAT32_WriteCluster(P_IFAT pFat,DWORD cluNum)
{
	DWORD addr;
	if (cluNum != pFat->bufferCluster)
	{
		if (pFat->hasModify == TRUE)
		{
			addr = iFAT32_GetSecFromCluster(pFat->bufferCluster)*iFat.secSize;
			iDisk_WriteBuffer(addr, pFat->optBuf,iFat.secNumInCluster*iFat.secSize);
		}
		pFat->bufferCluster = cluNum;
	}
	pFat->hasModify = TRUE;	
}


/**
*******************************************************************************
* @brief      Read disk cluster
* @param[in]  cluNum      cluster number
* @param[out] buf         content buffer
* @retval     None
*
* @par Description
* @details    This function is called to read disk cluster. 
* @note 
*******************************************************************************
*/
void iFAT32_ReadCluster(P_IFAT pFat,DWORD cluNum)
{
	DWORD addr;
	if (cluNum != pFat->bufferCluster)
	{
		if (pFat->hasModify == TRUE)
		{
			addr = iFAT32_GetSecFromCluster(pFat->bufferCluster)*iFat.secSize;
			iDisk_WriteBuffer(addr, pFat->optBuf,iFat.secNumInCluster*iFat.secSize);
		}

		addr = iFAT32_GetSecFromCluster(cluNum)*iFat.secSize;
		iDisk_ReadBuffer(addr, pFat->optBuf,iFat.secNumInCluster*iFat.secSize);
		pFat->bufferCluster = cluNum;
		pFat->hasModify = FALSE;
	}	
}

/**
*******************************************************************************
* @brief      Read disk cluster that never used 
* @param[in]  cluNum      cluster number
* @param[out] buf         content buffer
* @retval     None
*
* @par Description
* @details    This function is called to read disk cluster that never used. 
* @note 
*******************************************************************************
*/
void iFAT32_ReadNewCluster(P_IFAT pFat,DWORD cluNum)
{
	DWORD addr;
	if (cluNum != pFat->bufferCluster)
	{
		if (pFat->hasModify == TRUE)
		{
			addr = iFAT32_GetSecFromCluster(pFat->bufferCluster)*iFat.secSize;
			iDisk_WriteBuffer(addr, pFat->optBuf,iFat.secNumInCluster*iFat.secSize);
		}
		memset(pFat->optBuf,0,iFat.secNumInCluster*iFat.secSize);
		pFat->bufferCluster = cluNum;
		pFat->hasModify = FALSE;
	}	
}




/**
*******************************************************************************
* @brief      Analyse MBR(master boot record) section
* @param[in]  Buffer         MBR section buffer
* @param[out] None
* @retval     secNum         DBR section number
*
* @par Description
* @details    This function is called to analyse MBR(master boot record) section. 
* @note 
*******************************************************************************
*/
static DWORD iFAT32_MBR(char* Buffer)
{
	DWORD dbrSec;
	DWORD offsetAddr;
	U8    num;

	if (strcmp((const char*)Buffer,DBRFlag) == 0)
	{
		return 0;
	}
	else
	{
		offsetAddr = 0x1BE;
		for(num=0;num<4;num++)
		{
			if(*(Buffer+offsetAddr) == 0x80)
			{
				dbrSec  = (*(Buffer+offsetAddr+11)<<24)|(*(Buffer+offsetAddr+10)<<16)|(*(Buffer+offsetAddr+9)<<8)|(*(Buffer+offsetAddr+8));
				return dbrSec;
			}
			else
			{
				offsetAddr = offsetAddr + 0x10;
			}
		}
	}
	return INVALID_VALUE;
}


/**
*******************************************************************************
* @brief      Analyse DBR(DOS BOOT RECORD) section
* @param[in]  Buffer      DBR section buffer
* @param[out] None
* @retval     TRUE        Successfully
* @retval     FALSE       Fail
*
* @par Description
* @details    This function is called to analyse DBR(DOS BOOT RECORD) section,
and get disk use status.
* @note 
*******************************************************************************
*/
bool iFAT32_DBR(char* Buffer)
{
	DWORD secNum;
	DWORD itemNum;
	DWORD clusterNum;
	DWORD clusterValue;
	bool  flag;
	if (strcmp((const char*)Buffer,DBRFlag) != 0)
	{
		return FALSE;
	}

	iFat.secNumInCluster =  (*(Buffer+0x0d));
	iFat.fatNum          =  (*(Buffer+0x10));
	iFat.secSize         =  (*(Buffer+0x0c)<<8) |(*(Buffer+0x0b));
	iFat.fat1Sec         =  (*(Buffer+0x0f)<<8) |(*(Buffer+0x0e));
	iFat.hiddenSec       =  (*(Buffer+0x1f)<<24)|(*(Buffer+0x1e)<<16)|(*(Buffer+0x1d)<<8)|(*(Buffer+0x1c));
	iFat.secNum          =  (*(Buffer+0x23)<<24)|(*(Buffer+0x22)<<16)|(*(Buffer+0x21)<<8)|(*(Buffer+0x20));
	iFat.fatSecNum       =  (*(Buffer+0x27)<<24)|(*(Buffer+0x26)<<16)|(*(Buffer+0x25)<<8)|(*(Buffer+0x24));
	iFat.rootDirCluNum   =  (*(Buffer+0x2F)<<24)|(*(Buffer+0x2E)<<16)|(*(Buffer+0x2B)<<8)|(*(Buffer+0x2C));
	iFat.fat1Sec         =  iFat.hiddenSec + iFat.fat1Sec;
	iFat.rootDirSec      =  iFat.fat1Sec + iFat.fatSecNum*iFat.fatNum;
	iFat.maxItemNum      =  (iFat.secSize*iFat.secNumInCluster)/32;

	if (iFat.fatNum == 2)
	{
		iFat.fat2Sec = iFat.fat1Sec + iFat.fatSecNum;
	}

	//´Ë´¦ÊÇÒ»ÕÅSD¿¨¸ÕÐ´ÈëÊý¾ÝÊ±FAT±íÐèÒªÐÞ¸ÄµÄµØ·½£¬½«´Ø1×öÎªÏµÍ³±£Áô´Ø
	iFat32_ReadFAT(0,Buffer);
	*(DWORD*)(Buffer+4) = 0xFFFFFFFF;
	iFat32_WriteFAT(0,Buffer);

	//ÉèÖÃ»º³åÇøÎªÎÞÐ§
	iFat.cluSec = INVALID_VALUE;


	//»ñµÃÊ£Óà¿Õ¼ä
	flag = FALSE;
	clusterNum = 0;
	iFat.vacantClusters = 0;
	iFat.assignCluster  = 0;
	iFat.maxAvailableCluster = iFat.fatSecNum*4;
	for (secNum=0;secNum<iFat.fatSecNum;secNum++)
	{
		iFat32_ReadFAT(secNum,Buffer);
		for (itemNum=0;itemNum<iFat.secSize;itemNum=itemNum+4)
		{
			clusterValue = *(DWORD*)(Buffer+itemNum);
			if (clusterValue == 0)
			{
				iFat.vacantClusters++;
				if (flag == FALSE)
				{
					iFat.assignCluster = clusterNum;
					flag = TRUE;
				}
			}
			else
			{
				flag = FALSE;
				iFat.assignSize = clusterNum - iFat.assignCluster;
			}
			clusterNum++;
		}
	}
	if (flag == TRUE)
	{
		iFat.assignSize = clusterNum - iFat.assignCluster;
	}
	return TRUE;
}


/**
*******************************************************************************
* @brief      Set iFat struct
* @param[in]  pFat     IFAT struct pointer  
* @param[out] None
* @retval     None      
*
* @par Description
* @details    This function is called to set iFat struct.
* @note 
*******************************************************************************
*/
void iFAT32_SetIFat(P_IFAT pFat)
{
	memcpy(pFat->curDir,"ROOT",5);
	pFat->rootDirCluster = iFat.rootDirCluNum;
	pFat->curDirCluster  = iFat.rootDirCluNum;

	// rootÎ»ÓÚ´Ø2
	pFat->offsetAddrInFAT = iFat.rootDirSec*4;
}



/**
*******************************************************************************
* @brief      Initialize iFAT32
* @param[in]  None      
* @param[out] pFat        IFAT struct pointer 
* @retval     TRUE        Initialize successfully  
* @retval     FALSE       Initialize fail
*
* @par Description
* @details    This function is called to initialize iFAT32. 
* @note        
*******************************************************************************
*/
bool iFAT32_Init(P_IFAT pFat)
{
	char* buf;
	DWORD dbrSec;
	DWORD blkSize;

	if (iFat.fatStatus == FAT_Initialized)
	{
		iFAT32_SetIFat(pFat);
		return TRUE;
	}

	iFat.fatStatus = FAT_Uninitialized;

	if (iDisk_Init() == FALSE)
		return FALSE;


	buf = pFat->optBuf;
	blkSize = iDisk_GetBlockSize();
	iDisk_ReadBlock(0,buf,blkSize);
	dbrSec = iFAT32_MBR(buf);
	if (dbrSec == INVALID_VALUE)
	{
		return FALSE;
	}

	iDisk_ReadBlock(dbrSec*blkSize,buf,blkSize);
	if (iFAT32_DBR(buf) == FALSE)
	{
		return FALSE;
	}

	iDisk_ReadBlock(iFat.rootDirSec*iFat.secSize,buf,iFat.secSize);

	// »ñÈ¡ÓÃ»§×Ô¶¨Òå·ÖÇøÃû
	if(*(buf+0xB) == FILE_SYS)
	{
		memcpy(iFat.PartitionName,buf,11);
	}

	iFat.fatStatus = FAT_Initialized;
	iFAT32_SetIFat(pFat);

	return TRUE;
}


/**
*******************************************************************************
* @brief      Uninstall iFAT32
* @param[in]  None      
* @param[out] None   
* @retval     None
*
* @par Description
* @details    This function is called to Uninstall iFAT32. 
* @note        
*******************************************************************************
*/
void iFAT32_Uninstall(void)
{
	iFat.fatStatus = FAT_Uninitialized;
}




/**
*******************************************************************************
* @brief      Get next cluster
* @param[in]  cluNum      current cluster number      
* @param[out] None
* @retval     nextCluster next cluster number
*
* @par Description
* @details    This function is called to get next cluster. 
* @note        
*******************************************************************************
*/
DWORD iFAT32_GetNextCluster(DWORD cluNum)
{
	DWORD cluSec;
	DWORD cluOffset;
	DWORD nextCluster;
	cluSec    = (cluNum*4)/iFat.secSize;
	cluOffset = ((cluNum*4)%iFat.secSize)/4;
	if (cluSec != iFat.cluSec)
	{
		if (iFat.isDirty == TRUE)
		{
			iFat32_WriteFAT(iFat.cluSec,(char*)(iFat.fatBuf));
		}
		iFat32_ReadFAT(cluSec,(char*)(iFat.fatBuf));
		iFat.cluSec = cluSec;
	}
	nextCluster = *(iFat.fatBuf+cluOffset);
	return nextCluster;
}


/**
*******************************************************************************
* @brief      Get new cluster
* @param[in]  preCluster         precede cluster number      
* @param[out] None
* @retval     FAT_InvalidValue   No vacant cluster
* @retval     newCluster         new cluster number
*
* @par Description
* @details    This function is called to get new cluster. 
* @note        
*******************************************************************************
*/
DWORD iFTA32_GetNewCluster(DWORD preCluster)
{
	DWORD  newCluster;
	DWORD  secNum;
	DWORD  preSecNum;
	DWORD  itemNum;
	DWORD* Buffer;
	//³ýÁË×îºóÒ»´ó¿é£¬ÆäÓàË³Ðò²éÕÒ
	if (iFat.vacantClusters == 0)
	{
		return FAT_InvalidValue;
	}
	if (iFat.assignSize != 0)
	{
		newCluster = iFat.assignCluster;
		iFat.assignSize--;
		if (iFat.assignSize == 0)
		{
			iFat.assignCluster = 0;
		}
		else
		{
			iFat.assignCluster++;
		}		
	}
	else
	{
		Buffer = iFat.fatBuf;

		for (secNum=(iFat.assignCluster)/iFat.secSize;secNum<iFat.fatSecNum;secNum++)
		{
			iFat32_ReadFAT(secNum,(char*)Buffer);
			for (itemNum=0;itemNum<(iFat.secSize/4);itemNum++)
			{
				if (*(Buffer+itemNum) == 0)
				{
					newCluster = secNum*(iFat.secSize/4) + itemNum;
					iFat.assignCluster = newCluster;
					goto exit;
				}
			}
		}
		preSecNum = iFat.assignCluster/iFat.secSize;
		for (secNum=0;secNum<preSecNum;secNum++)
		{
			iFat32_ReadFAT(secNum,(char*)Buffer);
			for (itemNum=0;itemNum<(iFat.secSize/4);itemNum++)
			{
				if (*(Buffer+itemNum) == 0)
				{
					newCluster = secNum*(iFat.secSize/4) + itemNum;
					iFat.assignCluster = newCluster;
					goto exit;
				}
			}
		}
	}
exit:
	// ¿ÕÏÐ´ØÊýÄ¿´¦Àí
	iFat.vacantClusters--;
	// ÉèÖÃÕâÒ»´Ø
	secNum  = (newCluster*4)/iFat.secSize;
	itemNum = ((newCluster*4)%iFat.secSize)/4;
	if (secNum != iFat.cluSec)
	{
		if (iFat.isDirty == TRUE)
		{
			iFat32_WriteFAT(iFat.cluSec,(char*)(iFat.fatBuf));
		}
		iFat32_ReadFAT(secNum,(char*)(iFat.fatBuf));
		iFat.cluSec = secNum;
	}
	*(iFat.fatBuf+itemNum) = FILE_LastCluster;
	iFat.isDirty = TRUE;

	//ÉèÖÃÇ°Ò»´Ø
	if (preCluster!=0)
	{
		secNum  = (preCluster*4)/iFat.secSize;
		itemNum = ((preCluster*4)%iFat.secSize)/4;
		if (secNum != iFat.cluSec)
		{
			if (iFat.isDirty == TRUE)
			{
				iFat32_WriteFAT(iFat.cluSec,(char*)(iFat.fatBuf));
			}
			iFat32_ReadFAT(secNum,(char*)(iFat.fatBuf));
			iFat.cluSec = secNum;
		}
		*(iFat.fatBuf+itemNum) = newCluster;
		iFat.isDirty = TRUE;
	}
	return newCluster;
}



/**
*******************************************************************************
* @brief      Get new clusters
* @param[in]  preCluster         precede cluster number      
* @param[in]  clusterNum         clusters number   
* @param[out] rc                 status return value
* @retval     FAT_InvalidValue   No vacant cluster
* @retval     newCluster         new cluster number
*
* @par Description
* @details    This function is called to get new clusters. 
* @note        
*******************************************************************************
*/
DWORD iFTA32_GetNewClusters(DWORD preCluster,DWORD clusterNum,U8* rc)
{
	DWORD newCluster;
	DWORD nextCluster;
	DWORD secNum;
	DWORD itemNum;
	DWORD addr;			  
	DWORD mClusterNum;
	char  fatNum;
	if (iFat.vacantClusters == 0)
	{
		*rc = E_SPACE_LACK;
		return INVALID_VALUE;
	}
	if (iFat.assignSize < clusterNum)
	{
		*rc = E_SPACE_DISCRETE;
		return INVALID_VALUE;
	}
	newCluster = iFat.assignCluster;
	iFat.assignSize -= clusterNum;
	if (iFat.assignSize == 0)
	{
		iFat.assignCluster = 0;
	}
	else
	{
		iFat.assignCluster += clusterNum;
	}	

	iFat.vacantClusters -= clusterNum;

	// ÉèÖÃÕâÒ»´Ø


	// Ð´ÈëFAT1
	fatNum = iFat.fatNum;
	mClusterNum = clusterNum;
	while (fatNum--)
	{
		clusterNum = mClusterNum;
		secNum  = (newCluster*4)/iFat.secSize;
		itemNum = ((newCluster*4)%iFat.secSize)/4;	

		if (secNum != iFat.cluSec)
		{
			if (iFat.isDirty == TRUE)
			{
				iFat32_WriteFAT(iFat.cluSec,(char*)(iFat.fatBuf));
			}
			iFat32_ReadFAT(secNum,(char*)(iFat.fatBuf));
			iFat.cluSec = secNum;
		}

		if (fatNum == 1)
		{
			addr = (iFat.fat2Sec+secNum)*iFat.secSize;
		}
		else 
		{
			addr = (iFat.fat1Sec+secNum)*iFat.secSize;
		}

		if (iDisk_BeginBulkWrite(addr) == FALSE)
		{
			*rc = E_DISK_OPT_ERR;
			return INVALID_VALUE;
		}
		nextCluster = newCluster+1;
		while(clusterNum)
		{
			if (clusterNum == 1)
			{
				*(iFat.fatBuf+itemNum) = FILE_LastCluster;
				iDisk_DoBulkWrite((char*)(iFat.fatBuf),iFat.secSize,0);
				break;
			}
			else
			{
				*(iFat.fatBuf+itemNum) = nextCluster;
			}
			clusterNum--;
			nextCluster++;
			itemNum++;
			if (itemNum == (iFat.secSize/4))
			{
				iDisk_DoBulkWrite((char*)(iFat.fatBuf),iFat.secSize,1);
				memset((char*)(iFat.fatBuf),0,512);
				iFat.cluSec++;
				itemNum = 0;
			}
		}
	}

	//ÉèÖÃÇ°Ò»´Ø
	if (preCluster!=0)
	{
		secNum  = (preCluster*4)/iFat.secSize;
		itemNum = ((preCluster*4)%iFat.secSize)/4;
		if (secNum != iFat.cluSec)
		{
			if (iFat.isDirty == TRUE)
			{
				iFat32_WriteFAT(iFat.cluSec,(char*)(iFat.fatBuf));
			}
			iFat32_ReadFAT(secNum,(char*)(iFat.fatBuf));
			iFat.cluSec = secNum;
		}
		*(iFat.fatBuf+itemNum) = newCluster;
		iFat.isDirty = TRUE;
	}
	return newCluster;
}



/**
*******************************************************************************
* @brief      Recover cluster
* @param[in]  cluNum            cluster number      
* @param[out] None
* @retval     None
*
* @par Description
* @details    This function is called to recover cluster. 
* @note        
*******************************************************************************
*/
void iFAT32_RecoverCluster(DWORD cluNum)
{
	DWORD cluSec;
	DWORD cluOffset;
	cluSec    = cluNum/iFat.secSize;
	cluOffset = cluNum%iFat.secSize;
	if (cluSec != iFat.cluSec)
	{
		if (iFat.isDirty == TRUE)
		{
			iFat32_WriteFAT(iFat.cluSec,(char*)(iFat.fatBuf));
		}
		iFat32_ReadFAT(cluSec,(char*)(iFat.fatBuf));
		iFat.cluSec = cluSec;
	}
	*(iFat.fatBuf+cluOffset) = 0;
	iFat.isDirty = TRUE;
}


/**
*******************************************************************************
* @brief      Get father directory
* @param[in]  buf         content buffer      
* @param[out] pFat        IFAT struct pointer 
* @retval     TRUE        Initialize successfully  
* @retval     FALSE       Initialize fail
*
* @par Description
* @details    This function is called to get father directory. 
* @note        
*******************************************************************************
*/
void iFAT32_GetFatherDir(P_IFAT pFat,char* buf)
{
	DWORD clusterNum;
	clusterNum = (*(buf+0x15)<<24)|(*(buf+0x14)<<16)|(*(buf+0x1b)<<8)|(*(buf+0x1a)); 
	pFat->fatherDirAddr = iFAT32_GetSecFromCluster(clusterNum);
}


/**
*******************************************************************************
* @brief      Recover file cluster
* @param[in]  fileCluster       file first cluster      
* @param[out] None
* @retval     None
*
* @par Description
* @details    This function is called to recover file cluster. 
* @note        
*******************************************************************************
*/
void iFAT32_RecoverFileCluster(DWORD fileCluster)
{
	DWORD nextCluster;
	do 
	{
		nextCluster = iFAT32_GetNextCluster(fileCluster);
		iFAT32_RecoverCluster(fileCluster);
		fileCluster = nextCluster;
	} while (nextCluster != FILE_LastCluster);
}



/**
*******************************************************************************
* @brief      Flush FAT buffer data
* @param[in]  None
* @param[out] None
* @retval     None
*
* @par Description
* @details    This function is called to flush FAT buffer data. 
* @note        
*******************************************************************************
*/
void iFAT32_FlushData(P_IFile fp)
{
	DWORD addr;
	DWORD length;
	DWORD clusterSize;
	DWORD clusterNum;
	DWORD recoverCluster;
	char* buffer;
	char item[32];

	if (iBulkT.fp == fp)
	{
		if (iBulkT.residueLength != 0)
		{
			if (iBulkT.bufferOffset!=0)
			{
				iDisk_DoBulkWrite(iBulkT.buffer,iBulkT.bufferSize,0);
			}
			fp->fileSize = iBulkT.totalSize - iBulkT.residueLength;
			length = fp->fileSize;
			clusterSize    = iFat.secSize*iFat.secNumInCluster;
			recoverCluster = (length+clusterSize-1)/clusterSize +fp->fileClusterNum;
			clusterNum     = (iBulkT.totalSize+clusterSize-1)/clusterSize +fp->fileClusterNum -recoverCluster;
			if (fp->fileSize == 0)
			{
				fp->fileClusterNum = 0;
			}
			while(clusterNum--)
			{
				iFAT32_RecoverCluster(recoverCluster++);
			}
		}
		iBulkT.fp = NULL;
	}

	if (iFat.isDirty == TRUE)
	{
		iFat32_WriteFAT(iFat.cluSec,(char*)(iFat.fatBuf));
	}

	if (fp->hasModify == TRUE)
	{
		iFAT32_ReadCluster(fp->pFat,fp->ItemCluster);
		buffer = fp->pFat->optBuf;
		memcpy(item,buffer+32*(fp->ItemNum),32);	

		item[0x15] = (fp->fileClusterNum & 0xff000000)>>24;
		item[0x14] = (fp->fileClusterNum & 0x00ff0000)>>16;
		item[0x1B] = (fp->fileClusterNum & 0x0000ff00)>>8;
		item[0x1A] = (fp->fileClusterNum & 0x000000ff);

		item[0x1C] = (fp->fileSize & 0x000000ff);
		item[0x1D] = (fp->fileSize & 0x0000ff00)>>8;
		item[0x1E] = (fp->fileSize & 0x00ff0000)>>16;
		item[0x1F] = (fp->fileSize & 0xff000000)>>24;
		memcpy(buffer+32*(fp->ItemNum),item,32);
		iFAT32_WriteCluster(fp->pFat,fp->ItemCluster);
		fp->hasModify = FALSE;
	}
	if (fp->pFat->hasModify == TRUE)
	{
		addr = iFAT32_GetSecFromCluster(fp->pFat->bufferCluster)*iFat.secSize;
		iDisk_WriteBuffer(addr, fp->pFat->optBuf,iFat.secNumInCluster*iFat.secSize);
		fp->pFat->hasModify = FALSE;
	}
}



/**
*******************************************************************************
* @brief      Get short name check num
* @param[in]  shortname  short file name
* @param[out] None
* @retval     chknum     check num
*
* @par Description
* @details    This function is called to get short name check num. 
* @note        
*******************************************************************************
*/
U8 iFAT32_GetChkNum(char* shortname)
{
	U8 chknum;
	U8 i,j;
	chknum=i=j=0;
	for (i=11; i>0; i--)
		chknum = ((chknum & 1) ? 0x80 : 0) + (chknum >> 1) + shortname[j++];
	return chknum;
}



/**
*******************************************************************************
* @brief      Dispose short file name
* @param[in]  Buffer         file item buffer
* @param[in]  shortFileName  is short file item for long file name
* @param[out] None
* @retval     None
*
* @par Description
* @details    This function is called to dispose short file name. 
* @note        
*******************************************************************************
*/
void iFAT32_DisposeShortFileName(char* Buffer,bool shortFileName)
{
	char num,i;
	iFile.fileAttr        = *(Buffer+0x0b);
	iFile.fileCreatTime   = (*(Buffer+0x0f)<<8) |(*(Buffer+0x0e)); 
	iFile.fileCreatDate   = (*(Buffer+0x11)<<8) |(*(Buffer+0x10)); 
	iFile.fileModifyTime  = (*(Buffer+0x17)<<8) |(*(Buffer+0x16)); 
	iFile.fileModifyDate  = (*(Buffer+0x19)<<8) |(*(Buffer+0x18));  
	iFile.fileClusterNum  = (*(Buffer+0x15)<<24)|(*(Buffer+0x14)<<16)|(*(Buffer+0x1b)<<8)|(*(Buffer+0x1a));
	iFile.fileSize        = (*(Buffer+0x1F)<<24)|(*(Buffer+0x1E)<<16)|(*(Buffer+0x1D)<<8)|(*(Buffer+0x1C));


	if (shortFileName == TRUE )
	{
		for (num=0;num<8;num++)
		{
			if (*(Buffer+num)==0x20)
			{
				break;
			}
			iFile.fileName[num] = *(Buffer+num);
		}
		if (iFile.fileAttr != FILE_DIR)
		{
			iFile.fileName[num++] = '.';
			for (i=0;i<3;i++,num++)
			{
				if (*(Buffer+i+8)==0x20)
				{
					break;
				}
				iFile.fileName[num] = *(Buffer+i+8);
			}
		}
		iFile.fileName[num] = 0;
		iFile.fileNameLen = num;
	}

}



/**
*******************************************************************************
* @brief      Dispose long file name
* @param[out] pFat           IFAT struct pointer 
* @param[in]  buf            file item buffer
* @param[in]  itemNum        file item offset in buffer
* @param[out] itemNum        file item offset in buffer
* @retval     Item_Invalid   No matching  short file name
* @retval     Item_Valid     Matching  short file name
*
* @par Description
* @details    This function is called to dispose long file name. 
* @note        
*******************************************************************************
*/
char iFAT32_DisposeLongFileName(P_IFAT pFat,char* buf,DWORD* itemNum)
{
	unsigned char fileNameLen;
	unsigned char checkNum;
	unsigned char num;
	WORD     data;
	char     i;
	char*    pBuf;

	// »ñµÃ³¤ÎÄ¼þÃû×Ö£¬UnicodeÐÍ
	pBuf = buf + 32*(*itemNum);
	while (*(pBuf+0xB) == 0x0F)
	{
		// ÅÐ¶ÏÊÇ·ñÎª×îºóÒ»Ïî
		if (((*pBuf)&0x40) == 0x40)
		{
			fileNameLen = ((*pBuf)&0x1f) * 26;
			checkNum    = *(pBuf+0xD);
		}
		num = (((*pBuf)&0x1f) -1) * 26;
		for(i=1;i<=0xa;i++)
		{
			iFile.fileName[num++]=*(pBuf+i);
		}
		for(i=0xe;i<=0x19;i++)
		{
			iFile.fileName[num++]=*(pBuf+i);
		}
		for(i=0x1c;i<=0x1f;i++)
		{
			iFile.fileName[num++]=*(pBuf+i);
		}
		*itemNum = (*itemNum) +1;
		if ((*itemNum) == iFat.maxItemNum)
		{
			pFat->curCluster = iFAT32_GetNextCluster(pFat->curCluster);
			iFAT32_ReadCluster(pFat,pFat->curCluster);
			*itemNum = 0;
		}
		pBuf = buf + 32*(*itemNum);
	}
	if(iFAT32_GetChkNum(pBuf) != checkNum)
	{
		return Item_Invalid;
	}

	// Unicode×ª»¯³ÉGB2312
	iFile.fileNameLen = 0;
	for(i=0;i<fileNameLen;i=i+2)
	{
		data=iFile.fileName[i];
		data=data|(iFile.fileName[i+1]<<8);	
		if (data == 0xFFFF)
		{
			break;
		}
		else if(data< 0xA4)
		{
			iFile.fileName[iFile.fileNameLen++] = (U8)data;
		}
		else
		{
			data=UnicodeToGB2312(data);
			iFile.fileName[iFile.fileNameLen++]  =(U8)((data&0xff00)>>8);
			iFile.fileName[iFile.fileNameLen++]  =(U8)(data&0xff); 
		}
	}

	iFAT32_DisposeShortFileName(pBuf,FALSE);
	iFile.ItemCluster = pFat->curCluster;
	iFile.ItemNum     = *itemNum;
	*itemNum          = (*itemNum) +1;
	return Item_Valid;
}



/**
*******************************************************************************
* @brief      Dispose file name
* @param[out] pFat           IFAT struct pointer 
* @param[in]  buf            file item buffer
* @param[in]  itemNum        file item offset in buffer
* @param[out] itemNum        file item offset in buffer
* @retval     Item_Invalid   Invalid file item
* @retval     Item_Valid     Valid file item
* @retval     Item_END       End file item
*
* @par Description
* @details    This function is called to dispose file name. 
* @note        
*******************************************************************************
*/
char iFAT32_DisposeFileItem(P_IFAT pFat,char* buf,DWORD* itemNum)
{
	char*  pBuf;
	pBuf = buf + 32*(*itemNum);


	// µ±Ç°ÎÄ¼þÏîÊÇ·ñÒÑÉ¾³ý
	while(*pBuf == 0xE5)
	{
		*itemNum = *itemNum +1;
		return Item_Invalid;
	}


	// ÊÇ·ñÎªÓÐÐ§ÎÄ¼þÐÅÏ¢ÃèÊöÏî
	if (*pBuf == 0)
	{
		return Item_END;
	}

	// ÊÇ·ñÎª³¤ÎÄ¼þÃûÏî£¿
	if (*(pBuf+0xB) == 0x0F)
	{
		return iFAT32_DisposeLongFileName(pFat,buf,itemNum);
	}
	else
	{
		iFAT32_DisposeShortFileName(pBuf,TRUE);
		iFile.ItemCluster = pFat->curCluster;
		iFile.ItemNum     = *itemNum;
		*itemNum = (*itemNum) +1;
	}
	return Item_Valid;
}



/**
*******************************************************************************
* @brief      Spread directory for file name
* @param[out] pFat           IFAT struct pointer 
* @param[in]  fp             IFile struct pointer,include file name
* @param[out] itemNum        file item offset in buffer
* @retval     FALSE          No matching file
* @retval     TRUE           Get matching file
*
* @par Description
* @details    This function is called to dispose file name. 
* @note        
*******************************************************************************
*/
bool iFAT32_SpreadAllOverDir(P_IFAT pFat,P_IFile fp)
{
	char   rc;
	DWORD  itemNum;
	DWORD  curCluster;
	char*  buf;

	itemNum   = 0;
	iFAT32_ReadCluster(pFat,pFat->curCluster);
	buf = pFat->optBuf;
	while (1)
	{
		rc = iFAT32_DisposeFileItem(pFat,buf,&itemNum);
		if (rc == Item_END)
		{
			break;
		}

		if (itemNum == iFat.maxItemNum)
		{
			curCluster = iFAT32_GetNextCluster(pFat->curCluster);
			if (curCluster != INVALID_VALUE)
			{
				pFat->curCluster = curCluster;
				iFAT32_ReadCluster(pFat,pFat->curCluster);
				itemNum = 0;
			}
			else
			{
				break;
			}
		}

		if (rc == Item_Valid)
		{
			// ±È½ÏÊÇ·ñÊÇ²éÕÒÏî

			if(iFat32_NameCmp(iFile.fileName,fp->fileName) == TRUE)
			{
				memcpy(fp,&iFile,sizeof(IFile));
				fp->pFat = pFat;
				return TRUE;
			}
		}
	}

	//²éÕÒÊ§°Ü£¬Îª´´½¨×öÒ»Ð©×¼±¸¹¤×÷
	fp->ItemCluster = pFat->curCluster;
	fp->ItemNum     = itemNum;	
	return FALSE;
}



/**
*******************************************************************************
* @brief      Show file list in current directory
* @param[in]  pFat           IFAT struct pointer 
* @param[out] None    
* @retval     None      
*
* @par Description
* @details    This function is called to show file list in current directory. 
* @note        
*******************************************************************************
*/
void iFAT32_ShowCurDirList(P_IFAT pFat)
{
	char   rc;
	DWORD  itemNum;
	char*  buf;

	itemNum       = 0;

	iFAT32_ReadCluster(pFat,pFat->curCluster);
	buf = pFat->optBuf;
	if (pFat->curCluster != iFat.rootDirCluNum)
	{
		iFAT32_GetFatherDir(pFat,buf);
	}

	do 
	{
		rc = iFAT32_DisposeFileItem(pFat,buf,&itemNum);
		if (rc == Item_END)
		{
			break;
		}

		if (itemNum >= iFat.maxItemNum)
		{
			pFat->curCluster = iFAT32_GetNextCluster(pFat->curCluster);
			if (pFat->curCluster != INVALID_VALUE)
			{
				iFAT32_ReadCluster(pFat,pFat->curCluster);
				itemNum = 0;
			}
			else
			{
				break;
			}
		}

		if (rc == Item_Valid)
		{
			//Êä³öÎÄ¼þÐÅÏ¢
		}
	} while (1);
}



/**
*******************************************************************************
* @brief      Create short name file
* @param[in]  pFat           IFAT struct pointer 
* @param[in]  fp             IFile struct pointer
* @param[out] None    
* @retval     None      
*
* @par Description
* @details    This function is called to create short name file.
* @note        
*******************************************************************************
*/
void iFAT32_CreateShortNameFile(P_IFAT pFat, P_IFile fp)
{
	U8 item[32];
	WORD len;
	U8 i,j;

	//ÉèÖÃÎÄ¼þÃû
	for (i=0;i<11;i++)
	{
		item[i] = 0x20;
	}
	if (fp->fileAttr == FILE_DIR)
	{
		for (i=0;i<fp->fileNameLen;i++)
		{
			item[i] = iFat32_GetCapital(fp->fileName[i]);
		}
	}
	else
	{
		len = fp->fileNameLen;
		for (i=(len-1);(*(fp->fileName+i)) != '.';i--);

		for (j=0;j<i;j++)
		{
			item[j] = iFat32_GetCapital(fp->fileName[j]);
		}
		for(j=i+1;j<len;j++)
		{
			item[7+j-i] = iFat32_GetCapital(fp->fileName[j]);
		}
	}

	//ÎÄ¼þÊôÐÔ
	item[0xB] = fp->fileAttr;
	item[0xC] = 0;  //ÏµÍ³±£Áô
	item[0xD] = 0;  //´´½¨Ê±¼äµÄºÁÃëÎ»£¬±£ÁôÎª0
	//´´½¨Ê±¼ä 
	item[0xE] = (U8)(fp->fileCreatTime & 0xff);
	item[0xF] = (U8)(fp->fileCreatTime>>8);
	//´´½¨ÈÕÆÚ
	item[0x10] = (U8)(fp->fileCreatDate & 0xff);
	item[0x11] = (U8)(fp->fileCreatDate>>8);
	//×îºó·ÃÎÊÈÕÆÚ£¬´Ë´¦µÈÓÚ´´½¨ÈÕÆÚ
	item[0x12] = (U8)(fp->fileCreatDate & 0xff);
	item[0x13] = (U8)(fp->fileCreatDate>>8);
	//×î½üÐÞ¸ÄÊ±¼ä£¬´Ë´¦µÈÓÚ´´½¨Ê±¼ä
	item[0x16] = (U8)(fp->fileModifyTime & 0xff);
	item[0x17] = (U8)(fp->fileModifyTime >>8);
	//×î½üÐÞ¸ÄÈÕÆÚ£¬´Ë´¦µÈÓÚ´´½¨ÈÕÆÚ
	item[0x18] = (U8)(fp->fileModifyDate & 0xff);
	item[0x19] = (U8)(fp->fileModifyDate >>8);
	//ÎÄ¼þÄÚÈÝÊý¾ÝµÄÆðÊ¼´Ø£¬´Ë´¦Îª0
	item[0x14] = 0;
	item[0x15] = 0;
	item[0x1A] = 0;
	item[0x1B] = 0;
	//ÎÄ¼þÄÚÈÝÊý¾Ý´óÐ¡£¬´Ë´¦Îª0
	item[0x1C] = 0;
	item[0x1D] = 0;
	item[0x1E] = 0;
	item[0x1F] = 0;

	if (fp->ItemNum == iFat.maxItemNum)
	{
		fp->ItemCluster = iFTA32_GetNewCluster(fp->ItemCluster);
		fp->ItemNum     = 0;
	}
	iFAT32_ReadCluster(pFat,fp->ItemCluster);
	memcpy(pFat->optBuf+fp->ItemNum*32,item,32);
	iFAT32_WriteCluster(pFat,fp->ItemCluster);	
}



/**
*******************************************************************************
* @brief      Create long name file
* @param[in]  pFat           IFAT struct pointer 
* @param[in]  fp             IFile struct pointer
* @param[out] None    
* @retval     None      
*
* @par Description
* @details    This function is called to create long name file.
* @note        
*******************************************************************************
*/
void iFAT32_CreateLongNameFile(P_IFAT pFat, P_IFile fp)
{
	char item[32];
	U8 longNameItem[32];
	U8 fname[NAME_LENGTH];
	WORD i,j,len,offset;
	WORD data;
	U8 chknum;
	U8 itemNum;

	//´¦Àí¶ÌÎÄ¼þÃûÏî
	for (i=0;i<11;i++)
	{
		item[i] = 0x20;
	}
	if (fp->fileAttr == FILE_DIR)
	{
		for (i=0;i<6;i++)
		{
			item[i] = iFat32_GetCapital(fp->fileName[i]);
		}
		item[6] = '~';
		item[7] = '1';
	}
	else
	{
		len = fp->fileNameLen;
		for (i=(len-1);(*(fp->fileName+i)) != '.';i--);

		for (j=0;j<6;j++)
		{
			item[j] = iFat32_GetCapital(fp->fileName[j]);
		}
		item[6] = '~';
		item[7] = '1';
		for(j=i+1;j<len;j++)
		{
			item[7+j-i] = iFat32_GetCapital(fp->fileName[j]);
		}
	}

	chknum = iFAT32_GetChkNum(item);

	//ÎÄ¼þÊôÐÔ
	item[0xB] = fp->fileAttr;
	item[0xC] = 0;  //ÏµÍ³±£Áô
	item[0xD] = 0;  //´´½¨Ê±¼äµÄºÁÃëÎ»£¬±£ÁôÎª0
	//´´½¨Ê±¼ä 
	item[0xE] = (U8)(fp->fileCreatTime & 0xff);
	item[0xF] = (U8)(fp->fileCreatTime>>8);
	//´´½¨ÈÕÆÚ
	item[0x10] = (U8)(fp->fileCreatDate & 0xff);
	item[0x11] = (U8)(fp->fileCreatDate>>8);
	//×îºó·ÃÎÊÈÕÆÚ£¬´Ë´¦µÈÓÚ´´½¨ÈÕÆÚ
	item[0x12] = (U8)(fp->fileCreatDate & 0xff);
	item[0x13] = (U8)(fp->fileCreatDate>>8);
	//×î½üÐÞ¸ÄÊ±¼ä£¬´Ë´¦µÈÓÚ´´½¨Ê±¼ä
	item[0x16] = (U8)(fp->fileModifyTime & 0xff);
	item[0x17] = (U8)(fp->fileModifyTime >>8);
	//×î½üÐÞ¸ÄÈÕÆÚ£¬´Ë´¦µÈÓÚ´´½¨ÈÕÆÚ
	item[0x18] = (U8)(fp->fileModifyDate & 0xff);
	item[0x19] = (U8)(fp->fileModifyDate >>8);
	//ÎÄ¼þÄÚÈÝÊý¾ÝµÄÆðÊ¼´Ø£¬´Ë´¦Îª0
	item[0x14] = 0;
	item[0x15] = 0;
	item[0x1A] = 0;
	item[0x1B] = 0;
	//ÎÄ¼þÄÚÈÝÊý¾Ý´óÐ¡£¬´Ë´¦Îª0
	item[0x1C] = 0;
	item[0x1D] = 0;
	item[0x1E] = 0;
	item[0x1F] = 0;


	//´¦Àí³¤ÎÄ¼þÃûÏî

	// GB2312×ª»¯³ÉUnicode
	for (i=0,len=0;i<fp->fileNameLen;)
	{
		if (*(fp->fileName+i) >= 0x80)
		{
			data = *(fp->fileName+i+1) |(*(fp->fileName+i)<<8);
			data = GB2312ToUnicode(data);
			i=i+2;
		}
		else
		{
			data = *(fp->fileName+i);
			i++;
		}
		fname[len++] = (U8)(data&0xff);
		fname[len++] = (U8)(data>>8);
	}
	if ((len%26) != 0 )
	{
		fname[len++] = 0x00;
		fname[len++] = 0x00;
	}
	while ( (len%26) != 0 )
	{
		fname[len++] = 0xFF;
	}


	for (itemNum=(len/26);itemNum!=0;itemNum--)
	{
		if (fp->ItemNum == iFat.maxItemNum)
		{
			iFAT32_WriteCluster(pFat,fp->ItemCluster);
			fp->ItemCluster = iFTA32_GetNewCluster(fp->ItemCluster);
			iFAT32_ReadCluster(pFat,fp->ItemCluster);
			fp->ItemNum     = 0;
		}		

		if (itemNum == (len/26))
		{
			longNameItem[0] = itemNum|0x40;
		}
		else
		{
			longNameItem[0] = itemNum;
		}
		offset = (itemNum-1)*26;
		for (i=0;i<10;i++)
		{
			longNameItem[1+i] = fname[offset+i];
		}
		offset = offset+10;
		for (i=0;i<12;i++)
		{
			longNameItem[0xE + i] = fname[offset+i];

		}
		offset = offset+12;
		for (i=0;i<4;i++)
		{
			longNameItem[0x1C+i] = fname[offset+i];
		}
		longNameItem[0xB]  = 0x0F;
		longNameItem[0xC]  = 0;
		longNameItem[0xD]  = chknum;
		longNameItem[0x1A] = 0;
		longNameItem[0x1B] = 0;	
		memcpy(pFat->optBuf+fp->ItemNum*32,longNameItem,32);
		fp->ItemNum++;
	}

	if (fp->ItemNum == iFat.maxItemNum)
	{
		iFAT32_WriteCluster(pFat,fp->ItemCluster);
		fp->ItemCluster = iFTA32_GetNewCluster(fp->ItemCluster);
		iFAT32_ReadCluster(pFat,fp->ItemCluster);
		fp->ItemNum     = 0;
	}		
	memcpy(pFat->optBuf+fp->ItemNum*32,item,32);
	iFAT32_WriteCluster(pFat,fp->ItemCluster);
}


/**
*******************************************************************************
* @brief      Read file content
* @param[in]  fp             IFile struct pointer
* @param[in]  length         Content length
* @param[out] buf            Content buffer
* @retval     hadReadLen     Content length that had read.
*
* @par Description
* @details    This function is called to read file content.
* @note        
*******************************************************************************
*/
DWORD iFAT32_FileRead(P_IFile fp,char* buf,DWORD length)
{
	DWORD  clusterNum;
	DWORD  clusterSize;
	DWORD  readLen;
	DWORD  curLocation;
	DWORD  hadReadLen;
	char*  buffer;
	if (buf == NULL)
	{
		return 0;
	}
	if (fp->fileSize == 0)
	{
		return 0;
	}
	if ((fp->fileSize - fp->curLocation) < length)
	{
		length = fp->fileSize - fp->curLocation;
	}
	if (length == 0)
	{
		return 0;
	}
	hadReadLen = 0;


	clusterSize = iFat.secNumInCluster*iFat.secSize;
	curLocation = fp->curLocation;
	clusterNum  = fp->fileClusterNum;
	while ((curLocation/clusterSize) != 0)
	{
		clusterNum = iFAT32_GetNextCluster(clusterNum);
		curLocation = curLocation-clusterSize;
	}

	readLen = clusterSize - curLocation;
	if (length <= readLen)
	{
		readLen = length;
		length = 0;
	}
	else
	{
		length = length -readLen;
	}
	iFAT32_ReadCluster(fp->pFat,clusterNum);
	buffer = fp->pFat->optBuf;
	memcpy(buf,buffer+curLocation,readLen);
	hadReadLen = readLen;
	while (length != 0)
	{
		clusterNum = iFAT32_GetNextCluster(clusterNum);
		iFAT32_ReadCluster(fp->pFat,clusterNum);
		readLen = clusterSize;
		if (length <= readLen)
		{
			readLen = length;
			length = 0;
		}
		else
		{
			length = length -readLen;
		}
		memcpy(buf+hadReadLen,buffer,readLen);
		hadReadLen = hadReadLen+readLen;
	}
	return hadReadLen;
}



/**
*******************************************************************************
* @brief      Write file content.
* @param[in]  fp          IFile struct pointer      
* @param[in]  length      Content length
* @param[in]  buf         Content buffer
* @param[out] None
* @retval     hadReadLen  Content length that had written.
*
* @par Description
* @details    This function is called to write file content.
* @note       Only support to write content at the end of file.
*******************************************************************************
*/
DWORD iFAT32_FileWrite(P_IFile fp,char* buf,DWORD length)
{
	DWORD  clusterNum;
	DWORD  clusterSize;
	DWORD  writeLen;
	DWORD  hadWriteLen;
	DWORD  curLocation;
	char*  buffer;
	if (buf == NULL)
	{
		return 0;
	}
	if (length == 0)
	{
		return 0;
	}


	fp->hasModify = TRUE;

	buffer = fp->pFat->optBuf;

	if (fp->fileClusterNum == 0)
	{
		fp->fileClusterNum = iFTA32_GetNewCluster(0);
	}
	clusterNum = fp->fileClusterNum;
	clusterSize = iFat.secNumInCluster*iFat.secSize;

	writeLen = clusterSize - (fp->fileSize)%clusterSize;
	curLocation = fp->fileSize;
	fp->fileSize = fp->fileSize + length;
	if (length <= writeLen)
	{
		writeLen = length;
		length = 0;
	}
	else
	{
		length = length -writeLen;
	}

	if(curLocation != 0)
	{
		curLocation = curLocation-1;
		while ((curLocation/clusterSize) != 0)
		{
			clusterNum = iFAT32_GetNextCluster(clusterNum);
			curLocation = curLocation-clusterSize;
		}
		curLocation = curLocation +1;
	}


	if(curLocation == clusterSize)
	{
		clusterNum = iFTA32_GetNewCluster(clusterNum);
		curLocation = 0;
	}

	if (curLocation == 0)
	{
		iFAT32_ReadNewCluster(fp->pFat,clusterNum);
	}
	else
	{
		iFAT32_ReadCluster(fp->pFat,clusterNum);
	}

	memcpy(buffer+curLocation,buf,writeLen);
	iFAT32_WriteCluster(fp->pFat,clusterNum);
	hadWriteLen = writeLen;

	while(length != 0)
	{
		clusterNum = iFTA32_GetNewCluster(clusterNum);
		iFAT32_ReadNewCluster(fp->pFat,clusterNum);
		writeLen = clusterSize;
		if (length <= writeLen)
		{
			writeLen = length;
			length = 0;
		}
		else
		{
			length = length -writeLen;
		}
		memcpy(buffer,buf+hadWriteLen,writeLen);
		iFAT32_WriteCluster(fp->pFat,clusterNum);
		hadWriteLen = hadWriteLen + writeLen;
	}

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
P_BulkT iFAT32_BulkWriteFile(P_IFile fp,DWORD size,U8* rc)
{
	DWORD  clusterSize;
	DWORD  clusterNum;
	DWORD  curCluster;
	DWORD  addr;
	if (fp->fileSize != 0)
	{
		return NULL;
	}
	iFAT32_FlushData(fp);
	clusterSize = iFat.secNumInCluster*iFat.secSize;
	clusterNum = (size+clusterSize-1)/clusterSize;
	curCluster = iFTA32_GetNewClusters(0,clusterNum,rc);
	if (curCluster == INVALID_VALUE)
	{
		return NULL;
	}
	addr = iFAT32_GetSecFromCluster(curCluster)*iFat.secSize;
	if (iDisk_BeginBulkWrite(addr) == FALSE)
	{
		return NULL;
	}

	fp->hasModify       = TRUE;
	fp->fileSize        = size;
	fp->fileClusterNum  = curCluster;
	iBulkT.fp           = fp;
	iBulkT.buffer       = fp->pFat->optBuf;
	fp->pFat->bufferCluster = INVALID_VALUE;
	iBulkT.bufferOffset = 0;
	iBulkT.bufferSize   = ClusterSize;
	iBulkT.clusterNum   = curCluster;
	iBulkT.totalSize    = size;
	iBulkT.residueLength= size;
	return (&iBulkT);	
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
DWORD iFAT32_DoBulkWrite(P_BulkT pBulk,char* buf,DWORD size)
{
	DWORD length;
	DWORD hadWriteLen;
	if (size == 0)
	{
		return 0;
	}

	if (pBulk->residueLength <= size)
	{
		size = pBulk->residueLength;
		hadWriteLen = size;
	}

	length  = pBulk->bufferSize - pBulk->bufferOffset;
	if (length >size)
	{
		memcpy(pBulk->buffer+pBulk->bufferOffset,buf,size);
		pBulk->bufferOffset  += size;
		pBulk->residueLength -= size;
	}
	else
	{
		memcpy(pBulk->buffer+pBulk->bufferOffset,buf,length);
		pBulk->bufferOffset   = 0;
		pBulk->residueLength -= length;
		size = size - length;
		buf = buf+length;
		iDisk_DoBulkWrite(pBulk->buffer,pBulk->bufferSize,pBulk->residueLength);


		while (size >= pBulk->bufferSize)
		{
			pBulk->residueLength -= pBulk->bufferSize;
			iDisk_DoBulkWrite(buf,pBulk->bufferSize,pBulk->residueLength);
			buf = buf + pBulk->bufferSize;
			size = size - pBulk->bufferSize;
		}
		if (size != 0)
		{
			memcpy(pBulk->buffer,buf,size);
			pBulk->bufferOffset   = size;
			pBulk->residueLength -= size;
		}
	}
	return hadWriteLen;
}

/**
*******************************************************************************
* @brief      Delete file
* @param[in]  fp          IFile struct pointer     
* @param[out] None
* @retval     None  
*
* @par Description
* @details    This function is called to delete file.
* @note       
*******************************************************************************
*/
void iFAT32_DeleteFile(P_IFile fp)
{
	char* buf;

	iFAT32_ReadCluster(fp->pFat,fp->ItemCluster);
	buf = fp->pFat->optBuf;
	//ÉèÎªÉ¾³ý
	*(buf+fp->ItemNum*32) = 0xE5;
	iFAT32_WriteCluster(fp->pFat,fp->ItemCluster);
	iFAT32_RecoverFileCluster(fp->fileClusterNum);
	iFAT32_FlushData(fp);
}
