#!/usr/bin/env python
# coding: utf-8

import gevent
from ping import *

import gevent.monkey
gevent.monkey.patch_socket()

'''
def ping_test(pid):
    eth0 = '121.18.213.37'
    print "pid=",pid
    verbose_ping('61.135.169.105',eth0)
    return
'''
def evping (ip,export):
    #import pdb; pdb.set_trace()
    r = verbose_ping(ip,export)
    return  r 


def evping_connect (eth0,eth1):
    list_elapsed = []  
    eth0 = '121.18.213.37'
    eth1 = '211.151.82.36'
    try:
        #Eth0 Ping Result
        ping_a = evping('180.153.210.33',eth0)
        p_list_a = ping_a.split(' ')
        #p_delay0,i0 = [p_list_a[6][5:],'A']

        #Eth1 Ping Result
        ping_b = evping('180.153.210.33',eth1)
        p_list_b = ping_a.split(' ')
        #p_delay1,i1 = [p_list_b[6][5:],'B']

        """
        #Eth6 Ping Result
        ping_c = evping(ip,eth6)
        p_list = ping_a.split(' ')
        p_delay6,i6 = [p_list[15][6:],'C']
        """

        if p_delay0 == 'itted,' and p_delay1 == 'itted,' :
            list_elapsed[:] = []
        else:
            list_elapsed = min([p_delay0,i0],[p_delay1,i1])

    except Exception,e:
        #loginfo = "ERROR in ping_all. error=%s"%(e)
        #syslog.syslog(loginfo)
        print 'test'

    return list_elapsed

def main():
    eth0 = '121.18.213.37'
    eth1 = '211.151.82.36'
    threads = []
    for i in xrange(1,2):
        #threads.append(gevent.spawn(ping_test,i))
        threads.append(gevent.spawn(evping_connect,eth0,eth1))
    gevent.joinall(threads)


    return


if __name__ == '__main__':
    main()

