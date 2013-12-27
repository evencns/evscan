/*
 * thread_pool.h
 *
 *  Created on: Oct 29, 2013
 *      Author: root
 */

#ifndef THREAD_POOL_H_
#define THREAD_POOL_H_
#include "common_header.h"



//for process network
const int i_max_thread_num = 8;

void * network_task_process(void * arg);
//create thread pool
int create_thread_pool(int num);

int create_udp_task(const char * buffer,int buffer_len);

//create new socket ,and connect peer addr
int create_tcp_conn(uint32_t ip,uint16_t port);

//设置为非阻塞描述符
int set_nonblocking(int fd);

//注册事件
void add_fd( int epollfd, int fd );

void modify_fd(int epollfd,int fd);

void on_tcp_read(epoll_event * ev);

void on_tcp_write(epoll_event * ev);



#endif /* THREAD_POOL_H_ */
