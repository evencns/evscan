#!/usr/bin/env python
# -*- coding: utf-8 -*-

import MySQLdb as mdb
import Queue
import thread
import threading
from   threading import Thread
import getopt
import time,sys,string
#import nmap
import syslog
import copy
import socket
import subprocess
from utils import * 

version_info="checkport 1.0 build 01.20130226"
#MAX_THREAD_NUM = 48 # 开启的最大的线程数
MAX_THREAD_NUM = int(get_config("env","MAX_THREAD_NUM"))
task_q = Queue.Queue()
g_thread_num = 1
g_begin = 1
g_putq_timeout = 0.005 # S
g_working_num = 0
#nm = nmap.PortScanner()
g_list_result=[]
#g_db_lock=threading.Lock()

#timea = time.strftime('%Y-%m-%d %H:%M:%S',time.localtime(time.time()))
socket.setdefaulttimeout(1)# set timeout 1 second.
#eth0 = "121.18.213.37"
#eth0 = "192.168.19.102"
#eth1 = "211.151.82.36"
#eth6 = "192.168.38.234"

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

#define task queue.------------------------------------------------------------
class MyTask(object):
    def __init__(self, ip,port):
        self.ip = ip
        self.port = str(port)

#define Threading -------------------------------------------------------------
class MyThread(threading.Thread):
    def __init__(self,task_q):
        threading.Thread.__init__(self);
        self.task_q = task_q
        self.eth0 = get_config("env","eth0")
        self.eth1 = get_config("env","eth1")
        self.eth6 = get_config("env","eth6")

    
    # scan all interface.
    def ping_host(self,ethx,ip):
        #import pdb; pdb.set_trace()
        r = ""
        try:
            handle = subprocess.Popen('ping -I %s -c 1 -w 1 %s' % (ethx,ip),stdout=subprocess.PIPE,stderr=subprocess.PIPE,shell=True) 
            r = handle.stdout.read()
            return  r
            
        except Exception,e:
             loginfo = "ERROR in ping_host. error=%s"%(e)
             syslog.syslog(loginfo)

    def ping_all(self,ip,port):
        #import pdb; pdb.set_trace()
        elapsed = []
        eth0='eth0'
        eth1='eth1'
        eth6='eth6'
        try:   
            # eth0 ping result.
            p_s1 = self.ping_host(eth0,ip)
            p_list = p_s1.split(' ')
            p_delay0,i0 = [p_list[15][6:],'A']

            #eth1 ping result.
            p_s2 = self.ping_host(eth1,ip)
            p_list = p_s2.split(' ')
            p_delay1,i1 = [p_list[15][6:],'B']

            #eth6 ping result
            '''
            p_s6 = self.ping_host(eth6,ip)
            p_list = p_s6.split(' ')
            p_delay6,i6 = [p_list[15][6:],'C']

            '''
            if p_delay0 == 'itted,' and p_delay1 == 'itted,' :
                elapsed[:] = []
            else:
                elapsed = min([p_delay0,i0],[p_delay1,i1])

        except Exception,e:
            loginfo = "ERROR in ping_all. error=%s"%(e)
            syslog.syslog(loginfo)

        return elapsed

        # Scan one interface only. 
    def scan_connect_one(self,s,ip,port,local_ip,export):
        spend_time = 999.99
        try:
            start_time = time.time()
            #s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            #s.setsockopt( socket.SOL_SOCKET, socket.SO_REUSEADDR, 1 )
            #s.settimeout(1)
            #s.bind( (local_ip, 0) )
            #import pdb; pdb.set_trace()
            r = s.connect((ip,int(port)))
            bytes = s.send("hello")
            if bytes==5 :
                end_time = time.time()
                spend_time = end_time - start_time
        except socket.error, msg:
            loginfo = "ERROR in scan_connect_one. Error message=%s,ip=%s port=%s "%(str(msg),ip,port)
            syslog.syslog(loginfo)
            #loginfo ="ip=%s port=%s socketid=%d"%(ip,port,id(s))
            #syslog.syslog(loginfo)
             pass
        #s.close()
        #s = None
        #del s
        return spend_time,export


    def scan_all(self,ip,port):
        #s2_ping = self.ping_host(ip,eth1)
        #s6_ping = self.ping_host(ip,eth6)

        # Create socket for all interface.
        #eth0
        s0 = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        s0.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        s0.settimeout(1)
        s0.bind((self.eth0, 0))

        #eth1
        s1 = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        s1.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        s1.settimeout(1)
        s1.bind((self.eth1, 0))

        #eth6
        s6 = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        s6.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        s6.settimeout(1)
        s6.bind((self.eth6, 0))

        elapsed = []
        try:
            
            delay0,i0 = self.scan_connect_one(s0,ip,port,self.eth0,'A')#eth0
            delay1,i1 = self.scan_connect_one(s1,ip,port,self.eth1,'B') #eth1
            delay6,i6 = self.scan_connect_one(s6,ip,port,self.eth6,'C')#eth6
            elapsed = min([delay0,i0],[delay1,i1],[delay6,i6])
            #delay6 = 999.99
            #i6 = 'B'
            #elapsed = min([delay0,i0],[delay6,i6])
            if elapsed[0]==999.99:
                elapsed[:] = []
        except Exception,e:
            loginfo = "ERROR in scan_all. error=%s"%(e)
            syslog.syslog(loginfo)
            elapsed[:] = []
            pass

        s0.close()
        s1.close()
        s6.close()
        s0 = None
        s1 = None
        s6 = None
        del s0
        del s1
        del s6

        return elapsed

    def proc(self,ip,port):
        try:
            #result = self.scan_all(ip,port)
            result = self.ping_all(ip,port)
            if len(result)>0:
                info_t = ReportInfo_T()
                info_t.ip = ip
                info_t.port = port
                info_t.delay = result[0]
                info_t.portchannel = result[1]
                info_t.status = 'up'
                info_t.time = time.strftime('%Y-%m-%d %H:%M:%S',time.localtime(time.time()))
        
                global g_list_result
                #global g_db_lock
                #g_db_lock.acquire()
                g_list_result.append(info_t)
                #g_db_lock.release()
            result[:]=[]
        except Exception,e:
            loginfo = "ERROR in proc. error=%s"%(e)
            syslog.syslog(loginfo)
            pass
        return

    def run(self):
        global g_thread_num,g_working_num
        num_loop = 0
        try:
            while True:
                if self.task_q.qsize()>0:
                    #print "run: self.task_q.qsize=",self.task_q.qsize()
                    g_working_num += 1
                    task = self.task_q.get()
                    ip = copy.copy(task.ip)
                    port = copy.copy(task.port)
                    self.proc(ip,port)
                    num_loop = 0
                    g_working_num -= 1
                    #continue
                else:
                    #有可能会从数据库中读取到新的记录。
                    if num_loop < 300 :# 5min
                        num_loop += 1
                        time.sleep(1)
                        #print "OK. break. task_q.qsize=",self.task_q.qsize()
                        #break 
                    else:
                        break 
        except Exception,e:
            loginfo = "ERROR in run. error=%s"%(e)
            syslog.syslog(loginfo)
            pass

        self.task_q.task_done()
        """
        self.s0.close()
        self.s1.close()
        self.s6.close()
        self.s0 = None
        self.s1 = None
        self.s6 = None
        del self.s0
        del self.s1
        del self.s6
        """

        #print "task_done. return."
        if g_thread_num>0 :
            g_thread_num -= 1
        return

 
#Get Mysql list ----------------------------------------------------------------  
def get_new_data(cur,begin,num):
    sql = "SELECT ip,port FROM acceleration where active='1' limit %s,%s" %(begin,num)
    cur.execute(sql)
    list_result = cur.fetchall()
    loginfo = "get_new_data: sql= %s"%(sql)
    syslog.syslog(loginfo)

    return list_result

#-------------------------------------------------------------------------------
# usage:
#       python thinkrouting.py -s 1 -e 10000 -n 1000
#
def main():
    #print version_info ," Starting..."
    syslog.openlog("checkport")
    loginfo = version_info+ " started..."
    syslog.syslog(loginfo)

    global g_begin, g_putq_timeout, g_list_result, g_thread_num, g_working_num,MAX_THREAD_NUM

    end_num = 10000
    read_num = 1000

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

    #import pdb; pdb.set_trace()
    remote_ip = get_config("remote_db", "db_ip")
    remote_user = get_config("remote_db", "login_name")
    remote_pwd = get_config("remote_db", "login_pwd")
    remote_dbname = get_config("remote_db", "db_name")

    local_ip = get_config("local_db", "db_ip")
    local_user = get_config("local_db", "login_name")
    local_pwd = get_config("local_db", "login_pwd")
    local_dbname = get_config("local_db", "db_name")


    conn = mdb.connect(remote_ip,remote_user,remote_pwd,remote_dbname)
    #conn = mdb.connect('127.0.0.1','root','','vianet_export')
    cur = conn.cursor()
    conn_local = mdb.connect(local_ip,local_user,local_pwd,local_dbname)
    #conn_local = mdb.connect('127.0.0.1','root','','vianet_export')
    cur_local = conn_local.cursor()

    
    #global g_db_lock
    list_hostinfo=[]
    #num = 5000
    loop_num = 1

    t_list=[]
    start_time = time.time()

    #提前创建200个线程
    for i in range(1,MAX_THREAD_NUM):
        t = MyThread(task_q)
        #t.setDaemon(True)
        t.start()
        time.sleep(g_putq_timeout)
    time.sleep(5)

    while True:
        if g_begin <= end_num :
            loginfo = "loop_num=%s"%(loop_num)
            syslog.syslog(loginfo)
            list_hostinfo = get_new_data(cur,g_begin,read_num)

            for line in list_hostinfo:
                ip = line[0]
                port= int(line[1])
                task = MyTask(ip, port)
                task_q.put(task)
                time.sleep(0.0005)

            i = 0
            while g_working_num >0 and i<6:
                #print "g_working_num=",g_working_num
                time.sleep(10)
                i +=1

            #将获取到的结果list全部写入到mysql.
            #g_db_lock.acquire()
            #import pdb; pdb.set_trace()
            loginfo = "g_list_result=%s"%len(g_list_result)
            syslog.syslog(loginfo)
            '''
            for r in g_list_result:
                sql = "INSERT INTO vianet_pass_zj1(ip,status,port,delay_time,port_channel,insert_time) VALUES('%s','%s','%s','%s','%s','%s')"\
                    %(r.ip,r.status,r.port,r.delay,r.portchannel,r.time)
                cur_local.execute(sql)
                conn_local.commit()
                #import pdb; pdb.set_trace()
                #print "r.ip=",r.ip
            '''
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
            #g_db_lock.release()

            g_begin += read_num
            loop_num += 1
        else:
            break
        
        #print "threading.activeCount=%d  g_begin=%d task_q.qsize=%d g_working_num=%d"%(threading.activeCount(),g_begin,task_q.qsize(),g_working_num)

    end_time = time.time()
    spend_time = end_time - start_time
    loginfo ="spend_time: (%d)" % (spend_time)
    syslog.syslog(loginfo)
    loginfo = "spend_time: (%0.2fMINs)" % (spend_time/60)
    syslog.syslog(loginfo)
    loginfo ="g_thread_num=%s" % g_thread_num
    syslog.syslog(loginfo)
    loginfo = "END..."
    syslog.syslog(loginfo)

if __name__ == "__main__":
    main()
