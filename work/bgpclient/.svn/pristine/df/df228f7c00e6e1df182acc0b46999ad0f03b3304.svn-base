#!/usr/bin/env python
#coding=utf-8
import os
import socket
import sys
import telnetlib
import time


def login_bgp(host,password,enter):  

    t = telnetlib.Telnet(host,port=2605)  
    t.read_until("Password: ")  
    t.write(password + enter) 
  
    t.write("enable"+enter)
    t.read_until("bgpd#")

    t.write("show ip bgp"+enter)

    str_all = ''
    while 1:
        temp = t.read_until("bgpd#",0.1)
        str_all = str_all + temp;
        if temp[-1] == '#':
            break;        
        t.write(enter)
    
    print all
    output = open('data', 'w')
    output.write(str_all)
    output.close()
    t.close()
    
    #os.system("sh /home/doon/filter.sh")
if __name__=='__main__':

    host = "192.168.26.169"  
    password = 'zebra'  
    enter = '\n'  
    login_bgp(host,password,enter)
