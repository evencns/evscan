#include   <stdio.h>  
#include   <stdlib.h>  
#include   <string.h>  

int main()
{
	char stSwitch[15] = { 0 } ;
	char stTelnet[15] = { 0 } ;
	char stUser[15]   = { 0 } ;
	char stPasswd[15] = { 0 } ; 
	FILE *FpSwitch = NULL;
	if(( FpSwitch = fopen("Switch.cfg","r")) == NULL )
	{
		#ifdef DEBUG
		printf("can not open Switch.cfg\n");
		#endif
		exit(0);
	}
	fscanf( FpSwitch,"%s",stSwitch);
	fscanf( FpSwitch,"%s",stTelnet);
	printf(" stTelnet %s \n",stTelnet);
	
	fscanf( FpSwitch,"%s",stSwitch);
	fscanf( FpSwitch,"%s",stUser);
	printf("stUser %s \n",stUser);
	
	fscanf( FpSwitch,"%s",stSwitch);
	fscanf( FpSwitch,"%s",stPasswd);
	printf(" stPasswd %s \n",stPasswd);
	
	return 0;
}