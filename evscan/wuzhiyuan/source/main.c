/* This C program that connects to MySQL Database server*/ 
/* Coverge the list and config to SWITCH or ROUTER*/  
#include   <stdio.h>  
#include   <stdlib.h>  
#include   <string.h>   
#include   <sys/types.h>
#include   <unistd.h>
#include   <mysql/mysql.h> 
#include   <sys/wait.h>
#include   <time.h> 
#include   <pthread.h>

#include   "coverge.h"

Route_link g_HeadArray[PORT_NUMBER][PORT_LIST] = {{ NULL } };
Route_link g_TailArray[PORT_NUMBER][PORT_LIST] = {{ NULL } };
Route_link g_ResHead = NULL;
Route_link g_ResTail = NULL;

Route_link_C g_ListArray[PORT_NUMBER] = {  NULL  };


int main( ) {   
	CLOCK_T start    = 0;
	CLOCK_T finish   = 0;
	UINT32 uiTotal   = 0;
	UINT32 uiMaskLen = 0;
	UINT32 uiOutPort = 0;
	UINT32 uiListNum = 0;
	UINT32 uiAddress = 0;
	UINT32 uiRet  = 0xFFFFFFFF;
	UINT32 uiLoop = 0;

	pthread_t pid[PORT_NUMBER];
	UINT32 ret[PORT_NUMBER] = { 0 };
	UINT32 arg[PORT_NUMBER]= {0,1,2,3,4};
	/* init a link table  include 5 port*/  
	Route_link_C pstLoop   =  NULL ;   // for coverge list Loop
	Route_link_C pstDel    =  NULL ;   // for coverge list memory release 
	Route_link pstRes      =  NULL ;   // for reserve addr memory release
	Route_link pstDelRes   =  NULL ;   // for reserve addr memory release
	
	time_t rawtime;
	struct tm * timeinfo;
	time(&rawtime);
	
	
	/*新增FpConfig.sh做配置*/
	FILE *FpLog = NULL;
	if(( FpLog = fopen("Cov.log","at+")) == NULL )
	{
		printf("can not open Cov.log\n");
		exit(0);
	}
	char ch = 0;
	char stHopNum[PORT_NUMBER] = {'A','B','C','D','E'};
	FILE *Fuport = NULL;
	if( ( Fuport = fopen("Uport.txt","r") ) != NULL )
	{
		uiLoop = 0;
		while( EOF != ( ch = fgetc( Fuport ) ) && (ch != '\n') )
		{
			if( ch != stHopNum[uiLoop] )
			{
				stHopNum[uiLoop] = '0';
			}
			uiLoop++;
		}
	}
	else 
	{
		fprintf(FpLog,"can not open Uport.txt\n");
		exit(0);
	}
	for( uiLoop = 0 ; uiLoop < PORT_NUMBER ; uiLoop++ )
	{
		timeinfo = localtime(&rawtime);
		fprintf(FpLog,"%s        The %dth port is %c!\n",asctime(timeinfo),uiLoop,stHopNum[uiLoop]);
	}
	FILE *FpHop = NULL;
	if((FpHop = fopen("Nexthop.cfg","r")) == NULL )
	{
		fprintf(FpLog,"can not open file\n");
		exit(0);
	}
	char stSwitch[15] = { 0 } ; 
	char stTelnet[15] = { 0 } ; 
	char stUser[15]   = { 0 } ; 
	char stPasswd[15] = { 0 } ; 
	
	FILE *FpSwitch = NULL; 
	if(( FpSwitch = fopen("Switch.cfg","r")) == NULL ) 
	{ 
		fprintf(FpLog,"can not open Switch.cfg\n"); 
		exit(0); 
	} 
	SWITCHINFO(FpSwitch,stSwitch) 
	
	
	char stHopPort[6][12] = {{0}};
	char stHopAddr[6][15] = {{0}};
	char stHopOut[6][12] = {{0}};
	
	READ_PORTADDRESS(FpHop,ch,uiLoop,stHopPort,stHopAddr,stHopOut)
	
	
	FILE *FpCoveg = NULL;
    if(( FpCoveg = fopen("ipstatic.txt","w+")) == NULL )
    {
        fprintf(FpLog,"can not open ipstatic.txt\n");
        exit(0);
    }
	
	/*直接生成new.txt*/
	FILE *FpNew = NULL;
	if(( FpNew = fopen("new.txt","w+")) == NULL )
	{
		fprintf(FpLog,"can not open new.txt\n");
		exit(0);
	}
	
	/*新增FpConfig.sh做配置*/
	FILE *FpConfig = NULL;
	if(( FpConfig = fopen("Config.sh","w+")) == NULL )
	{
		fprintf(FpLog,"can not open Config.sh\n");
		exit(0);
	}
	
	//保留地址文件初始化
	char stres[20] = {0};
	FILE * FpRes = NULL;
	if((FpRes = fopen("Reserve.cfg","r")) == NULL )
	{
		fprintf(FpLog,"can not open reserve.cfg\n");
		exit(0);
	}
	//默认路由什么都不做
	while( EOF != fscanf( FpRes,"%s",stres) )
	{
		timeinfo = localtime(&rawtime);
		fprintf(FpLog,"%s        The %s is reserved Address!\n",asctime(timeinfo),stres);
		uiAddress = Str_to_Num ( stres );
		uiRet  = Insert_addr( &g_ResHead, &g_ResTail, uiAddress);
	}
	

	
	for( uiOutPort = 0; uiOutPort < PORT_NUMBER; uiOutPort++ )
	{
		g_ListArray[uiOutPort] = NULL;
		for(uiListNum = 0; uiListNum < PORT_LIST ; uiListNum++)
		{
			g_HeadArray[uiOutPort][uiListNum] = NULL ;
			g_TailArray[uiOutPort][uiListNum] = NULL ;
		}
	}
		
	timeinfo = localtime(&rawtime);
	fprintf(FpLog,"%s        Begin to insert IP Address !\n",asctime(timeinfo));
	
	for( uiOutPort = 0;uiOutPort < PORT_NUMBER;uiOutPort++ )
	{
		/*output table name */
		timeinfo = localtime(&rawtime);
		fprintf(FpLog,"%s        Begin to query %c's data in database!\n",asctime(timeinfo),arg[uiOutPort]+'A');
		if( stHopNum[uiOutPort] == 'A'+ arg[uiOutPort])	
		{
			ret[uiOutPort] = pthread_create(&pid[uiOutPort],NULL,(void *)Get_Insert_Joint_IP,&arg[uiOutPort]);
			usleep(100000);
		}
		else
		{
			fprintf(FpLog,"%s        Sorry! The port A cannot connect!\n",asctime(timeinfo));
		}
	}
	
	/*仅成功开启的线程需要join*/
	for( uiLoop = 0; uiLoop < PORT_NUMBER; uiLoop++)
	{
		if( ( '0' != stHopNum[uiLoop] ) && ( 0 == ret[uiLoop] ) )
		{
			usleep(100000);
			pthread_join(pid[uiLoop],NULL);
		}
		
	}
	

	
	/*put the big list into the last node list*/
	timeinfo = localtime(&rawtime);
	fprintf(FpLog,"%s        Begin to Coverge the IP Address !\n",asctime(timeinfo));
	start = clock();
	for( uiOutPort = 0; uiOutPort < PORT_NUMBER ; uiOutPort++ )
	{
		if(g_ListArray[uiOutPort] == NULL )
		{
			continue ;
		}
			
		uiRet = Coverge_addr( &g_ListArray[uiOutPort],uiOutPort); 
		
	}
	finish = clock();
	
	/*将三条链表中的相同的C类地址，依据是哪条链表中的初始地址少，地址条数设为0****/
	Coverge_Mask_addr( );
	
	for( uiOutPort = 0; uiOutPort < PORT_NUMBER ; uiOutPort++ )
	{
		if( NULL == g_ListArray[uiOutPort])
		{
			continue;
		}
		pstLoop = &(*g_ListArray[uiOutPort]);
		pstLoop = pstLoop->next;
		fprintf(FpCoveg,"this is the %d list\n",uiOutPort);
		while( pstLoop != NULL )
		{	
			fprintf(FpCoveg,"IP route-static ");
			
			WRITEADDR(FpCoveg,pstLoop->addr)
			
			uiMaskLen = 0;
			while( ( pstLoop->addr << uiMaskLen ) != 0x80000000 && ( pstLoop->addr << uiMaskLen ) != 0x0 )
			{
				uiMaskLen++;
			}
		
			fprintf( FpCoveg," %2d",++uiMaskLen );
			fprintf( FpCoveg," %s\n",stHopAddr[uiOutPort] );
		
			pstLoop = pstLoop->next ;
			uiTotal++;
		}
	}
	timeinfo = localtime(&rawtime);
	fprintf(FpLog,"%s        I have use %f seconds to coverge the list!\n",asctime(timeinfo),(double)(finish - start) / CLOCK_PER_SEC);
	fprintf(FpLog,"%s        Route tables Coverge completed and Begin to Futher Coverge !\n",asctime(timeinfo));
	/*再次深度汇聚，在24为掩码基础上，将同一端口中可以深度汇聚的项合并*/
	for( uiOutPort = 0; uiOutPort < PORT_NUMBER ; uiOutPort++ )
	{
		if( g_ListArray[uiOutPort] == NULL )
		{
			continue ;
		}
			
		uiRet = Coverge_addr_Bet( &g_ListArray[uiOutPort],uiOutPort);
	}
	
	uiTotal = 1;

	//FpConfig不为空，证明目前还是初始化阶段，第一次汇聚，生成FpConfig.sh 做配置。
	fprintf(FpConfig,"%s","#!/bin/sh\n");
	TELNET_HEAD
	for( uiOutPort = 0; uiOutPort < PORT_NUMBER ; uiOutPort++ )
	{
		if( g_ListArray[uiOutPort] == NULL )
		{
			continue ;
		}
		pstLoop = &(*g_ListArray[uiOutPort]);
		
		pstDel = pstLoop ;
		pstLoop = pstLoop->next;
		free( pstDel );
		
		pstDel = pstLoop ;
		pstLoop = pstLoop->next;
		free( pstDel );
		
		fprintf(FpNew,"%c\n", uiOutPort+'A' );
		
		while( pstLoop != NULL )
		{
			if( ! ( 0 == pstLoop->cnum || 0 == ( pstLoop->addr & 0xFFFF ) ) )//16位掩码这里要屏蔽么，怎么屏蔽？
			//保留两个值，查看是否和这两个相同，相同则跳过，不同配置，并更新保留值??
			/*This address has been equal to other port because cover number is 0 or addr mask long than 0xFFFF*/
			{
				//FpConfig不为空，证明目前还是初始化阶段，第一次汇聚，生成FpConfig.sh 做配置。
				fprintf(FpConfig,"echo \"ip route-static ");
				
				WRITEADDR(FpConfig,pstLoop->addr)
				
				uiMaskLen = 0;
				
				while( ( pstLoop->addr << uiMaskLen ) != 0x80000000 && ( pstLoop->addr << uiMaskLen ) != 0x0 )
				{
					uiMaskLen++;
				}
				uiMaskLen++;
				
				/*将当前的IP地址以16进制的形式存进文本文档*/
				fprintf(FpNew,"%x\n", pstLoop->addr);
				fprintf(FpNew,"%d\n", uiMaskLen);
			
			
				fprintf(FpConfig,"%2d",uiMaskLen );
				fprintf(FpConfig," %s\"\n",stHopAddr[uiOutPort]);				
			
				uiTotal++;
				if( uiTotal%4096 == 0 )
				{
					TELNET_TAIL
					TELNET_HEAD
				}
			}
			pstDel = pstLoop ;
			pstLoop = pstLoop->next ;
			free( pstDel );
			pstDel = NULL;
		}
		fprintf(FpNew,"%x\n", 0xFFFFFFFF);
	}


	TELNET_TAIL

	
	
	pstRes = &(*g_ResHead);
	while( pstRes != NULL )
	{
		pstDelRes = pstRes ;
		pstRes = pstRes->next ;
		free( pstDelRes );
		pstDelRes = NULL ;
	}
	
	timeinfo = localtime(&rawtime);
	fprintf( FpLog,"%s        Coverge success ! ,the total table is:%d\n",asctime(timeinfo),uiTotal );
	
	fclose( FpCoveg );  FpCoveg= NULL;
	fclose( FpNew );    FpNew  = NULL;
	fclose( Fuport );   Fuport = NULL;
	fclose( FpRes );    FpRes  = NULL;
	fclose( FpConfig ); FpConfig = NULL;
	fclose( FpLog );    FpLog = NULL;
	fclose( FpSwitch);  FpSwitch  = NULL;
	return OK;
}   

