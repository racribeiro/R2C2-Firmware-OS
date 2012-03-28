/**
*******************************************************************************
* @file       iDisk.c
* @version    V0.3 
* @date       2010.03.01
* @brief      interface function code of iDisk kernel.
*******************************************************************************
* @copy
*
* Author email: piaolingsky@gmail.com
* 
* COPYRIGHT 2010 Tomy.Gu 
*******************************************************************************
*/

#include "iDisk.h"

unsigned int BlockSize;

/**
*******************************************************************************
* @brief      Initialize iDisk
* @param[in]  None      
* @param[out] None     
* @retval     TRUE        Initialize successfully  
* @retval     FALSE       Initialize fail
*
* @par Description
* @details    This function is called to initialize iDisk. 
* @note        
*******************************************************************************
*/
char iDisk_Init(void)
{
	return 0;
}


/**
*******************************************************************************
* @brief      Get block size
* @param[in]  None      
* @param[out] None     
* @retval     BlockSize   block size  
*
* @par Description
* @details    This function is called to block size. 
* @note        
*******************************************************************************
*/
unsigned int  iDisk_GetBlockSize(void)
{
	return BlockSize;
}

/**
*******************************************************************************
* @brief      Read disk block
* @param[in]  addr        Disk block address      
* @param[out] buf         Block content buffer     
* @retval     TRUE        Read successfully  
* @retval     FALSE       Read fail
*
* @par Description
* @details    This function is called to read disk block. 
* @note        
*******************************************************************************
*/
char iDisk_ReadBlock(int addr,char* buf,int blockSize)
{
	return 0;
}


/**
*******************************************************************************
* @brief      Write disk block
* @param[in]  addr        Disk block address      
* @param[out] buf         Block content buffer     
* @retval     TRUE        Write successfully  
* @retval     FALSE       Write fail
*
* @par Description
* @details    This function is called to write disk block. 
* @note        
*******************************************************************************
*/
char iDisk_WriteBlock(int addr,char* buf,int blockSize)
{
	return 0;
}


/**
*******************************************************************************
* @brief      Read disk buffer
* @param[in]  addr        Disk buffer address      
* @param[out] buf         Block content buffer     
* @retval     TRUE        Read successfully  
* @retval     FALSE       Read fail
*
* @par Description
* @details    This function is called to read disk buffer. 
* @note        
*******************************************************************************
*/
char iDisk_ReadBuffer(int addr,char* buf,int bufSize)
{
	return 0;
}


/**
*******************************************************************************
* @brief      Write disk buffer
* @param[in]  addr        Disk buffer address      
* @param[out] buf         Block content buffer     
* @retval     TRUE        Write successfully  
* @retval     FALSE       Write fail
*
* @par Description
* @details    This function is called to write disk buffer. 
* @note        
*******************************************************************************
*/
char iDisk_WriteBuffer(int addr,char* buf,int bufSize)
{
	return 0;
}


/**
*******************************************************************************
* @brief      Set Write Bulk address
* @param[in]  addr        start address      
* @param[out] rc          return value
* @retval     TRUE        set successfully  
* @retval     FALSE       set fail
*
* @par Description
* @details    This function is called to start to Set Write Bulk address.
* @note       
*******************************************************************************
*/
char iDisk_BeginBulkWrite(int addr)
{
	return 0;
}


/**
*******************************************************************************
* @brief      Write Bulk content into disk.
* @param[in]  residueLength  residue content length     
* @param[in]  size        Content length
* @param[in]  buf         Content buffer
* @param[out] None
* @retval     hadWriteLen  Content length that had written.
*
* @par Description
* @details    This function is called to start toWrite Bulk content into disk.
* @note       if residueLength=0,show Bulk write end.
*******************************************************************************
*/
char iDisk_DoBulkWrite(char* buf,int bufSize,int residueLength)
{
	return 0;
}

