
#define PORT_NUMBER   5   //  port number 

typedef  unsigned int  UINT32; 

typedef  struct ComP_T{
	UINT32 uiAddress;
	UINT32 uiMask;
	struct ComP_T  *next;
}ComP_T,*Comp_Link;

