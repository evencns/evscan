#include   <stdio.h>  
#include   <stdlib.h>  
#include   <string.h>   
#include   <mysql/mysql.h> 
#include   <time.h> 
#include   <pthread.h>
#include   "coverge.h"

extern Route_link g_HeadArray[PORT_NUMBER][PORT_LIST];
extern Route_link g_TailArray[PORT_NUMBER][PORT_LIST];
extern Route_link g_ResHead;
extern Route_link g_ResTail;

extern Route_link_C g_ListArray[PORT_NUMBER] ;



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
	UCHAR *ptemp = NULL;
	ptemp = (UCHAR*)str;
	//printf("%s\n",ptemp);
	
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
			while(*ptemp>='0' && *ptemp<='9')
			{
				uiWord = uiWord* 10 + *ptemp-'0';
				ptemp++;
			}
			
			uiBlock = (uiBlock<<8) + uiWord;
		}
	}
	return uiBlock;
}

/*********************************************************************************
  *Function:  Skip_Check_addr
  *Description：判断该地址是否在保留名单内，如果是则跳过不参与汇聚
  *Calls:  NULL
  *Called By:  Get_Insert_Joint_IP
  *Input:  待判断IP地址
  *Output:  NULL
  *Return: 判断结果，命中返回0 ，未命中返回ERR
  *date:  2013-02-22
**********************************************************************************/
UINT32  Skip_Check_addr( UINT32 uiAddr )
{
	Route_link pstRes = NULL;
	pstRes = &(*g_ResHead);
	
	while( pstRes != NULL )
	{
		if( ( uiAddr & 0xFFFFFF00 ) == pstRes->addr )
		{
			
			return OK;
		}
		
		pstRes = pstRes->next;
	}
	return ERR;
}
/*********************************************************************************
  *Function:  Insert_addr
  *Description：将32bit的无符号整数形式的Ip地址按从小到大顺序插入链表中
  *Calls:  NULL
  *Called By:  Get_Insert_Joint_IP
  *Input:  目的链表的头结点和尾节点以及待插入的IP地址
  *Output:  NULL
  *Return: 插入结果
  *date:  2013-02-22
**********************************************************************************/
UINT32 Insert_addr( Route_link *pstHead,Route_link *psTail,UINT32 uiIpaddr)
{
	//修改算法，如果比当前节点的值大，则往后遍历找位置，比当前小的从头遍历找位置，插入后返回插入位置，为下一次插入做比较基准
	UINT32 uiLtemp 	   = 0;
	Route_link pstmp   = NULL;
	Route_link pstHsek = NULL;
	Route_link pstHtil = NULL;
	
	pstHtil = *psTail ;
	/*新申请节点 */  
	pstmp = ( Route_link ) malloc( sizeof ( struct Route_T ) );
	if( pstmp == NULL )
	{
		#ifdef DEBUG
		printf("sorry,failed to malloc mem pstmp == NULL!!\n");
		#endif
		return OK;
	}
		
		
	if( *pstHead == NULL )
	{
		/* 如果还没有节点，则新增节点，入参放到新增节点中去 */  
		pstmp->next = NULL;
		*psTail  = pstmp;
		*pstHead = pstmp;
		pstmp->addr = uiIpaddr ;
		return OK;
	}
	/* 判断是否大于末节点的值 */ 
	if( uiIpaddr == pstHtil->addr )
	{
		/* 如果入参等于末尾节点，则释放节点 */ 
		free(pstmp);
		//pstmp = NULL;
		return OK;
	}
		
	if( uiIpaddr  > pstHtil->addr )
	{
		/* 如果入参比末尾节点大，则新增节点，入参放到新增节点中去 */  
		pstmp->next = NULL;
		pstHtil->next = pstmp;
		
		*psTail = pstmp;
			
		pstmp->addr = uiIpaddr;
		return OK;
	}
	else if( uiIpaddr < pstHtil->addr )
	{
		/* 如果入参比末尾节点小，则从头遍历节点，在比入参小的第一个节点后新增节点然后交换值 */  
		for( pstHsek = *pstHead ; pstHsek->next != NULL; pstHsek = pstHsek->next )
		{
			if( pstHsek->addr == uiIpaddr )
			{
				free(pstmp);
				//pstmp = NULL;
				return OK;
			}
			if( pstHsek->addr > uiIpaddr )
			break;
		}
		
		pstmp->next = pstHsek->next;
		//如果仅有一个节点，则尾节点应该指向新增节点
		if( pstHsek->next == NULL )
		{
			*psTail = pstmp ;
		}
		pstHsek->next = pstmp ;
		uiLtemp = pstHsek->addr;
		pstHsek->addr = uiIpaddr;
		pstmp->addr = uiLtemp;	
		return OK;
	}
	return OK;
}
/*********************************************************************************
  *Function:  Cover_listJoint
  *Description：将端口对应的所有列表整合到一个长链表中（长链表插入时指针有可能指飞）
  *Calls:  NULL
  *Called By:  Get_Insert_Joint_IP
  *Input:  目的IP对应的端口号
  *Output:  整合之后的长链表
  *Return: 整合结果
  *date:  2013-02-22
**********************************************************************************/
UINT32 Cover_listJoint( UINT32 uiOutPort )
{
	UINT32 uiListNum = 0;
	UINT32 uiFlag    = 0;
	UINT32 uiLoop    = 0;
	UINT32 uiTotal   = 0;
	UINT32 uiMin     = 0xFFFFFFFF;
	Route_link_C pstemp  = NULL ;
	Route_link_C pstList = NULL ;
	Route_link psTary[PORT_LIST] = { NULL } ; 
	Route_link pstDel = NULL;
	
	#ifdef DEBUG
	printf("Cover_listJoint:The %d list member is \n",uiOutPort);
	#endif
	for( uiListNum = 0 ;uiListNum < PORT_LIST ; uiListNum++ )
	{
		psTary[uiListNum] = g_HeadArray[uiOutPort][uiListNum];
	}
	pstList = ( Route_link_C ) malloc( sizeof ( struct Route_C ) );
	pstList->addr = uiListNum ;
	pstList->cnum = 1 ;
	pstList->next = NULL;
	g_ListArray[uiOutPort] = pstList;
	uiTotal = uiListNum ;

	while(1)
	{
		for( uiListNum = 0, uiFlag = 0,uiMin = 0xFFFFFFFF ; uiListNum < uiTotal; uiListNum++ )
		{
			/*如果当前节点指向最后一个元素，检查是否所有的单链表都到了尽头*/
			if( psTary[uiListNum] == NULL )
			{
				for(uiLoop = 0; uiLoop< uiTotal; uiLoop++)
				{
					if(psTary[uiLoop] != NULL)
					{
						uiLoop = 0xFFFF;
						break;
					}
				}
				if(uiLoop == 0xFFFF)
				continue;
				else 
				break;
			}
			if( psTary[uiListNum]->addr < uiMin )
			{	
				
				uiMin = psTary[uiListNum]->addr;
				uiFlag = uiListNum;	
			}
			
		}
		if(uiTotal == 0 || uiMin == 0xFFFFFFFF )
		{
			break;
		}
		pstemp = ( Route_link_C ) malloc( sizeof ( struct Route_C ) );
		pstemp->addr  = uiMin;
		pstemp->cnum = 1;
		pstemp->next  = NULL;
		pstList->next = pstemp;
		pstList = pstList->next;
		if( psTary[uiFlag]->next == NULL )
		{
			psTary[uiFlag] = NULL;
		}
		else
		{
			psTary[uiFlag] = psTary[uiFlag]->next;
		}
	}

	for( uiListNum = 0;uiListNum < PORT_LIST ;uiListNum++ )
	{
		psTary[uiListNum] = &(*g_HeadArray[uiOutPort][uiListNum]);
		while( psTary[uiListNum] != NULL )
		{
			pstDel = psTary[uiListNum];
			psTary[uiListNum] = psTary[uiListNum]->next;
			free(pstDel);
		}
	}
	
	return OK;
}
/*********************************************************************************
  *Function:  Get_Insert_Joint_IP
  *Description：获取从本出接口出去的字符串形式的Ip地址，并且插入，整合到一条链表
  *Calls:  NULL
  *Called By:  Get_Insert_Joint_IP
  *Input: IP对应的出端口编号
  *Output:  每个端口号对应一个长链表，存放所有本出端口的地址
  *Return: 执行结果
  *date:  2013-02-22
**********************************************************************************/
void Get_Insert_Joint_IP( UINT32 *uiPort ) 
{	
	MYSQL *conn;  
	MYSQL_RES *res; 
	MYSQL_ROW  row;  	
	conn = mysql_init(NULL);
	
	UINT32 uiOutPort = 0;
	UINT32 uiTotal   = 0 ;
	UINT32 uiListNum = 0 ;
	UINT32 uiRet     = 0 ;
	UINT32 uiAddress = 0 ;
	CLOCK_T start    = 0 ;
	CLOCK_T finish   = 0 ;

	char select[60] = {0};
	const char  *seif = "select ip from ";
	char *mytable = NULL;
	const char *whpc =" where port_channel='";
	const char *PortA = "A';";
	const char *PortB = "B';";
	const char *PortC = "C';";
	const char *PortD = "D';";
	const char *PortE = "E';";
	
	char *IpAddr = NULL;
	char Server[15] = {0};
	char User[15] = {0};
	char Password[15] = { 0 };
	char Database[20] = { 0 };
	char Table[20] = { 0 };
	char Item[15] = {0};
	
	FILE *FpMysql = NULL;
	if(( FpMysql = fopen("Mysql.cfg","r")) == NULL )
	{
		#ifdef  DEBUG
		printf("Get_Insert_Joint_IP:can not open mysql.cfg\n");
		#endif 
		exit(0);
	}

	
	fscanf( FpMysql,"%s",Item);
	fscanf( FpMysql,"%s",Server);
	
	#ifdef  DEBUG
	printf("ucServer is %s \n",Server);
	#endif 
	
	fscanf( FpMysql,"%s",Item);
	fscanf( FpMysql,"%s",User);
	#ifdef  DEBUG
	printf("ucUser is %s \n",User);
	#endif 
	
	fscanf( FpMysql,"%s",Item);
	fscanf( FpMysql,"%s",Password);
	#ifdef  DEBUG
	printf("ucPassword is %s \n",Password);
	#endif 
	
	
	fscanf( FpMysql,"%s",Item);
	fscanf( FpMysql,"%s",Database);
	#ifdef  DEBUG
	printf("ucDatabase is %s \n",Database);
	#endif 
	
	fscanf( FpMysql,"%s",Item);
	fscanf( FpMysql,"%s",Table);
	#ifdef  DEBUG
	printf("ucTable is %s \n",Table);
	#endif 
	
	
	
        
	/* Connect to database */   
	if (!mysql_real_connect(conn, Server,  User, Password, Database, 0, NULL, 0))
	{  
		fprintf(stderr, "%s\n", mysql_error(conn));   
		exit(1);   
	} 
	
	strcat(select,seif);
	mytable = Table;
	strcat(select,mytable);
	strcat(select,whpc);
	
	uiOutPort = *uiPort;
	switch(uiOutPort) 
	{
		case 0:	
			strcat(select,PortA);
			break;
		case 1:
			strcat(select,PortB);
			break;
		case 2:
			strcat(select,PortC);
			break;
		case 3:
			strcat(select,PortD);
			break;
		case 4:
			strcat(select,PortE);
			break;
		default:
			#ifdef  DEBUG
			printf("Sorry! I cannot find the port!\n");
			#endif 
			return;
	}
	
	#ifdef  DEBUG
	printf("%s\n",select);
	#endif 
	
	if ( mysql_query(conn, select) ) 
	{  
		fprintf(stderr, "%s\n", mysql_error(conn));   
		exit(1);   
	}  
	res = mysql_use_result(conn); 
	while ( ( row = mysql_fetch_row( res ) ) != NULL )   
	{	
		/* get the ip address and sort to diff tables */  
		IpAddr = row[0];
		uiAddress = Str_to_Num( IpAddr ); 
		//如果需要排除某些指定的地址，此处判断地址是否在给定的列表中，如果在，则不插入链表不参与汇聚，
		//之后直接将给定地址输出到配置文件中
		uiRet = Skip_Check_addr( uiAddress );
		if( 0 == uiRet )
		{
			continue;
		}
		else
		{
			uiRet = Insert_addr( &g_HeadArray[uiOutPort][uiListNum], &g_TailArray[uiOutPort][uiListNum],uiAddress );
		}
		uiTotal++;
		if( (uiTotal<<16) == 0 )
		{
			uiListNum++;
			#ifdef  DEBUG
			printf("I have insert   %d   tables\n",uiTotal);
			#endif 
			
		}
	}
	
	if( uiAddress == 0 )
	{
		#ifdef  DEBUG
		printf("Sorry! This database have no this port's data!!\n");
		#endif 
	}
	else
	{
		start = clock();
		uiRet = Cover_listJoint(uiOutPort);
		finish = clock();
		#ifdef  DEBUG
		printf("I have use %f seconds to Cover_listJoint %d !\n",(double)(finish - start) / CLOCK_PER_SEC ,uiOutPort);
		#endif 
	}
	
	
	mysql_free_result(res); 
	mysql_close(conn); 
	conn = NULL;
	//fclose( FpMysql );
	//FpMysql = NULL;
	//mysql_library_end();	
	//caution: this is not a bug , if this open ,there will be unexcept things happen
	return;
}

/*********************************************************************************
  *Function:  Coverge_addr
  *Description：按掩码24位汇聚并记录本条C类地址是有多少条源地址组成，放在cnum字段标示。
  *Calls:  NULL
  *Called By:  MAIN
  *Input:   待汇聚地址集合所在链表的首地址，及这条链表对应的出端口编号
  *Output:  汇聚之后的链表
  *Return: 执行结果
  *date:  2013-02-22
**********************************************************************************/
UINT32 Coverge_addr( Route_link_C *pstHeadCov,UINT32 uiPort )
{
	UINT32 uiCovered  = 1;
	Route_link_C pstLoop        =  NULL ;
	Route_link_C pstDel     =  NULL ;
	
	#if 0
	pstLoop = *pstHeadCov;//逐个出接口汇聚
	pstLoop=pstLoop->next;
	printf("This is %d list init\n",uiPort);
	while( pstLoop!= NULL) 
	{
		printf("%3d.",(pstLoop->addr&0xFF000000)>>24);
		printf("%3d.",(pstLoop->addr&0xFF0000)>>16);
		printf("%3d.",(pstLoop->addr&0xFF00)>>8);
		printf("%3d\n",(pstLoop->addr&0xFF));
		pstLoop = pstLoop->next;
	}
	#endif
	pstLoop = *pstHeadCov;//逐个出接口汇聚
	pstLoop = pstLoop->next;
	while( pstLoop != NULL) 
	{
		/*判断当前节点和下一个节点是否能汇聚*/
		uiCovered = 1;
		while( pstLoop->next!= NULL &&( ~( pstLoop->addr^pstLoop->next->addr) ) >= 0xFFFFFF00 )
		{
			pstDel = pstLoop->next;
			pstLoop->next  = pstLoop->next->next;	
			free(pstDel);
			uiCovered++;
		}
		pstLoop->addr &= MASK(8);
		pstLoop->cnum = uiCovered;
	
		pstLoop = pstLoop->next;
	}//完成一条链表的汇聚
	
	#if 0
	pstLoop = *pstHeadCov;//逐个出接口汇聚
	pstLoop = pstLoop->next;
	printf("This is %d list before mask\n",uiPort);
	while( pstLoop != NULL) 
	{
		printf("%3d.",(pstLoop->addr&0xFF000000)>>24);
		printf("%3d.",(pstLoop->addr&0xFF0000)>>16);
		printf("%3d.",(pstLoop->addr&0xFF00)>>8);
		printf("%3d\n",(pstLoop->addr&0xFF));
		pstLoop = pstLoop->next;
	}
	#endif
	return OK;
	
}

/**********************************************************************************
  *Function:  Coverge_Mask_addr
  *Description：找到各长链表中相同的C类地址，初始地址少的，地址条数设为0，不会输出到
				最终的配置文件中这里是一种模糊处理，即24位掩码地址相同时，取源地址条
				目最多的做出端口
  *Calls:  NULL
  *Called By:  main
  *Input:  NULL
  *Output:  NULL
  *Return: 执行结果
  *date:  2013-02-22
**********************************************************************************/
UINT32 Coverge_Mask_addr()
{
	UINT32 uiMin         = 0;
	UINT32 uiMinPort     = 0;
	UINT32 uiMax         = 0;
	UINT32 uiPort        = 0;
	UINT32 uiRet         = 0;
	UINT32 uiNullList    = 0;
	UINT32 uiAry[PORT_NUMBER]  = { 0 };
	Route_link_C pOut[ PORT_NUMBER ]   = { NULL };

	struct Equal stEqual = {0,0};
	//此处是否应该也加continue形式，不能保证空是连续的
	for( uiPort = 0;uiPort < PORT_NUMBER; uiPort++ )
	{
		if( NULL != g_ListArray[ uiPort ] )
		{
			pOut[uiPort] = &(*g_ListArray[uiPort]);
			pOut[uiPort] = pOut[uiPort]->next;
		}
	}
	
	
	while(1)
	{
		/*判断当前链表是否已经全部指向最后的NULL*/
		uiNullList = 0;
		for( uiPort = 0;uiPort < PORT_NUMBER; uiPort++ )
		{
			if( NULL == pOut[uiPort] )
			{
				uiNullList++;
			}
		}
		if( uiNullList == PORT_NUMBER )
		{
			break;
		}
		
		uiMin = 0xFFFFFFFF;
		uiMax = 0;
	
		for( uiPort = 0; uiPort < PORT_NUMBER ; uiPort++ )
		{
			if( pOut[uiPort] == NULL )
			{
				uiAry[uiPort] = 0xFFFF;
				continue;
			}
			
			uiAry[uiPort] = pOut[uiPort]->addr;
			
			if( uiAry[uiPort] < uiMin)
			{
				uiMin = uiAry[uiPort];
				uiMinPort = uiPort ;
			}
		}
		
		/*find out the min*/
		uiRet = ISame( uiAry,&stEqual);
		if( OK == uiRet )
		{
			if(pOut[stEqual.uiHusband]->cnum >= pOut[stEqual.uiWife]->cnum)
			{
				pOut[stEqual.uiHusband]->cnum += pOut[stEqual.uiWife]->cnum;
				pOut[stEqual.uiWife]->cnum = 0;
				pOut[stEqual.uiWife] = pOut[stEqual.uiWife]->next;
			}
			else if(pOut[stEqual.uiHusband]->cnum <= pOut[stEqual.uiWife]->cnum)
			{
				pOut[stEqual.uiWife]->cnum += pOut[stEqual.uiHusband]->cnum;
				pOut[stEqual.uiHusband]->cnum = 0;
				pOut[stEqual.uiHusband] = pOut[stEqual.uiHusband]->next;
			}
		}
		else 
		{
			pOut[uiMinPort]= pOut[uiMinPort]->next;
		}
	}
	
	return OK;
	
}


/*********************************************************************************
  *Function:  ISame
  *Description：判断三个入参中有无相等值，通过返回值标示那两个相等
  *Calls:  NULL
  *Called By:  main
  *Input:  三个IP地址
  *Output:  NULL
  *Return: 0-1和2为0；1-1和2相等；2-1和3相等；3-2和3相等；0xFFFF -没有找到相等项
  *date:  2013-02-22
**********************************************************************************/	
UINT32 ISame( UINT32 *puiArray , Equal *stEqual)
{
	UINT32 uiCurrent = 0;
	UINT32 uiCompare = 0;
	
	for( uiCurrent = 0; uiCurrent < PORT_NUMBER ; uiCurrent++ )
	{
		if(*( puiArray + uiCurrent ) == 0xFFFF)
		{
			continue;
		}
		for( uiCompare = uiCurrent+1; uiCompare < PORT_NUMBER ; uiCompare++ )
		{
			if(*( puiArray + uiCompare ) == 0xFFFF)
			{
				continue;
			}
			if( *( puiArray + uiCurrent ) == *( puiArray + uiCompare ) )
			{
				stEqual->uiHusband = uiCurrent;
				stEqual->uiWife    = uiCompare;
				return OK;
			}
		} 
	}
	
	return ERR;
}


/*********************************************************************************
  *Function:  Coverge_addr_Bet
  *Description：判断本条链表是否能进一步汇聚，
				依据是别的链表中是否有处于当前相邻两节点中间的值
  *Calls:  NULL
  *Called By:  main
  *Input:  链表首地址，出端口编号
  *Output: 汇聚之后的链表
  *Return: 执行结果
  *date:  2013-02-22
**********************************************************************************/		
UINT32 Coverge_addr_Bet( Route_link_C *pstHeadCov,UINT32 uiPort )
{
	UINT32 uiMask     = 0;
	UINT32 uiMaskLen  = 0;
	UINT32 uiCheck    = 0;
	UINT32 uiFind     = 0;
	Route_link_C pstChk      =  NULL ;//just for loop
	Route_link_C pstLoop     =  NULL ;
	Route_link_C pstDel      =  NULL ;
	Route_link_C pstChkSeek[ PORT_NUMBER ]   = { NULL };
	
	pstLoop = *pstHeadCov;//逐个出接口汇聚
	pstLoop=pstLoop->next;
		
	while( pstLoop->next != NULL ) 
	{
		uiFind  = 0;
		/*判断当前节点和下一个节点是否能汇聚*/
		for( uiCheck =0; uiCheck < PORT_NUMBER; uiCheck++ )
		{	
			if( g_ListArray[uiCheck] == NULL || uiCheck == uiPort )
			{
				continue;
			}
			else
			{
				pstChk = &(*g_ListArray[uiCheck]);
			}
			
			pstChk = pstChk->next;
			if( pstChkSeek[uiCheck] != NULL )
			{
				//如果下一个元素的循环又到了这个列表，则直接和上次的比他大的值的位置开始比较。
				pstChk = pstChkSeek[uiCheck];
			}
			while( pstChk->next != NULL )
			{
			
				if( pstChk->addr > pstLoop->addr )   
				{
					//被比较链表中有比当前准备汇聚的前一个值大的值，则判断是否小于下一个
					pstChkSeek[uiCheck] = pstChk ;//记录上次位置
					if( pstChk->addr <= pstLoop->next->addr )
					{
						//小于等于下一个则说明pstChk->addr处于中间，这两条不能汇  ===  shandiao 
						uiFind = 1;	
						//printf("sorry,this 2 node : %x _ %x cannot coverge!because list %d have 0x%x between them!\n",pstLoop->addr,pstLoop->next->addr,uiCheck,pstChk->addr);
					}
					
					break;
				}
				else
				{
					pstChk = pstChk->next;
				}
			}//此处循环结束，说明在被比较的链表中没找到处于准备汇聚的两个地址中间的地址
			
			if(	uiFind == 1 )
			break;

		}//此处循环结束uiFind == 0，说明在所有被比较的链表中没找到处于准备汇聚的两个地址中间的地址

		if(	uiFind == 0 && pstLoop->next != NULL) 
		{
			/*coverge */
			uiMask = ~( pstLoop->addr^pstLoop->next->addr);
			for( uiMaskLen = 0; uiMaskLen < 32; uiMaskLen++ )
			{
				if( (uiMask << uiMaskLen ) < 0x80000000 )
				{
					break;
				}
			}
			if( uiMaskLen == 0 )
			{
				//printf("sorry,this 2 node : %x _ %x cannot coverge!\n",pstLoop->addr,pstLoop->next->addr);
				pstLoop = pstLoop->next;
				continue;
			}
			//printf("the first node is %x ,the second node is %x ,they have %d same bit \n",pstLoop->addr,pstLoop->next->addr,uiMaskLen);
			pstLoop->addr &= MASK( 32-uiMaskLen ) ;
			//printf("after coverge,the new node is %x\n",pstLoop->addr);
			pstDel = pstLoop->next;
			//printf("the second node %x  has been discard!\n",pstLoop->next->addr);
			pstLoop->next  = pstLoop->next->next;	
			free(pstDel);
			pstDel = NULL;
			/*coverge */
		}
		else
		{
			pstLoop = pstLoop->next;
		}
	}//完成一条链表的汇聚
	return OK;
	
}