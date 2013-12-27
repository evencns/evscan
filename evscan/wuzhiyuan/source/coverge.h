/*the define of route_coverge */

#define MASK(m)  (0xFFFFFFFF<<(m))
#define CLOCK_PER_SEC ((CLOCK_T)1000) 
#define ERR 0x1 
#define OK  0x0 
#define EVERY_LISTNUM  65536   // every list head have how many IP address
#define PORT_LIST     30       // every port has how many list head 
#define PORT_NUMBER   5
#define DEBUG  1

#define SWITCHINFO(FpSwitch,stSwitch)    		fscanf( FpSwitch,"%s",stSwitch); \
												fscanf( FpSwitch,"%s",stTelnet); \
												fprintf(FpLog,"Telnet Switch Address is  %s\n",stTelnet); \
												fscanf( FpSwitch,"%s",stSwitch); \
												fscanf( FpSwitch,"%s",stUser); \
												fprintf(FpLog,"Telnet Switch user name is %s\n",stUser); \
												fscanf( FpSwitch,"%s",stSwitch); \
												fscanf( FpSwitch,"%s",stPasswd); \
												fprintf(FpLog,"Telnet Switch user Password is %s\n",stPasswd); 
									
							        
							
#define WRITEADDR(FP,uiAddress)	    fprintf(FP,"%d.",(uiAddress&0xFF000000)>>24); \
									fprintf(FP,"%d.",(uiAddress&0xFF0000)>>16); \
									fprintf(FP,"%d.",(uiAddress&0xFF00)>>8); \
									fprintf(FP,"%d ",uiAddress&0xFF); 
								
								
#define TELNET_HEAD	            fprintf(FpConfig,"%s","{\n"); \
								fprintf(FpConfig,"%s","sleep 1\n");  \
								fprintf(FpConfig,"%s","echo \""); \
								fprintf(FpConfig,"%s",stUser); \
								fprintf(FpConfig,"%s","\"\n"); \
								fprintf(FpConfig,"%s","sleep 1\n"); \
								fprintf(FpConfig,"%s","echo \""); \
								fprintf(FpConfig,"%s",stPasswd); \
								fprintf(FpConfig,"%s","\"\n"); \
								fprintf(FpConfig,"%s","sleep 1\n"); \
								fprintf(FpConfig,"%s","echo \"sys\"\n"); \
								fprintf(FpConfig,"%s","sleep 5\n");  
								
#define TELNET_TAIL				fprintf(FpConfig,"%s"," }| telnet "); \
								fprintf(FpConfig,"%s\n",stTelnet); 
								
								
#define WRITEADDR_MASK(FP,pst)		fprintf(FP,"%d.",(pst->uiAddress&0xFF000000)>>24); \
									fprintf(FP,"%d.",(pst->uiAddress&0xFF0000)>>16); \
									fprintf(FP,"%d.",(pst->uiAddress&0xFF00)>>8); \
									fprintf(FP,"%d ",pst->uiAddress&0xFF); \
									fprintf(FP,"%2d",pst->uiMask ); 
								
							
#define READ_PORTADDRESS(FpHop,ch,uiLoop,stHopPort,stHopAddr,stHopOut)		uiLoop = 0; \
																			while( EOF != ( ch = fgetc( FpHop ) ) ) \
																			{ \
																				ch = fgetc( FpHop ); \
																				fscanf( FpHop,"%s",stHopPort[uiLoop]); \
																				ch = fgetc( FpHop ); \
																				fscanf( FpHop,"%s", stHopAddr[uiLoop] ); \
																				ch = fgetc( FpHop ); \
																				fscanf( FpHop,"%s",stHopOut[uiLoop]); \
																				ch = fgetc( FpHop ); \
																				uiLoop++; \
																			}
typedef  unsigned int  UINT32; 
typedef  unsigned char UCHAR; 
typedef  unsigned long CLOCK_T; 

							
typedef  struct ComP_T{
	UINT32 uiAddress;
	UINT32 uiMask;
	struct ComP_T  *next;
}ComP_T,*Comp_Link;

typedef  struct Route_T{
	UINT32 addr;
	struct Route_T  *next;
}Route_T,*Route_link;

typedef  struct Route_C{
	UINT32 addr;
	UINT32 cnum; //how many ip cover to this C address
	struct Route_C  *next;
}Route_C,*Route_link_C;

typedef struct Equal{
		UINT32 uiHusband;
		UINT32 uiWife;
}Equal;
	
UINT32 Str_to_Num ( char *str ) ;
UINT32 Insert_addr( Route_link *psthead,Route_link *pstail,UINT32 uiIpaddr);
UINT32 Coverge_addr( Route_link_C *pstHeadCov,UINT32 uiOutPort); 
UINT32 Cover_listJoint( UINT32 uiOutPort );

UINT32 Coverge_Mask_addr(void);
UINT32 Coverge_addr_Bet( Route_link_C *pstHeadCov,UINT32 uiPort );
UINT32 ISame( UINT32 *pAry, Equal *pstEqual);

void Get_Insert_Joint_IP( UINT32 *puiOutPort);


