#include   <stdio.h>  
#include   <stdlib.h>  
#include   <string.h>   
#include   <sys/types.h>
#include   <unistd.h>
#include   <time.h> 
#include   "coverge.h"


Comp_Link g_ListNew[PORT_NUMBER] = { NULL };
Comp_Link g_ListOld[PORT_NUMBER] = { NULL };



/*********************************************************************************
  *Function:  Str_to_Num
  *Description：将从数据库中获取的字符串形式的IP地址，转化成32bit的无符号整数
  *Calls:  NULL
  *Called By:  Get_Insert_Joint_IP
  *Input:  从数据库中获取的字符串形式的IP地址
  *Output:  NULL
  *Return:  IP地址对应的32bit的无符号整数
  *date:  2013-02-22
**********************************************************************************/
UINT32 Str_to_Num ( char*str ) 
{
	UINT32 uiWord = 0;
	UINT32 uiBlock = 0;	
	char *ptemp = NULL;
	ptemp = (char*)str;
	
	while( *ptemp != '\0' )
	{
		uiWord = 0;
		if(*ptemp <'0'||*ptemp >'9')
		{
		/* 如果取到的不是0~9的数字，取下一个 */  
			ptemp++;
		}
		else
		{
			/* 计算IP地址，放到uiBlock中 */  
			while( *ptemp >= '0' && *ptemp <= '9' )
			{
				uiWord = uiWord* 10 + *ptemp-'0';
				ptemp++;
			}
			
			uiBlock = (uiBlock<<8) + uiWord;
		}
	}
	return uiBlock;
}



void Com_Addr(void)
{
	UINT32 uiTotal   = 0;
	UINT32 uiPort    = 0;
	UINT32 uiLoop    = 0;
	UINT32 uiNewPort = 0;
	UINT32 uiOldPort = 0;
	UINT32 uiChanged = 0;
	
	Comp_Link pNew[PORT_NUMBER] = { NULL } ;
	Comp_Link pOld[PORT_NUMBER] = { NULL } ;
	
	time_t rawtime;
	struct tm * timeinfo;
	time(&rawtime);
	
	FILE *FpLog = NULL;
	if(( FpLog = fopen("Cov.log","at+")) == NULL )
	{
		timeinfo = localtime(&rawtime);
		printf("%s can not open Cov.log!\n",asctime(timeinfo));
		exit(0);
	}
	
	FILE *FpHop = NULL;
	if((FpHop = fopen("Nexthop.cfg","r")) == NULL )
	{
		timeinfo = localtime(&rawtime);
		fprintf(FpLog,"%s      Can not open Nexthop.cfg\n",asctime(timeinfo));
		exit(0);
	}
	
	char stSwitch[15] = { 0 } ; 
	char stTelnet[15] = { 0 } ; 
	char stUser[15]   = { 0 } ; 
	char stPasswd[15] = { 0 } ; 
	
	FILE *FpSwitch = NULL; 
	if(( FpSwitch = fopen("Switch.cfg","r")) == NULL ) 
	{ 
		timeinfo = localtime(&rawtime);
		fprintf(FpLog,"%s        Can not open Switch.cfg\n",asctime(timeinfo));
		exit(0); 
	} 
	SWITCHINFO(FpSwitch,stSwitch) 
	
	char stHopPort[6][12] = {{0}};
	char stHopAddr[6][15] = {{0}};
	char stHopOut[6][12] = {{0}};
	
	char ch = 0;
	READ_PORTADDRESS(FpHop,ch,uiLoop,stHopPort,stHopAddr,stHopOut)
	
	
	char stLastHopPort[6][12] = {{0}};
	char stLastHopAddr[6][15] = {{0}};
	char stLastHopOut[6][12] = {{0}};
	FILE *Fplasthop = NULL;
    if( access("Lasthop.cfg",0) )
    {
		FpHop = fopen("Nexthop.cfg","r");
		timeinfo = localtime(&rawtime);
		fprintf(FpLog,"%s        Can not open Lasthop.cfg\n",asctime(timeinfo));
		/*copy the nexthop.cfg to lasthop.cfg*/
		Fplasthop = fopen("Lasthop.cfg","w");
		while( EOF != ( ch = fgetc( FpHop ) ) )
		{
			fprintf(Fplasthop,"%c",ch);
		}
    }
	else 
	{
		Fplasthop = fopen("Lasthop.cfg","r");
		READ_PORTADDRESS(Fplasthop,ch,uiLoop,stLastHopPort,stLastHopAddr,stLastHopOut)
	}
	
	FILE *FpConfig = NULL;
    if((FpConfig=fopen("Config.sh","w+")) == NULL )
    {
		timeinfo = localtime(&rawtime);
		fprintf(FpLog,"%s        Can not open Config.sh\n",asctime(timeinfo));
        exit(0);
    }
	uiTotal = 1;
	fprintf(FpConfig,"%s","#!/bin/sh\n");
	TELNET_HEAD
	for( uiPort = 0; uiPort < PORT_NUMBER ; uiPort++ )
	{
		uiChanged = 0;
		/*新旧数据出现某端口断掉，不存在的情况怎么办*/
		if( NULL == g_ListNew[uiPort] )
		{
			continue;
		}
		pNew[uiPort] = &(*g_ListNew[uiPort]);
		pOld[uiPort] = &(*g_ListOld[uiPort]);
		uiOldPort = Str_to_Num ( stLastHopAddr[uiPort] );
		uiNewPort = Str_to_Num ( stHopAddr[uiPort] );
		if( uiOldPort != uiNewPort )
		{
			uiChanged = 1;
		}
		else
		{
			timeinfo = localtime(&rawtime);
			fprintf(FpLog,"%s        The %c port old address  %s equal  new address %s !\n", \
					      asctime(timeinfo), uiPort+'A',stLastHopAddr[uiPort],stHopAddr[uiPort]);
		}
		while( pNew[uiPort]->next != NULL && pOld[uiPort]->next != NULL )
		{
			if(pNew[uiPort]->uiAddress == pOld[uiPort]->uiAddress)
			{
				//如果新旧相等，则判断下一跳是否相等，不等以新的为准
				if( uiChanged && ( uiNewPort != 0 ) )
				{
					//新下一跳和旧下一跳不等，以新的为准，旧的删掉，新的配上
					//删除旧的
					timeinfo = localtime(&rawtime);
					fprintf(FpLog,"%s        The %c port address has been changed from %s to %s !\n", \
					              asctime(timeinfo),uiPort+'A',stLastHopAddr[uiPort],stHopAddr[uiPort]);
					fprintf(FpConfig,"echo \"undo ip route-static ");
					WRITEADDR_MASK( FpConfig,pOld[uiPort] )
					fprintf(FpConfig," %s\"\n",stLastHopAddr[uiPort]);
					
					uiTotal++;
					
					//配上新的
					fprintf(FpConfig,"echo \"ip route-static ");
					WRITEADDR_MASK( FpConfig,pNew[uiPort] )
					fprintf(FpConfig," %s\"\n",stHopAddr[uiPort]);	
					
					uiTotal++;					
				}
				pNew[uiPort] = pNew[uiPort]->next;
				pOld[uiPort] = pOld[uiPort]->next;
			}
			else if( pNew[uiPort]->uiAddress > pOld[uiPort]->uiAddress )
			{
				//新大于旧，则证明旧的被删掉了，输出删除旧项的配置，旧项指针指向下一条
				fprintf(FpConfig,"echo \"undo ip route-static ");
				WRITEADDR_MASK(FpConfig,pOld[uiPort])
				
				fprintf(FpConfig," %s\"\n",stHopAddr[uiPort]);	
				pOld[uiPort] = pOld[uiPort]->next;
				
				uiTotal++;
				
			}
			else if( pNew[uiPort]->uiAddress < pOld[uiPort]->uiAddress )
			{
				//新小于旧，则证明老表中原来没有，输出增加新项的配置，新指针指向下一条
				fprintf(FpConfig,"echo \"ip route-static ");
				
				WRITEADDR_MASK(FpConfig,pNew[uiPort])
				fprintf(FpConfig," %s\"\n",stHopAddr[uiPort]);	
				pNew[uiPort] = pNew[uiPort]->next;
				
				uiTotal++;
				
			}
			
			if( uiTotal % 4096 == 0 )
			{
				TELNET_TAIL
				TELNET_HEAD
			}
		}
	}
	
	TELNET_TAIL
	
	fclose( FpConfig );
	FpConfig = NULL ;
	
	fclose( FpLog );
	FpLog = NULL;
	
	fclose( FpHop );
	FpHop = NULL;
	
	fclose(FpSwitch);
	FpSwitch = NULL;
	
	fclose(Fplasthop);
	Fplasthop = NULL;
	
	return ;
}


int main( ) 
{ 
	UINT32 uiAddr = 0;
	UINT32 uiPort = 0;
	Comp_Link pstList     =  NULL ;
	Comp_Link pstListOld  =  NULL ;
	Comp_Link psTmp       =  NULL ;
	Comp_Link pstDel      =  NULL ;
	Comp_Link pstDelOld   =  NULL ;
	
	time_t rawtime;
	struct tm * timeinfo;
	time(&rawtime);
	
	char ch = 0;
	char chold = 0;
	
	FILE *FpLog = NULL;
	if(( FpLog = fopen("Cov.log","at+")) == NULL )
	{
		timeinfo = localtime(&rawtime);
		printf("%s Can not open Cov.log\n",asctime(timeinfo));
		exit(0);
	}
	
	FILE *FpNew   = NULL;
    if((FpNew=fopen("new.txt","rt")) == NULL )
    {
		timeinfo = localtime(&rawtime);
		fprintf(FpLog,"%s Can not open new.txt\n",asctime(timeinfo));
        exit(0);
    }
	
	//读取新数据到内存链表中
	while( ( ( ch = fgetc( FpNew )) >= 'A') && ch != EOF ) 
	{
		fgetc( FpNew );
		pstList = ( Comp_Link ) malloc( sizeof ( struct ComP_T ) );
		pstList->uiAddress = 0 ;
		pstList->uiMask    = 0 ;
		pstList->next      = NULL ;
		uiPort = ( ch-'A' );
		g_ListNew[uiPort] = pstList;
		while( EOF != fscanf( FpNew,"%x\n",&uiAddr ) )
		{
			/**/
			if( uiAddr == 0xFFFFFFFF )
			{
				//0xFFFFFFFF标识不同端口的分界
				break;
			}
			psTmp = ( Comp_Link ) malloc( sizeof( ComP_T ) );
			psTmp->uiAddress = uiAddr;
			psTmp->next = NULL;
			fscanf( FpNew,"%d\n",&psTmp->uiMask);
			pstList->next = psTmp;
			pstList = pstList->next;
		}
	}
	//读取老数据到内存链表中
	FILE *FpOld = NULL;
    if((FpOld = fopen("Lastest.cfg","rt")) == NULL )
    {
		timeinfo = localtime(&rawtime);
		fprintf(FpLog,"%s Can not open Lastest.cfg\n",asctime(timeinfo));
        exit(0);
    }
	while( ((chold = fgetc( FpOld )) >= 'A') && chold != EOF )
	{
		fgetc( FpOld );
		pstListOld = ( Comp_Link ) malloc( sizeof ( struct ComP_T ) );
		pstListOld->uiAddress = 0 ;
		pstListOld->uiMask    = 0 ;
		pstListOld->next      = NULL ;
		uiPort = ( chold-'A' );
		g_ListOld[uiPort]   = pstListOld;
		while( EOF != fscanf( FpOld,"%x\n",&uiAddr ) )
		{
			/**/
			if(uiAddr == 0xFFFFFFFF)
			{
				//0xFFFFFFFF标识不同端口的分界
				break;
			}
			psTmp = ( Comp_Link ) malloc( sizeof( ComP_T ) );
			psTmp->uiAddress = uiAddr;
			psTmp->next = NULL;
			fscanf( FpOld,"%d\n",&psTmp->uiMask);
			pstListOld->next = psTmp;
			pstListOld = pstListOld->next;
		}
	}
	//比较并输出增量配置文件config.sh
	timeinfo = localtime(&rawtime);
	fprintf(FpLog,"%s Begin to Compare the old and new data !\n",asctime(timeinfo));
	Com_Addr();
	fprintf(FpLog,"%s New config.sh has been Created successfully!\n",asctime(timeinfo));
	for( uiPort = 0; uiPort < PORT_NUMBER ; uiPort++ )
	{
		pstList = &(*g_ListNew[uiPort]);
		
		while( pstList != NULL )
		{
			pstDel = pstList ;
			pstList = pstList->next ;
			free( pstDel );
			pstDel = NULL;
		}
		pstListOld =&(*g_ListOld[uiPort]);
		while( pstListOld != NULL )
		{
			pstDelOld = pstListOld ;
			pstListOld = pstListOld->next ;
			free( pstDelOld );
			pstDelOld = NULL;
		}
	}

	fclose( FpNew );
	FpNew = NULL;
	
	fclose( FpOld );
	FpOld = NULL;
	
	fclose( FpLog );
	FpLog = NULL;
	return 0;
  
}