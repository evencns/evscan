/*
 * common_header.h
 *
 *  Created on: Oct 29, 2013
 *      Author: wanghekai
 */

#ifndef COMMON_HEADER_H_
#define COMMON_HEADER_H_

#include <stdint.h>
#include <time.h>
#include <map>
#include <endian.h>
#include <vector>
#include <string.h>
#include <strings.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/epoll.h>

#define IO_BUFFER_SIZE 2048

enum msg_type
{
	MSG_TYPE_BEGIN = 0,
	//client and server msg
	CLIENT_TO_SERVER_IP_LIST_REQ_MSG,
	SERVER_TO_CLIENT_IP_LIST_RES_MSG,
	CLIENT_TO_SERVER_IP_LIST_NOTIFY_MSG,
	SERVER_TO_CLIENT_IP_RESULT_REQ_MSG,
	CLIENT_TO_SERVER_IP_RESULT_RES_MSG,
	//server and cm msg
	CM_TO_SERVER_IP_LIST_NOTIFY_MSG = 100,
	//local msg define for local application
	LOCAL_RECV_PEER_CONNECT_MSG = 200,
	MSG_TYPE_END
};

enum ip_domain
{
	IP_DOMAIN_BEGIN = 0,
	IP_DOMAIN_CHINA_UNICOM,//联通
	IP_DOMAIN_CHINA_MOBILE,//移动
	IP_DOMAIN_CHINA_TELCOM,//电信
	IP_DOMAIN_END
};

struct cm_data_t
{
	uint32_t ip;
	uint16_t port;
	uint32_t aid;
	uint32_t pid;
	uint32_t bid;// bgp id ,for different client test
	int active;
	time_t update_time;
	time_t via_time;
	uint32_t counter;
	uint16_t pro_type;
} __attribute__ ((packed));
//bid and ip for key

//cm to server ip list notify
struct cm_to_se_iplist_notify
{
	uint32_t type;
	uint32_t ip;
	uint16_t port;
	uint32_t aid;
	uint32_t pid;
	uint32_t bid;// bgp id ,for different client test
	int      active;
	time_t   update_time;
	time_t   via_time;
	uint32_t counter;
	uint16_t pro_type;

} __attribute__ ((packed));

//client to server ip list req msg
struct client_to_se_iplist_req
{
	uint32_t type;
	uint32_t bid;
	//ip == 0 means all ip
	uint32_t ip;
} __attribute__ ((packed)) ;

//server to client ip list response
struct se_to_client_iplist_res
{
	uint32_t type;
	uint32_t bid;
	uint32_t ip;
	uint16_t port;
} __attribute__ ((packed)) ;

//client test result notify msg
struct client_notify
{
	uint32_t type;
	uint32_t addr;
	uint16_t port;
} __attribute__ ((packed)) ;

struct se_to_client_result_req
{
	uint32_t type;
} __attribute__ ((packed)) ;

struct client_to_se_result_res
{
	uint32_t type;
	uint32_t bid;
	uint32_t ip;
	uint16_t port;
	int32_t delay;//ms,-1 timeout
} __attribute__ ((packed)) ;


struct db_key
{
	uint32_t bid;
	uint32_t ip;
	bool operator==(const db_key & o_key) const
	{
		return  this->bid == o_key.bid && this->ip == o_key.ip ;
	}
	bool operator<(const db_key & key) const
	{
		if(this->bid != key.bid )
		{
			return this->bid < key.bid;
		}
		else if(this->ip != key.ip)
		{
			return this->ip < key.ip;
		}
		else
		{
			return false;
		}
	}
};

struct db_value
{
	uint16_t port;
	uint32_t aid;
	uint32_t pid;
	int      active;
	time_t   update_time;
	time_t   via_time;
	uint32_t counter;
	int      delay;
	int      protocol_type;
};

struct server_task
{
	uint32_t msg_type; // msg type
	uint32_t fd;   // conn fd
} __attribute__ ((packed));

typedef std::map<db_key,db_value> ip_map_t;

typedef std::vector<char> net_buffer ;

struct fd_data
{
	int fd ;
	net_buffer read_buffer;
	//uint32_t cur_read_index;
	net_buffer write_buffer;
	//uint32_t cur_write_index;
	fd_data()
	{
		fd = 0;
		read_buffer.clear();
		write_buffer.clear();
		//cur_read_index = 0;
		//cur_write_index = 0;
	}
	~fd_data()
	{
		fd = 0;
		read_buffer.clear();
		write_buffer.clear();
		//cur_read_index = 0;
		//cur_write_index = 0;
	}
};

#define ___constant_swab64(x) \
 ((uint64_t)( \
  (uint64_t)(((uint64_t)(x) & (uint64_t)0x00000000000000ffULL) << 56) | \
  (uint64_t)(((uint64_t)(x) & (uint64_t)0x000000000000ff00ULL) << 40) | \
  (uint64_t)(((uint64_t)(x) & (uint64_t)0x0000000000ff0000ULL) << 24) | \
  (uint64_t)(((uint64_t)(x) & (uint64_t)0x00000000ff000000ULL) <<  8) | \
  (uint64_t)(((uint64_t)(x) & (uint64_t)0x000000ff00000000ULL) >>  8) | \
  (uint64_t)(((uint64_t)(x) & (uint64_t)0x0000ff0000000000ULL) >> 24) | \
  (uint64_t)(((uint64_t)(x) & (uint64_t)0x00ff000000000000ULL) >> 40) | \
  (uint64_t)(((uint64_t)(x) & (uint64_t)0xff00000000000000ULL) >> 56) ))

uint64_t ntoh64(uint64_t val);

uint64_t htonll(uint64_t val);

//boundary must 2 pow
#define via_align(size, boundary) (((size) + ((boundary) - 1)) & ~((boundary) - 1))
//this buffer can be process only one thread and one point time
//please be carefully
struct io_buffer
{
	char * buffer;
	uint32_t cur_read_pos;
	uint32_t cur_write_pos;
	uint32_t capacity;
	io_buffer()
	{
		buffer = (char *)malloc(IO_BUFFER_SIZE);
		bzero(buffer,IO_BUFFER_SIZE);
		cur_read_pos = 0;
		cur_write_pos = 0;
		capacity = IO_BUFFER_SIZE;
	}
	~io_buffer()
	{
		if(NULL != buffer)
		{
			free(buffer);
			buffer = NULL;
			cur_read_pos = 0;
			cur_write_pos = 0;
			capacity = 0;
		}
	}
	//get current write available bytes number
	int get_write_avail()
	{
		if(cur_write_pos > cur_read_pos)
		{
			return capacity - cur_write_pos + cur_read_pos;
		}
		//means write have write cycle
		else if( cur_write_pos < cur_read_pos)
		{
			return cur_read_pos - cur_write_pos;
		}
		//for cur_write_pos == cur_read_pos
		return 0;
	}
	//append buffer size, and the size with 1024 byte aligned
	int append_size(int size)
	{
		if( size <= (int)capacity)
		{
			return 0;
		}
		int new_size = via_align(size,1024);
		char * temp = (char *)realloc(buffer,new_size);
		if(NULL == temp )
		{
			return -1;
		}
		buffer = temp;
		/*
		 * modify cur_read_pos and cur_write_pos for read and write copy
		 * when cur_write_pos >= cur_read_pos ,this case not need modify pos
		*/
		if(cur_write_pos < cur_read_pos)
		{
			int len = capacity - cur_read_pos + cur_write_pos;
			char read_buffer[len];
			bzero(read_buffer,0);
			int last_read_len = capacity - cur_read_pos;
			memcpy(read_buffer,buffer + last_read_len,last_read_len);
			memcpy(read_buffer+last_read_len,buffer,cur_write_pos);
			//set the new memory
			capacity = new_size;
			bzero(buffer,new_size);
			//copy the read buffer to the new buffer
			memcpy(buffer,read_buffer,len);
			cur_read_pos = 0;
			cur_write_pos = len;
		}
		return 0;
	}
	int write(const void * buf,int len)
	{
		int write_avail = get_write_avail();
		if(write_avail < len)
		{
			if(-1 == append_size(len))
			{
				printf("append size failed\n");
				return -1;
			}
			//append_size modify the capacity and read write pos index
			//so this direct copy the new buffer
			memcpy(buffer,buf,len);
			cur_write_pos += len;
			return 0;
		}
		if(capacity - cur_write_pos < (uint32_t)len)
		{
			//copy the last buffer
			memcpy(this->buffer+cur_write_pos,buf,capacity-cur_write_pos);
			//copy the head buffer
			memcpy(this->buffer,(char *)buf+capacity - cur_write_pos,len - (capacity -cur_write_pos));
			//update cur write pos
			cur_write_pos = len - (capacity - cur_write_pos);
		}
		else
		{
			memcpy(this->buffer+cur_write_pos,buf,len);
			cur_write_pos = cur_write_pos + len - capacity;
		}
	}

	int get_read_avail()
	{
		if(cur_write_pos > cur_read_pos)
		{
			return cur_write_pos - cur_read_pos;
		}
		else if(cur_write_pos < cur_read_pos)
		{
			return capacity - cur_read_pos + cur_write_pos;
		}
		//cur_write_pos == cur_read_pos
		return 0;
	}
	//return number of bytes for for read
	int read(char * read_buffer,int len)
	{
		return 0;
	}


};

#endif /* COMMON_HEADER_H_ */
