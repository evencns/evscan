#!/usr/bin/env python
#coding=utf-8
import os
import socket,select
import sys
import telnetlib
import time
import struct
import binascii

from netaddr import *
from evscan import *
from utils import *

BUFSIZE=120000
CLIENT_TO_SERVER_IP_LIST_REQ_MSG=1
CLIENT_TO_SERVER_IP_RESULT_RES_MSG=5
#Export BGP Server ipaddr..

def login_bgp(bgp_host,bgp_pwd,enter):
    #import pdb; pdb.set_trace()
    t = telnetlib.Telnet(bgp_host,port=2605)
    t.read_until("Password: ")
    t.write(bgp_pwd + enter)

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

    output = open('data', 'w')
    output.write(str_all)
    output.close()
    t.close()
    os.system("sh /home/doon/filter.sh")

#Ip subnetting

def part_subnet():
    #import pdb; pdb.set_trace()
    f=open("subnet/ip.txt",'r')
    ipaddr_list = []
    while True:
        try:
            ip = f.readline()
            if not ip:
                break
            ipnetwork = IPNetwork(ip)
            for line in ipnetwork:
                str_line=str(line)
                ipaddr_list.append(str_line)

        except:
            pass 
    print "ipddr_list is %s"% len(ipaddr_list)
    return ipaddr_list

#Send Ipaddr to server
def sendaddr(s,r,eth1,remote_ip,remote_port):
    #import pdb; pdb.set_trace()
    #eth1 = get_config("env","eth1")
    try:
        ip_subnet = part_subnet()     
        hton_type = socket.htonl(CLIENT_TO_SERVER_IP_LIST_REQ_MSG)
        hton_bid = socket.htonl(1)
        text = ""
        counter = 0
        #import pdb; pdb.set_trace()
        for index in xrange(len(ip_subnet)):
            line = ip_subnet[index]
            hton_ip = struct.unpack('I',socket.inet_aton(line))[0]
            sendmsg=struct.pack('III',hton_type,hton_bid,hton_ip);
            text += sendmsg
            counter +=1
            if counter == MAX_THREAD_NUM :
                bytes = s.sendall(text)
                counter = 0
                text = ""
                #continue 
        #import pdb; pdb.set_trace()         
        loginfo = "Ip send Successfully"
        #print "%s"%loginfo

    except socket.error, msg:
        loginfo = "ERROR in sendaddr To server. Error message=%s "%(str(msg))
        print loginfo

    #s.close()
    return 0


#接收Server端返回的结果，包括IP和PORT
def receive(s,eth1):
    #import pdb; pdb.set_trace()
    recv_list = []
    while True:
        infds,outfds,errfds = select.select([s,],[],[],5)
        if len(infds) == 0:
            break
            #clientsock,clientaddr = s.accept()
        reply = s.recv(14)
        if len(reply) != 0:
            #print reply         
            res = struct.Struct('IIIH')
            (a,b,c,d) = res.unpack(reply[0:14])
            ntoh_type = socket.ntohl(a)
            ntoh_bid = socket.ntohl(b)
            ntoh_ip = socket.inet_ntoa(reply[8:12])
            ntoh_port = socket.ntohs(d)
            recv_list.append([ntoh_ip,ntoh_port])
            print 'Received %s,%s,%s,%s'%(repr(ntoh_type),repr(ntoh_bid),repr(ntoh_ip),repr(ntoh_port))
 
    s.close()
    print "receive Server data finish.."
    return recv_list
    
    #IP Evscan Result
    #evscan = new_evprobe_all(eth1,ntoh_ip,ntoh_port)
    #if len(evscan) == 0:
    #    continue
def main():   
    # Get configs.
    #import pdb; pdb.set_trace()
    remote_ip = get_config("remote_server", "s_ip")
    remote_port = get_config("remote_server", "s_port")
    eth1 = get_config("env","eth1")

    #create socket  TCP Protocol
    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    r = s.connect((remote_ip,int(remote_port)))
    #s.settimeout(10)
    #create socket UDP protocol 
    su = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    rs = (remote_ip,int(remote_port))

    start_time = time.time()
    bgp_host = get_config("bgp_server","bgp_host")
    bgp_pwd = get_config("bgp_server","bgp_pwd")
    enter = '\n'  
    #login_bgp(bgp_host,bgp_pwd,enter)
    send_end = sendaddr(s,r,eth1,remote_ip,remote_port)
    recv_res = receive(s,eth1)
    evscan_main(s,r,remote_ip,remote_port,eth1,recv_res)

    end_time = time.time()
    spend_time = end_time - start_time
    loginfo ="spend_time: (%d)" % (spend_time)
    print loginfo

if __name__=='__main__':

    main()
    
