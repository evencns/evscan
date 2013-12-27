/*

 * thread_pool.cpp
 *
 *  Created on: Oct 29, 2013
 *      Author: wanghekai
 */

#include "thread_pool.h"
#include <sys/epoll.h>
#include "server_log.h"
#include <assert.h>
#include <pthread.h>
#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <netinet/in.h>
#include "common_header.h"
#include "server_mutex.h"
#include <list>
#include "mysql_db.h"
#include <arpa/inet.h>

#define THREAD_MAX_EVENT_NUM 1024
#define MAX_EVENT_NUMBER 1024 //最大事件数目
#define TCP_BUFFER_SIZE 1024//TCP缓冲区
#define UDP_BUFFER_SIZE 1024//UDP缓冲区

extern std::list<server_task> g_task_list;
extern ip_map_t g_ip_map;
static pthread_mutex_t mutex_map = PTHREAD_MUTEX_INITIALIZER;
extern int epollfd;

//for network conn task
void * network_task_process(void * arg)
{
	while(true)
	{
		server_task task;
		CServerMutex o_mutex;
		if(g_task_list.size() > 0 )
		{
			//get task
			task = g_task_list.front();
			g_task_list.pop_front();
		}
		else
		{
			o_mutex.unlock();
			sleep(1);
			continue;
		}
		o_mutex.unlock();
		switch (task.msg_type)
		{
			case LOCAL_RECV_PEER_CONNECT_MSG:
			{
				//recv_client_req(task.fd);
			}
			break;
			case SERVER_TO_CLIENT_IP_RESULT_REQ_MSG:
			{

			}
			break;
			default :
			{
				printf("thread task msg type is %d\n",task.msg_type);
			}
			break;
		}
		sleep(1);
	}
	return NULL;
}

//create thread pool
int create_thread_pool(int num)
{
	pthread_attr_t attr;
	pthread_t pthread_id;
	pthread_attr_init(&attr);
	pthread_attr_setscope(&attr,PTHREAD_SCOPE_SYSTEM);
	pthread_attr_setstacksize(&attr,20971520);
	pthread_attr_setdetachstate(&attr,PTHREAD_CREATE_DETACHED);

	for(int i = 0; i< num;i++)
	{
		pthread_create(&pthread_id,&attr,network_task_process,NULL);
		sleep(1);
	}
	return 0;
}

int global_num = 0;

int create_udp_task(const char * buffer,int buffer_len)
{
	printf("receive buf len %d\n",buffer_len);
	char my_buffer[buffer_len+1];
	bzero(my_buffer,buffer_len+1);
	memcpy(my_buffer,buffer,buffer_len);
	//parse notify msg
	char * p_temp = my_buffer;
	uint32_t type = ntohl(*(uint32_t * )p_temp);

	switch (type)
	{
		case CM_TO_SERVER_IP_LIST_NOTIFY_MSG:
		{
			if(buffer_len == sizeof(cm_to_se_iplist_notify)){
				//network order peer addr , port ,type
				cm_to_se_iplist_notify notify = *(cm_to_se_iplist_notify *)buffer;
				db_key key;
				key.bid = ntohl(notify.bid);
				key.ip  = ntohl(notify.ip);
				db_value value;
				value.active = ntohl(notify.active);
				value.aid  = ntohl(notify.aid);
				value.counter = ntohl(notify.counter);
				//init delay
				value.delay = -1;
				value.pid = ntohl(notify.pid);
				value.port = ntohs(notify.port);
				value.update_time = ntoh64(notify.update_time);
				value.via_time = ntoh64(notify.via_time);
				value.protocol_type = ntohl(notify.pro_type);
				pthread_mutex_lock(&mutex_map);
				g_ip_map[key] = value;
				pthread_mutex_unlock(&mutex_map);
				//db update ?
				CServerDB::get_instance()->insert_cur_data_info(key,value);
			}
		}
		break;
		case CLIENT_TO_SERVER_IP_LIST_NOTIFY_MSG:
		{
			if(buffer_len == sizeof(client_notify))
			{
				client_notify notify = *(client_notify*)buffer;
				int sockfd = create_tcp_conn(ntohl(notify.addr),ntohs(notify.port));
				assert(sockfd >= 0);
				//send request ,and wait response
				se_to_client_result_req req;
				req.type = SERVER_TO_CLIENT_IP_RESULT_REQ_MSG;
				req.type = htonl(req.type);
#if 0
				send(sockfd,&req,sizeof(req),MSG_DONTWAIT);
				set_nonblocking(sockfd);
				epoll_event event;
				event.events = EPOLLIN | EPOLLET;
				fd_data * pdata = new fd_data;
				pdata->fd = sockfd;
				event.data.ptr = pdata;
				epoll_ctl(epollfd,EPOLL_CTL_ADD,sockfd,&event);
#endif
			}
		}
		break;
		case CLIENT_TO_SERVER_IP_RESULT_RES_MSG:
		{
			global_num++;
			printf("receive client's result udp msg num %d\n",global_num);
#if 0
			if(buffer_len == sizeof(client_to_se_result_res))
			{
				client_to_se_result_res res;
				bzero(&res,sizeof(client_to_se_result_res));
				res = *(client_to_se_result_res*)buffer;
				in_addr addr;
				addr.s_addr = res.ip;
				printf("recive res type %d bid:%d ip %s port %d delay %d\n",
						ntohl(res.type),ntohl(res.bid),inet_ntoa(addr),ntohs(res.port),
						ntohl(res.delay));
				//time_t stamp = time(NULL);

				db_key key;
				key.bid = ntohl(res.bid);
				key.ip = ntohl(res.ip);
				//update cur map delay
				pthread_mutex_lock(&mutex_map);
				g_ip_map[key].delay = ntohl(res.delay);
				db_value value = g_ip_map[key];
				pthread_mutex_unlock(&mutex_map);
				//update db
				//CServerDB::get_instance()->insert_cur_data_info(key,value);
				//CServerDB::get_instance()->insert_history_data_info(stamp,key,value);

			}
			else
			{
				printf("receive invalid client's result msg\n");
			}
#endif
		}
		break;
		default:
		{
			printf("receive invalid udp msg\n");
			LOG_INFO_MSG("UDP TASK RECEIVE INVALID MSG");
		}
		break;
	}
	return 0;
}

//create new socket ,and connect peer addr
//para ip and port is host byte order
int create_tcp_conn(uint32_t ip,uint16_t port)
{
	//create socket
	sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = htonl(ip);
	addr.sin_port = htons(port);
	int sockfd = socket(AF_INET,SOCK_STREAM,0);
	assert(sockfd >= 0);
	//connnect
	int ret = connect(sockfd,(struct sockaddr *)&addr,sizeof(addr));
	if( 0 != ret){
		close(sockfd);
		return -1;
	}
	return sockfd;
}

int set_nonblocking(int fd)
{
    int old_option = fcntl( fd, F_GETFL );
    int new_option = old_option | O_NONBLOCK;
    fcntl( fd, F_SETFL, new_option );
    return old_option;
}

//注册事件
void add_fd( int epollfd, int fd )
{
    epoll_event event;
    event.data.fd = fd;
    event.events = EPOLLIN | EPOLLET;
    epoll_ctl( epollfd, EPOLL_CTL_ADD, fd, &event );
    set_nonblocking( fd );
}

void modify_fd(int epollfd,int fd)
{
    epoll_event event;
    event.data.fd = fd;
    event.events = EPOLLIN | EPOLLET;
    epoll_ctl( epollfd, EPOLL_CTL_MOD, fd, &event );
    set_nonblocking( fd );
}


void on_tcp_read(epoll_event * ev)
{
	assert(NULL != ev->data.ptr);
	const int length = 1024;
	char buffer[length+1];
	bzero(buffer,length+1);
	fd_data * temp  = (fd_data *)ev->data.ptr;
	int fd = temp->fd;
	while(true){
		int n = recv(fd,buffer,length,0);
		if( n < 0 ){
			if(errno == EAGAIN || errno == EWOULDBLOCK){
				break;
			}
			else if(errno == EINTR){
				continue;
			}
			else{
				printf("tcp fd %d find a error %d",fd,errno);
				if(NULL != temp){
					delete temp;
					ev->data.ptr = NULL;
				}
				close(fd);
				ev->data.fd = -1;
				return ;
			}
		}
		else if( 0 == n ){
			printf("tcp peer close the tcp connection \n");
			if(NULL != temp){
				delete temp;
				temp = NULL;
				ev->data.ptr = NULL;
			}
			close(fd);
			ev->data.fd = -1;
			return;
		}
		else if ( n <= length){
			printf("again use recv msg peek for get type info\n");
			for(int i = 0; i < n; i++){
				temp->read_buffer.push_back(buffer[i]);
			}
			bzero(buffer,length+1);
			printf("cur read buffer size is %u\n",(uint32_t)temp->read_buffer.size());
			continue ;
		}
	}
process_msg:
	//this is a label for goto
	if(sizeof(uint32_t) >= temp->read_buffer.size()){
		return ;
	}
	char * str_buf_data = temp->read_buffer.data();
	uint32_t msg_type = ntohl((*(uint32_t*)str_buf_data));
	switch (msg_type)
	{
		case CLIENT_TO_SERVER_IP_LIST_REQ_MSG:
		{
			uint32_t req_size = sizeof(client_to_se_iplist_req);
			if(temp->read_buffer.size() < req_size ){
				//msg not full ,wait next time
				return ;
			}
			client_to_se_iplist_req req;
			bzero(&req,req_size);
			memcpy(&req,temp->read_buffer.data(),req_size);
			//erase data from temp->read_buffer
			temp->read_buffer.erase(temp->read_buffer.begin(),temp->read_buffer.begin()+req_size);
			//network byte order convert
			req.bid = ntohl(req.bid);
			req.ip = ntohl(req.ip);
			req.type = ntohl(req.type);
			if( 0 == req.ip)
			{
				ip_map_t::const_iterator iter_const;
				se_to_client_iplist_res res;
				temp->write_buffer.clear();
				//mutex begin
				pthread_mutex_lock(&mutex_map);
				//int i_count = 0;
				for(iter_const = g_ip_map.begin();iter_const != g_ip_map.end();iter_const++)
				{
					if(iter_const->first.bid == req.bid)
					{
						res.bid = htonl(iter_const->first.bid);
						res.ip = htonl(iter_const->first.ip);
						res.port = htons(iter_const->second.port);
						res.type = htonl(SERVER_TO_CLIENT_IP_LIST_RES_MSG);
						//res.type = htonl(res.type);
						in_addr addr ;
						addr.s_addr = htonl(iter_const->first.ip);
						for(int i = 0 ; i< 14;i++)
						{
							temp->write_buffer.push_back(*(((char *)(&res))+i));
						}
					}
				}
				pthread_mutex_unlock(&mutex_map);
				printf("temp write buffer all size %lu\n",temp->write_buffer.size());
#if 1
				ev->events = EPOLLIN | EPOLLOUT | EPOLLET;
				epoll_ctl(epollfd,EPOLL_CTL_MOD,fd,ev);
#endif
				if(temp->read_buffer.size() > 0)
				{
					goto process_msg;
				}
			}
			else
			{
				ip_map_t::const_iterator iter_const;
				db_key key;
				key.bid = req.bid;
				key.ip = req.ip;
				se_to_client_iplist_res res;
				res.bid = htonl(req.bid);
				res.ip = htonl(req.ip);
				res.type = SERVER_TO_CLIENT_IP_LIST_RES_MSG;
				res.type = htonl(res.type);
				pthread_mutex_lock(&mutex_map);
				iter_const = g_ip_map.find(key);
				if(iter_const != g_ip_map.end())
				{
					//printf("find data ok \n");
					//construct res msg ,and send it to client
					res.port = htons(iter_const->second.port);
				}
				else
				{
					//printf("not find data ,res invalid data to client\n");
					res.port = htons(0);
				}
				for(int i = 0 ; i< 14;i++)
				{
					temp->write_buffer.push_back(*(((char *)(&res))+i));
				}
				printf("temp write buffer all size %lu\n",temp->write_buffer.size());
#if 1
				ev->events = EPOLLIN | EPOLLOUT | EPOLLET;
				epoll_ctl(epollfd,EPOLL_CTL_MOD,fd,ev);
#endif
				pthread_mutex_unlock(&mutex_map);
				if(temp->read_buffer.size() > 0)
				{
					goto process_msg;
				}
			}
		}
		break;
		case CLIENT_TO_SERVER_IP_RESULT_RES_MSG:
		{
			printf("receive tcp client to server result msg \n");
			uint32_t res_size = sizeof(client_to_se_result_res);
			if(temp->read_buffer.size() < res_size)
			{
				return ;
			}
			client_to_se_result_res res;
			bzero(&res,res_size);
			memcpy(&res,temp->read_buffer.data(),res_size);

			in_addr addr;
			addr.s_addr = res.ip;
			printf("recive res type %d bid:%d ip %s port %d delay %d\n",
					ntohl(res.type),ntohl(res.bid),inet_ntoa(addr),ntohs(res.port),
					ntohl(res.delay));

			//erase data from temp->read_buffer
			temp->read_buffer.erase(temp->read_buffer.begin(),temp->read_buffer.begin()+res_size);

			time_t stamp = time(NULL);
			db_key key;
			key.bid = ntohl(res.bid);
			key.ip = ntohl(res.ip);
			//update cur map delay
			pthread_mutex_lock(&mutex_map);
			g_ip_map[key].delay = ntohl(res.delay);
			db_value value = g_ip_map[key];
			pthread_mutex_unlock(&mutex_map);
			//update db
			CServerDB::get_instance()->insert_cur_data_info(key,value);
			CServerDB::get_instance()->insert_history_data_info(stamp,key,value);
			//reenable the connection
			ev->events = EPOLLIN | EPOLLET;
			epoll_ctl(epollfd,EPOLL_CTL_MOD,fd,ev);

			if(temp->read_buffer.size() > 0)
			{
				goto process_msg;
			}
		}
		break;
		default:
		{
			printf("tcp receive invalid msg\n");
		}
		break;
	}
}

void on_tcp_write(epoll_event * ev)
{
	assert(NULL != ev);
	assert(NULL != ev->data.ptr);
	fd_data * temp  = (fd_data *)ev->data.ptr;
	size_t size = 1024;
	int fd = temp->fd;
	printf("on write cur buf size is %d \n",(int)temp->write_buffer.size());
	while(temp->write_buffer.size() > 0 )
	{
		size_t len = temp->write_buffer.size() > size ? size : temp->write_buffer.size();
		printf("send length is %d\n",(int)len);
		int n = send(fd,temp->write_buffer.data(),len,MSG_DONTWAIT);
		if( n < 0 )
		{
			if( errno == EAGAIN || errno == EWOULDBLOCK)
			{
				printf("write errno %d\n",errno);
				break;
			}
			else if(errno == EINTR)
			{
				printf("error EINTR\n");
				continue;
			}
			//receive error ,close the fd for other service
			else
			{
				printf("send find a error ,and error no %d close fd\n",errno);
				if(temp)
				{
					delete temp;
					temp = NULL;
				}
				close(fd);
				return ;
			}
		}
		temp->write_buffer.erase(temp->write_buffer.begin(),temp->write_buffer.begin()+n);
	}

	//send finished
	if( temp->write_buffer.size() == 0 )
	{
#if 0
		printf("modify fd for epoll in \n");
		ev->events = EPOLLIN |EPOLLOUT | EPOLLET;
		epoll_ctl(epollfd,EPOLL_CTL_MOD,fd,ev);
#endif
#if 0
		if(temp->read_buffer.size() > 0)
		{
		}
		else
		{
			printf("why close the connection\n");
			//suggest close the conn
			if(NULL != temp)
			{
				delete temp;
				temp = NULL;
			}
			close(fd);
			ev->data.fd = -1;
		}
#endif
	}
	//means this time not send ok ,wait next time
	else{
#if 0
		ev->events = EPOLLIN |EPOLLOUT | EPOLLET;
		epoll_ctl(epollfd,EPOLL_CTL_MOD,fd,ev);
#endif
		printf("last write else find \n");
	}
	return ;
}


