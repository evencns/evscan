#include   <stdio.h>  
#include   <stdlib.h>  
#include   <string.h> 
#include   <time.h> 
#include "coverge.h"


int main()
{
	char ch = 0;
	char stHopPort[6][12] = {{0}};
	char stHopAddr[6][15] = {{0}};
	char stHopOut[6][12] = {{0}};
	int  uiLoop = 0;
	
	time_t rawtime;
	struct tm * timeinfo;
	time(&rawtime);
	
	FILE *FpLog = NULL;
	if(( FpLog = fopen("Cov.log","a")) == NULL )
	{
		timeinfo = localtime(&rawtime);
		printf("%s can not open Cov.log!\n",asctime(timeinfo));
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
		fprintf(FpLog,"%s        can not open Switch.cfg!\n",asctime(timeinfo));
		exit(0); 
	} 
	SWITCHINFO(FpSwitch,stSwitch) 
	
	FILE *FpHop = NULL;
	if((FpHop = fopen("Nexthop.cfg","r")) == NULL )
	{
		timeinfo = localtime(&rawtime);
		fprintf(FpLog,"%s        can not open Nexthop.cfg!\n",asctime(timeinfo));
		exit(0);
	}
	FILE *FpConfig = NULL;
	if((FpConfig = fopen("probe.sh","w+")) == NULL )
	{
		timeinfo = localtime(&rawtime);
		fprintf(FpLog,"%s        can not open probe.sh!\n",asctime(timeinfo));
		exit(0);
	}
	
	READ_PORTADDRESS(FpHop,ch,uiLoop,stHopPort,stHopAddr,stHopOut)
	
	fprintf(FpConfig,"%s","#!/bin/sh\n");
	TELNET_HEAD
	for(uiLoop = 0;uiLoop<5;uiLoop++)
	{
		fprintf(FpConfig,"%s","ping -c 4 ");
		fprintf(FpConfig,"%s",stHopAddr[uiLoop]);
		fprintf(FpConfig," %c",'>');
		fprintf(FpConfig,"%d",uiLoop);
		fprintf(FpConfig,"%s",".txt\n");
		fprintf(FpConfig,"%s","sleep 2\n");
	}
	
	TELNET_TAIL
	
	fclose(FpConfig);
	FpConfig = NULL;
	
	fclose(FpHop);
	FpHop = NULL;
	
	fclose(FpLog);
	FpLog = NULL;
	
	fclose(FpSwitch);
	FpSwitch = NULL;
	return 0;
}