/*
 * main.cpp
 *
 *  Created on: Apr 25, 2013
 *      Author: wanghekai
 */

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <assert.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/epoll.h>
#include <pthread.h>

#define MAX_EVENT_NUMBER 1024 //最大事件数目
#define TCP_BUFFER_SIZE 1024//TCP缓冲区
#define UDP_BUFFER_SIZE 1024//UDP缓冲区

#include "server_log.h"
#include <sys/time.h>
#include <sys/resource.h>
#include <syslog.h>
#include "common_header.h"
#include "thread_pool.h"
#include <list>
#include "mysql_db.h"
#include "server_mutex.h"

//global task list ,need mutex for access it
std::list<server_task> g_task_list;
ip_map_t g_ip_map;
int epollfd = 0;
const char * db_user = "root";
const char * db_pass =  "";

//rlimit resources set
int system_set(void)
{
	struct rlimit rt;
	if(getrlimit(RLIMIT_CORE, &rt) == 0)
	{
		rt.rlim_max = 0XFFFFFFFF;
		rt.rlim_cur = 0XFFFFFFFF;
		if (setrlimit(RLIMIT_CORE, &rt) != 0)
		{
			LOG_INFO_MSG("setrlimit core Failed \n");
		}
	}
	return 0;
}

int main( int argc, char* argv[] )
{
	printf("via_server Start , Please wait ...\n");
	openlog("via_server", LOG_NDELAY, LOG_USER);
	syslog (LOG_INFO, "via_server Start Begin ...");
	system_set();

	CServerDB * g_instance = CServerDB::get_instance();
	if(NULL == g_instance)
	{
		LOG_INFO_MSG("Get DB Instance Failed");
		return -1;
	}
	//connect db
	if (-1 == g_instance->connect_db())
	{
		LOG_INFO_MSG("CONNECT DB FAILED");
		return -1;
	}
	//get all cur_data_info to map
	g_instance->get_all_cur_data_info(g_ip_map);

	//init thread pool for net task process
	g_task_list.clear();
	create_thread_pool(1);

	const char * default_ip = "localhost";
	const int default_port = 10000;
	const char * ip = default_ip;
	int port = default_port;

	if( argc == 2){
		ip = argv[1];
		port = atoi(argv[2]);
	}
    int ret = 0;

    //绑定TCP端口
    struct sockaddr_in address;
    bzero( &address, sizeof(address));
    address.sin_family = AF_INET;
    inet_pton( AF_INET, ip, &address.sin_addr );
    address.sin_port = htons(port);
    int tcpfd = socket(AF_INET,SOCK_STREAM,0);
    assert( tcpfd >= 0 );
    int flag = 1;
    setsockopt(tcpfd,SOL_SOCKET,SO_REUSEADDR,&flag,sizeof(int));
    ret = bind(tcpfd,(struct sockaddr*)&address,sizeof(address));
    assert(-1 != ret);
    ret = listen(tcpfd,100);
    assert( ret != -1 );

    //绑定UDP端口
    bzero(&address,sizeof(address));
    address.sin_family = AF_INET;
    inet_pton(AF_INET,ip,&address.sin_addr);
    address.sin_port = htons(port);
    int udpfd = socket(AF_INET,SOCK_DGRAM,0);
    assert( udpfd >= 0 );
    ret = bind(udpfd,(struct sockaddr*)&address,sizeof(address));
    assert( ret != -1 );
    int buf_len = 1024 * 1024 * 100;
    setsockopt(udpfd,SOL_SOCKET,SO_RCVBUF,(const char *)&buf_len,sizeof(int));

    epoll_event events[MAX_EVENT_NUMBER];
    epollfd = epoll_create(1024);
    assert( epollfd != -1 );
    add_fd( epollfd, tcpfd );//TCP端口注册事件
    add_fd( epollfd, udpfd );//UDP端口注册事件

    while(true)
    {
    	//无限期等待事件发生
        int number = epoll_wait( epollfd, events, MAX_EVENT_NUMBER, -1 );
        if ( number < 0 ){
        	LOG_INFO_MSG("epoll failed");
            break;
        }
        //EPOLL就绪事件
        for ( int i = 0; i < number; i++ ){
            int sockfd = events[i].data.fd;
            //监听端口监听TCP连接事件
            if ( sockfd == tcpfd ){
            	printf("receive tcp fd\n");
                struct sockaddr_in addr;
                socklen_t addr_len = sizeof(addr);
                int connfd = accept(tcpfd,(struct sockaddr*)&addr,&addr_len);
                set_nonblocking(connfd);
                epoll_event event;
                event.data.fd = connfd;
                event.events = EPOLLIN | EPOLLOUT | EPOLLET;
                event.data.ptr = new fd_data;
                ((fd_data*)event.data.ptr)->fd = connfd;
                epoll_ctl( epollfd, EPOLL_CTL_ADD, connfd, &event );
            }
            //UDP连接
            else if (sockfd == udpfd){
            	printf("receive udp fd msg\n");
                char buf[ UDP_BUFFER_SIZE ];
                bzero(buf,UDP_BUFFER_SIZE);
                struct sockaddr_in addr;
                socklen_t addr_len = sizeof(addr);
                //addr is network addr (network byte order)
                ret = recvfrom(udpfd,buf,UDP_BUFFER_SIZE-1,0,(struct sockaddr*)&addr,&addr_len);
                //parse msg and create correspond task
                if(ret > 0){
                	//convert netowrk byte order to host byte order
                	create_udp_task(buf,ret);
                }
                modify_fd(epollfd,udpfd);
            }
            else if ( events[i].events & EPOLLIN ){
            	printf("tcp epoll in\n");
            	on_tcp_read(&events[i]);
            }
            else if(events[i].events * EPOLLOUT){
            	printf("tcp epoll out\n");
            	on_tcp_write(&events[i]);
            }
            else{
                printf( "something else happened \n" );
            }
        }
    }

    close( tcpfd );
    return 0;
}