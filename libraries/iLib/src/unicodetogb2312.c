/* Includes ------------------------------------------------------------------*/
#include "UnicodeToGB2312.h"

/* Function define -----------------------------------------------------------*/

/*******************************************************************************
* Function Name  : UnicodeToGB2312
* Description    : This function convert unicode to gb2312 code
* Input          : unicode--the unicode value
* Output         : None
* Return         : gb2312 code value
*******************************************************************************/
INT16U UnicodeToGB2312(INT16U unicode)//用二分查找算法
{
   	INT32U mid, low, high, len;
	len = sizeof(UnicodeToGB2312_Tab)/sizeof(UnicodeToGB2312_Tab[0]);
	low = 0;
	high = len - 1;
	if( unicode < 0xA4)
	   return unicode;
	while(low <= high)
	{
	    mid = (low + high) / 2;
		if(UnicodeToGB2312_Tab[mid][0] > unicode)
		    high = mid - 1;
		else if(UnicodeToGB2312_Tab[mid][0] < unicode)
		    low =  mid + 1;
		else
		    return 	UnicodeToGB2312_Tab[mid][1];
	}
	return 0 ; //没找到
}

INT16U GB2312ToUnicode(INT16U data)
{
   	INT32U low, high, len;
	len = sizeof(UnicodeToGB2312_Tab)/sizeof(UnicodeToGB2312_Tab[0]);
	low = 0;
	high = len - 1;

	while(low <= high)
	{
		if(UnicodeToGB2312_Tab[low][1] == data)
		{
			return 	UnicodeToGB2312_Tab[low][0];
		}
		low++;
	}
	return 0 ; //没找到
}

