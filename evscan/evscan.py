#!/usr/bin/env python
# -*- coding: utf-8 -*-

# This project use gevent to scan all objects. 

import MySQLdb as mdb
#import Queue
#import thread
import threading
#from   threading import Thread
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

import gevent.timeout
import gevent.monkey
gevent.monkey.patch_socket()

import gevent
from ping import *
import gevent.monkey
gevent.monkey.patch_socket()

task_q = Queue(maxsize=1000000)
MAX_THREAD_NUM = int(get_config("env","MAX_THREAD_NUM"))
g_begin = 1
g_flag_finish = False
g_list_result=[]
g_db_lock=threading.Lock()

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
        self.ip = ""
        self.status = ""
        self.port = 0
        self.delay = 0.0
        self.portchannel = ""
        self.time = ""
    def __del__(self):
        self.ip = ""
        self.status = ""
        self.port = 0
        self.delay = 0.0
        self.portchannel = ""
        self.time = ""

#-------------------------------------------------------------
def evping (ip,export):
    #import pdb; pdb.set_trace()
    r = verbose_ping(ip,export)
    return r

def evping_connect (ip,eth1_1,eth1):
    #import pdb; pdb.set_trace()
    list_elapsed = []  
    eth1 = get_config("env","eth1")
    eth1_1 = get_config("env","eth1_1")
    try:
        #Eth0 Ping Result
        ping_a = evping(ip,eth1_1)

        #Eth1 Ping Result
        ping_b = evping(ip,eth1)

        if ping_a is None and ping_b is None :
            list_elapsed[:] = []
        else:
            list_elapsed = min([ping_a,'A'],[ping_b,'B'])

    except Exception,e:
        loginfo = "ERROR evping_connect. error=%s"%(e)
        syslog.syslog(loginfo)

    return list_elapsed

    
# ------------------------------------------------------------
# Scan one interface only. 
def scan_connect_one(s,ip,port,local_ip,export):
    spend_time = 999.99
    try:
        start_time = time.time()
        with gevent.timeout.Timeout(2):
            #import pdb; pdb.set_trace()
            r = s.connect((ip,int(port)))
            bytes = s.send("Hello")
            if bytes==5 :
                end_time = time.time()
                spend_time = end_time - start_time
    except socket.error, msg:
        loginfo = "ERROR in scan_connect_one. Error message=%s "%(str(msg))
        syslog.syslog(loginfo)
        #loginfo ="ip=%s port=%s socketid=%d"%(ip,port,id(s))
        #syslog.syslog(loginfo)
        pass
    return spend_time,export

# ------------------------------------------------------------
def scan_all(eth1_1,eth1,ip,port):
    #import pdb; pdb.set_trace()
    # Create socket for all interface.
    #eth0
    s0 = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    s0.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
    s0.settimeout(2)
    s0.bind((eth1_1, 0))

    #eth1
    s1 = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    s1.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
    s1.settimeout(2)
    s1.bind((eth1, 0))

    list_elapsed = []
    try:
        #import pdb; pdb.set_trace()
        delay0,i0 = scan_connect_one(s0,ip,port,eth1_1,'A') #eth0
        delay1,i1 = scan_connect_one(s1,ip,port,eth1,'B') #eth1
        list_elapsed = min([delay0,i0],[delay1,i1])

        #list_elapsed = min([delay0,i0],[delay6,i6])
        #if list_elapsed[0]==999.99 :         
        #   list_elapsed[:] = []
                     
            # ping test all port
            # ping_all()
    except Exception,e:
        loginfo = "ERROR in scan_all. error=%s"%(e)
        syslog.syslog(loginfo)
        list_elapsed[:] = []
        pass

    s0.close()
    s1.close()
    s0 = None
    s1 = None
 
    del s0
    del s1

    return list_elapsed

def new_evprobe_all (ip,port):
    eth1_1 = get_config("env","eth1_1")
    eth1 = get_config("env","eth1")
    list_result= scan_all(eth1_1,eth1,ip,port)
    if list_result[0] == 999.99:
        list_result = evping_connect(ip,eth1_1,eth1)
    return list_result
# ------------------------------------------------------------
# Process records.
def proc(pid):
    #import pdb; pdb.set_trace()
    global g_flag_finish
    eth1_1 = get_config("env","eth1_1")
    eth1 = get_config("env","eth1")

    while True :
        try:
            task = task_q.get(timeout=1) # decrements queue size by 1
            ip = copy.copy(task.ip)
            port = copy.copy(task.port)
            #print "pid=%d ip=%s port=%s"%(pid,ip,port)
            #list_result1 = scan_all(eth0,eth1,ip,port)
            #list_result = evping_connect(ip,eth0,eth1,eth6)
            list_result1 = new_evprobe_all(ip,port)            
            if len(list_result1)>0:
                info_t = ReportInfo_T()
                info_t.ip = ip
                info_t.port = port
                info_t.delay = list_result1[0]
                info_t.portchannel = list_result1[1]
                info_t.status = 'up'
                info_t.time = time.strftime('%Y-%m-%d %H:%M:%S',time.localtime(time.time()))
                              
                global g_list_result
                #global g_db_lock
                #g_db_lock.acquire()
                g_list_result.append(info_t)
                #g_db_lock.release()

            list_result1[:]=[]
            gevent.sleep(0)

        
        except Empty:
            if g_flag_finish == True:
                break

    return

# ------------------------------------------------------------
# Get records from MySQL.
def get_new_data(conn,cur,begin,num):
    sql = "SELECT ip,port FROM acceleration WHERE counter >1 LIMIT  %s,%s" %(begin,num)
    cur.execute(sql)
    list_result = cur.fetchall()
    loginfo = "get_new_data: sql= %s"%(sql)
    syslog.syslog(loginfo)
    return list_result

# ------------------------------------------------------------
# Make tasks and put to task_q. 
def make_task(conn,cur,conn_local,cur_local,end_num,read_num):
    global g_begin, g_flag_finish
    #global g_db_lock

    while True:
        #import pdb; pdb.set_trace()
        if g_begin <= end_num :
            list_hostinfo = get_new_data(conn,cur,g_begin,read_num)
            for line in list_hostinfo:
                ip = line[0]
                port= int(line[1])
                task = MyTask(ip,port)
                task_q.put_nowait(task)
            g_begin += read_num
            gevent.sleep(0)
            print "g_begin = %s"% g_begin
            #print "make_task: out of sleep(0)"
            #Write g_list_result to MySQL.
            #将获取到的结果list全部写入到mysql.
            #import pdb; pdb.set_trace()       
            r_list=[]
            for r in g_list_result :
                r_list.append([r.ip,r.status,r.port,r.delay,r.portchannel,r.time])
            sql = "INSERT INTO vianet_pass_zj1(ip,status,port,delay_time,port_channel,insert_time) VALUES(%1s,%2s,%3s,%4s,%5s,%6s)"
            cur_local.executemany(sql,r_list)
            conn_local.commit()
            loginfo="r_list = %s"%len(r_list)
            syslog.syslog(loginfo)
            
            #释放g_list_result中的资源
            r_list[:] = []
            g_list_result[:] = []
                   
        else:
            break

    print "OK. size of task_q=",task_q.qsize()
    g_flag_finish = True
    return


#-------------------------------------------------------------------------------
# usage:
#       python evscan.py -s 1 -e 10000 -n 1000
# Process Start ....
def main():
    syslog.openlog("checkport")
    loginfo = version_info+ " started..."
    syslog.syslog(loginfo)

    # global...
    global g_begin,MAX_THREAD_NUM

    end_num = 1000000
    read_num = 1000

    # Get args.
    opts,args = getopt.getopt(sys.argv[1:],'vs:e:n:')
    for o,a in opts:
        if (o=='-V' or o=='-v'):
            #print version_info
            sys.exit()
        elif (o=='-s'):
            #print "s=%s a=%s"%(o,a)
            g_begin = int(a)
        elif (o=='-e'):
            #print "e=%s a=%s"%(o,a)
            end_num = int(a)
        elif (o=='-n'):
            #print "n=%s a=%s"%(o,a)
            read_num = int(a)
        else:
            print 'Usage: python thinkrouting.py -s 1 -e 10000 -n 1000'

    # Get configs.
    remote_ip = get_config("remote_db", "db_ip")
    remote_user = get_config("remote_db", "login_name")
    remote_pwd = get_config("remote_db", "login_pwd")
    remote_dbname = get_config("remote_db", "db_name")

    local_ip = get_config("local_db", "db_ip")
    local_user = get_config("local_db", "login_name")
    local_pwd = get_config("local_db", "login_pwd")
    local_dbname = get_config("local_db", "db_name")

    # Connect to remote MySQL.
    conn = mdb.connect(remote_ip,remote_user,remote_pwd,remote_dbname)
    cur = conn.cursor()

    #Connect to local MySQL
    conn_local = mdb.connect(local_ip,local_user,local_pwd,local_dbname)
    cur_local = conn_local.cursor()


    list_hostinfo=[]
    loop_num = 1
    t_list=[]
    start_time = time.time()
    
    threads = []
    threads.append(gevent.spawn(make_task,conn,cur,conn_local,cur_local,end_num,read_num))
    for i in xrange(1,MAX_THREAD_NUM):
        threads.append(gevent.spawn(proc,i))
    gevent.joinall(threads)

    end_time = time.time()
    spend_time = end_time - start_time
    loginfo ="spend_time: (%d)" % (spend_time)
    syslog.syslog(loginfo)
    loginfo = "spend_time: (%0.2fMINs)" % (spend_time/60)
    syslog.syslog(loginfo)

    loginfo = "END..."
    syslog.syslog(loginfo)

    print "OK.END..."

# ------------------------------------------------------------
if __name__ == '__main__':
     main()
