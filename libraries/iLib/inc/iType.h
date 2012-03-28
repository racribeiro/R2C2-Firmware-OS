/**
*******************************************************************************
* @file       iType.h
* @version    V0.3  
* @date       2010.03.01
* @brief      This file including all universal type declare of iFAT kernel.	
*******************************************************************************
* @copy
*
* Author email: piaolingsky@gmail.com
* 
* COPYRIGHT 2010 Tomy.Gu 
*******************************************************************************
*/ 

#ifndef  _ITYPE_H_
#define  _ITYPE_H_
/*---------------------------- Configure Define ------------------------------*/
#define  NAME_LENGTH         512
#define  ClusterSize         4096


#define  E_OK                0x00  //��ȷ����
#define  E_SPACE_LACK        0x01  //�������㹻�ռ�
#define  E_SPACE_DISCRETE    0x02  //�������㹻�ռ䣬���ռ䲻����
#define  E_DISK_OPT_ERR      0x03  //���̲�������
#define  E_INVALID_PARAMETER 0x04  //��������



/*---------------------------- Constant Define -------------------------------*/
#ifndef NULL
#define NULL           ((void *)0)
#endif

#ifndef FALSE
#define FALSE          (0)
#endif

#ifndef TRUE
#define TRUE           (1)
#endif

#ifndef INVALID_VALUE
#define INVALID_VALUE  (0xFFFFFFFF)
#endif

/*---------------------------- Type Define  ----------------------------------*/
typedef  unsigned char      bool;
typedef  unsigned char      BOOL;
typedef  unsigned short     WORD;
typedef  unsigned int       DWORD;

typedef  signed   char      S8;              
typedef  unsigned char      U8;			   	
typedef  short              S16;
typedef  unsigned short     U16;
typedef  int                S32;
typedef  unsigned int       U32;
typedef  long long          S64;
typedef  unsigned long long U64;
typedef  unsigned char      BIT;




/*---------------------------- Struct Define  --------------------------------*/

/**
* @struct  IFAT  iType.h	  	
* @brief   IFAT control block
* @details This struct is use to manage file system.
*/
typedef struct 
{
	char  curDir[NAME_LENGTH];
	char  optBuf[ClusterSize];   // ��д��λΪ��
	DWORD bufferCluster;
	DWORD hasModify;
	DWORD rootDirCluster;  // ��Ϊ��Ŀ¼��
	DWORD curDirCluster;   // ��Ϊ��ǰĿ¼��
	DWORD curCluster;      // ��Ϊ��ǰ�����Ĵص�ַ
	DWORD fatherDirAddr;
	DWORD offsetAddrInFAT;
	DWORD fileNum;
	DWORD dirAttr;
}IFAT,*P_IFAT;



/**
* @struct  iFILE  iType.h	  	
* @brief   iFILE control block
* @details This struct is use to manage file.
*/
typedef struct iFILE
{	
	char   fileName[NAME_LENGTH];
	P_IFAT pFat;
	bool   hasModify;
	WORD   fileNameLen;
	DWORD  fileAttr;
	DWORD  fileSize;
	DWORD  curLocation;
	WORD   fileCreatDate;
	WORD   fileCreatTime;
	WORD   fileModifyDate;
	WORD   fileModifyTime;
	DWORD  fileClusterNum;

	// �ļ�Ŀ¼���Ϊ���ļ�������Ϊ���ļ����� 
	DWORD  ItemCluster;
	WORD   ItemNum;	
}IFile,*P_IFile;


/**
* @struct  iTIME  iType.h	  	
* @brief   ITIME control block
* @details This struct is use to show time.
*/
typedef struct
{
	U16 year;
	U16 month;
	U16 date;
	U16 hour;
	U16 minute;
	U16 second;
}ITIME,*P_ITIME;


/**
* @struct  BulkT  iType.h	  	
* @brief   BulkT control block
* @details This struct is use to manage bulk write.
*/
typedef struct
{
	P_IFile fp;
	char* buffer;
	DWORD clusterNum;
	DWORD bufferSize;
	DWORD bufferOffset;
	DWORD totalSize;
	DWORD residueLength;
}BulkT,*P_BulkT;


#endif
