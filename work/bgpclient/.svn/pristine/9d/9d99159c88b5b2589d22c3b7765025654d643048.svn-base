#!/usr/bin/env python
# -*- coding: utf-8 -*-

# This project use gevent to scan all objects. 

import threading
import getopt
import time,sys
#import nmap
import syslog
import copy
import socket
from utils import *

import gevent
from gevent import Timeout
from gevent.queue import Queue,Empty

import subprocess

import gevent.monkey
gevent.monkey.patch_socket()

from ping import *
from clientbgp import *
import gevent.monkey
gevent.monkey.patch_socket()

task_q = Queue(maxsize=100000)
MAX_THREAD_NUM = int(get_config("env","MAX_THREAD_NUM"))
g_begin = 1
counter = 0
g_flag_finish = False
g_list_result=[]
send_result_all=[]
g_db_lock=threading.Lock()

hton_type = CLIENT_TO_SERVER_IP_RESULT_RES_MSG
hton_bid = 1

version_info="evscan 2.0 build 01.20130301"
# ------------------------------------------------------------
#define task queue.
class MyTask(object):
    def __init__(self, ip,port):
        self.ip = ip
        self.port = str(port)

#define data type.
class ReportInfo_T:
    def __init__(self):
        self.bid = 1
        self.ip = ""
        self.port = 0
        self.delay = 0.0
    def __del__(self):
        self.bid = 1
        self.ip = ""
        self.port = 0
        self.delay = 0.0

def evping (ip):
    try:
    #import pdb; pdb.set_trace()
        delay = verbose_ping(ip)
        return delay
    except Exception,e:
        loginfo = "ERROR evping. error=%s"%(e)
        syslog.syslog(loginfo)
   

def evping_connect (eth1,ip,port):
    #import pdb; pdb.set_trace()
    list_elapsed = []
    eth1 = get_config("env","eth1")
    try:
        #Eth1 Ping Result
        p_delay = evping(ip)

        if p_delay is None:
            #list_elapsed[:] = []
            list_elapsed = [ip,port,-1]
        else:
            list_elapsed = [ip,port,p_delay]

    except Exception,e:

        loginfo = "ERROR evping_connect. error=%s"%(e)
        syslog.syslog(loginfo)

    return list_elapsed

# Scan one interface only. TCP Test
def scan_connect_one(s,ip,port):
    spend_time = 999.99
    try:
        #import pdb; pdb.set_trace()
        start_time = time.time()
        with gevent.timeout.Timeout(2):
            r = s.connect((ip,int(port)))
            bytes = s.send("Hello")
            if bytes==5 :
                end_time = time.time()
                spend_time = (end_time - start_time)*1000
    except socket.error, msg:
        loginfo = "ERROR in scan_connect_one. Error message=%s "%(str(msg))
        syslog.syslog(loginfo)
        #loginfo ="ip=%s port=%s socketid=%d"%(ip,port,id(s))
        #syslog.syslog(loginfo)

    return spend_time

def scan_all(eth1,ip,port):
    #import pdb; pdb.set_trace()
    # Create socket for all interface.
    #eth1
    s1 = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    s1.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
    s1.settimeout(2)
    s1.bind((eth1, 0))

    list_elapsed = []
    try:
        #import pdb; pdb.set_trace()
        delay1 = scan_connect_one(s1,ip,port) #eth1
        list_elapsed = [ip,port,delay1]

    except Exception,e:
        loginfo = "ERROR in scan_all. error=%s"%(e)
        syslog.syslog(loginfo)
        list_elapsed[:] = []
        return 1
    pass
    s1.close()
    s1 = None

    del s1

    return list_elapsed
def new_evprobe_all (eth1,ip,port):
    #import pdb; pdb.set_trace()
    try:
        #if port == 0:  #判断无效端口
        #    list_result = []
        #    return list_result 
        #else:
        eth1 = get_config("env","eth1")
        list_result= scan_all(eth1,ip,port)
        if list_result[2] == 999.99:
            list_result = evping_connect(eth1,ip,port)
        return list_result
        #print (list_result[0],list_result[1],list_result[2])
    except Exception,e:
       loginfo = "ERROR in new_evprobe_all. error=%s"%(e)
       syslog.syslog(loginfo) 

# ------------------------------------------------------------
# Process records
def proc(pid,remote_ip,remote_port):
    #import pdb; pdb.set_trace()
    global g_flag_finish, g_list_result,send_result_all
    eth1 = get_config("env","eth1")
    while True :
        try:
            task = task_q.get(timeout=1) # decrements queue size by 1
            ip = copy.copy(task.ip)
            port = copy.copy(task.port)
            list_result_1 = new_evprobe_all(eth1,ip,port)
            if len(list_result_1)>0:
                info_t = ReportInfo_T()
                info_t.types = hton_type
                info_t.bid = hton_bid
                info_t.ip = ip
                info_t.port = port
                info_t.delay = list_result_1[2]
                g_list_result.append(info_t)
                list_result_1[:]=[]
            if len(g_list_result) == 10:
               send_all(remote_ip,remote_port)

            gevent.sleep(0)
        except Empty:
            break 
    return g_list_result
# Make tasks and put to task_q. 
def make_task(s,r,eth1,recv_res):
    #import pdb; pdb.set_trace()
    hton_type = socket.htonl(CLIENT_TO_SERVER_IP_RESULT_RES_MSG)
    hton_bid = socket.htonl(1) 
    list_hostinfo = recv_res #Get data from wangkekai.
    while True:
        if len(list_hostinfo) > 0:
            for line in list_hostinfo:
                ip = line[0]
                port= int(line[1]) 
                task = MyTask(ip,port)
                task_q.put_nowait(task)
            gevent.sleep(0)

        #print "make_task: out of sleep(0)"
        #将获取到的结果全部发送给王和凯.
        #import pdb; pdb.set_trace()
        #for r in g_list_result :
        #    print "result ip:%s,port:%s,delay:%s"%(r.ip,r.port,r.delay)
        #    sendudpmsg=struct.pack('!II4sHI',hton_type,hton_bid,socket.inet_aton(r.ip),int(r.port),long(r.delay))
        #   bytes = su.sendto(sendudpmsg,rs)
        #    time.sleep(0.1)
                    
        #释放g_list_result中的资源
        #g_list_result[:] = []               
        print "OK. size of task_q=",task_q.qsize()
        #g_flag_finish = True
        return

#把结果发送给王和凯
def send_all (remote_ip,remote_port):
    #create socket  TCP Protocol
    clientsock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    clientcon = clientsock.connect((remote_ip,int(remote_port)))
    
    #import pdb; pdb.set_trace()
    for r in g_list_result:
        sendudpmsg=struct.pack('!II4sHi',hton_type,hton_bid,socket.inet_aton(r.ip),int(r.port),long(r.delay))
        bytes = clientsock.send(sendudpmsg)
        if bytes > 0:
            global counter
            counter +=1
            print "result ip:%s,port:%s,delay:%s"%(r.ip,r.port,r.delay)
        else:
            print "Send error.."
    time.sleep(10)
    print "counter:%s"%counter
    #释放g_list_result中的资源
    g_list_result[:]=[] 

def evscan_main(s,r,remote_ip,remote_port,eth1,recv_res):
    #import pdb; pdb.set_trace()
    syslog.openlog("Clientbgp")
    loginfo = version_info+ " started..."
    syslog.syslog(loginfo)

    # Get configs
    list_hostinfo=[]
    loop_num = 1
    t_list=[]

    start_time = time.time() 
    threads = []
    threads.append(gevent.spawn(make_task,s,r,eth1,recv_res))
    for i in xrange(1,1000):
        threads.append(gevent.spawn(proc,i,remote_ip,remote_port))
    gevent.joinall(threads)
    
    # send_all
    #send_all(remote_ip,remote_port)

    end_time = time.time()
    spend_time = end_time - start_time
    loginfo ="spend_time: (%d)" % (spend_time)
    syslog.syslog(loginfo)
    loginfo = "spend_time: (%0.2fMINs)" % (spend_time/60)
    syslog.syslog(loginfo)

    loginfo = "END..."
    syslog.syslog(loginfo)

    print "OK.END..."

#if __name__=="__main__":
#    ip="192.168.26.169"
#    port="22"
#    eth1="192.168.26.169"
#    new_evprobe_all(eth1,ip,port)
