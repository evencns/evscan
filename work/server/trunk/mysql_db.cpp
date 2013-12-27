#include "mysql_db.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <arpa/inet.h>

CServerDB * CServerDB::p_instance = NULL;
const int buffer_len =1024;
extern const char * db_user;
extern const char * db_pass;

CServerDB::CServerDB()
{
	init();
}

CServerDB::~CServerDB()
{
	mysql_close(&conn);
	//please be carefull
	this->p_instance = NULL;
}

int CServerDB::init()
{
	mysql_init(&this->conn);
	this->str_user_name = std::string(db_user);
	this->str_pass = std::string(db_pass);
	this->str_db = "quality_test";
	this->port = 3306;
	this->str_host = "127.0.0.1";
	this->p_instance = this;
	return 0;
}

//strick need mutex ,but our code first init db
CServerDB *  CServerDB::get_instance()
{
	if(p_instance != NULL)
	{
		return p_instance;
	}
	p_instance = new CServerDB;
	return p_instance;
}

int CServerDB::connect_db()
{
	MYSQL * temp = NULL;
	temp = mysql_real_connect(&this->conn,this->str_host.c_str(),this->str_user_name.c_str(),
							this->str_pass.c_str(),this->str_db.c_str(),
							this->port,NULL,0);
	if(NULL != temp)
	{
		this->conn = *temp;
		return 0;
	}
	printf("mysql conn error %s \n",mysql_error(&this->conn));
	return -1;
}

/*
MariaDB [(none)]> desc quality_test.cur_data_info;
+-------------+----------------------+------+-----+---------+-------+
| Field       | Type                 | Null | Key | Default | Extra |
+-------------+----------------------+------+-----+---------+-------+
| bid         | int(11) unsigned     | NO   | PRI | NULL    |       |
| ip          | varchar(255)         | NO   | PRI | NULL    |       |
| port        | smallint(5) unsigned | NO   |     | NULL    |       |
| aid         | int(11) unsigned     | NO   |     | NULL    |       |
| pid         | int(11) unsigned     | NO   |     | NULL    |       |
| active      | int(11)              | NO   |     | 0       |       |
| update_time | datetime             | NO   |     | NULL    |       |
| via_time    | datetime             | NO   |     | NULL    |       |
| counter     | int(11) unsigned     | NO   |     | 0       |       |
| delay       | int(11)              | NO   |     | -1      |       |
| protocol_type| int(11)             | NO   |     | 0       |       |
+-------------+----------------------+------+-----+---------+-------+
*/

int CServerDB::get_all_cur_data_info(ip_map_t & o_ip_map)
{
	char buffer[buffer_len];
	bzero(buffer,buffer_len);
	snprintf(buffer,buffer_len,"select bid,ip,port,aid,pid,"
			"active,unix_timestamp(update_time),unix_timestamp(via_time),"
			"counter,delay,protocol_type from quality_test.cur_data_info");
	mysql_res * result = NULL;
	MYSQL_ROW row;
	db_key key;
	db_value value;
	result = do_query(buffer,strlen(buffer));
	if(NULL != result)
	{
		int rows = mysql_num_rows(result);
		for(int i = 0; i < rows; i++)
		{
			row = mysql_fetch_row(result);
			key.bid = (uint32_t)strtoul(row[0],NULL,10);
			//may be need regex check the ip string is OK
			//here just convert it
			key.ip  = ntohl(inet_addr(row[1]));
			value.port = (uint16_t)atoi(row[2]);
			value.aid = (uint32_t)strtoul(row[3],NULL,10);
			value.pid = (uint32_t)strtoul(row[4],NULL,10);
			value.active = atoi(row[5]);
			if(NULL != row[6])
			{
				value.update_time = strtoul(row[6],NULL,10);
			}
			if(NULL != row[7])
			{
				value.via_time = strtoul(row[7],NULL,10) ;
			}
			value.counter = (uint32_t)strtoul(row[8],NULL,10);
			value.delay = atoi(row[9]);
			value.protocol_type = atoi(row[10]);
			//into row data to the map
			o_ip_map[key] = value;
		}
	}

	return 0;
}

/*
 * below example
 insert into quality_test.cur_data_info values
	 (1,'1.1.1.1',80,2,2,0,'1990-10-10 10:10:10','1990-10-10 10:10:10',10,-1)
	ON DUPLICATE KEY UPDATE  port = 90

 * */

/*
MariaDB [(none)]> desc quality_test.cur_data_info;
+-------------+----------------------+------+-----+---------+-------+
| Field       | Type                 | Null | Key | Default | Extra |
+-------------+----------------------+------+-----+---------+-------+
| bid         | int(11) unsigned     | NO   | PRI | NULL    |       |
| ip          | varchar(255)         | NO   | PRI | NULL    |       |
| port        | smallint(5) unsigned | NO   |     | NULL    |       |
| aid         | int(11) unsigned     | NO   |     | NULL    |       |
| pid         | int(11) unsigned     | NO   |     | NULL    |       |
| active      | int(11)              | NO   |     | 0       |       |
| update_time | datetime             | NO   |     | NULL    |       |
| via_time    | datetime             | NO   |     | NULL    |       |
| counter     | int(11) unsigned     | NO   |     | 0       |       |
| delay       | int(11)              | NO   |     | -1      |       |
| protocol_type| int(11)             | NO   |     | 0       |       |
+-------------+----------------------+------+-----+---------+-------+
*/

int CServerDB::insert_cur_data_info(const db_key & key,const db_value & value)
{
	char buffer[buffer_len];
	bzero(buffer,buffer_len);
	in_addr addr;
	addr.s_addr = key.ip;
	char ip_buffer[16];
	bzero(ip_buffer,16);
	inet_neta(key.ip,ip_buffer,16);
	snprintf(buffer,buffer_len,
			" insert into quality_test.cur_data_info values"
			" (%u,'%s',%u,%u,%u,%d,from_unixtime(%ld),from_unixtime(%ld),%u,%d,%d) "
			" ON DUPLICATE KEY UPDATE "
			" port=%u,"
			" aid=%u,"
			" pid=%u,"
			" active=%d,"
			" update_time=from_unixtime(%ld),"
			" via_time=from_unixtime(%ld),"
			" counter=%u,"
			" delay = %d,"
			" protocol_type = %d",
			key.bid,
			ip_buffer,
			value.port,
			value.aid,
			value.pid,
			value.active,
			value.update_time,
			value.via_time,
			value.counter,
			value.delay,
			value.protocol_type,
			value.port,value.aid+1,value.pid,value.active,
			value.update_time,value.via_time,value.counter,
			value.delay,value.protocol_type);

	int ret = mysql_real_query(&conn,buffer,strlen(buffer));
	if( 0 != ret )
	{
		printf("insert cur data failed %s\n",mysql_error(&conn));
		return -1;
	}
	return 0;
}

/*
MariaDB [(none)]> desc quality_test.history_data_info;
+-------------+----------------------+------+-----+---------+-------+
| Field       | Type                 | Null | Key | Default | Extra |
+-------------+----------------------+------+-----+---------+-------+
| time_stamp  | datetime             | NO   | PRI | NULL    |       |
| bid         | int(11) unsigned     | NO   | PRI | NULL    |       |
| ip          | varchar(255)         | NO   | PRI | NULL    |       |
| port        | smallint(5) unsigned | NO   | PRI | NULL    |       |
| aid         | int(11) unsigned     | NO   |     | NULL    |       |
| pid         | int(11) unsigned     | NO   |     | NULL    |       |
| active      | int(11)              | NO   |     | 0       |       |
| update_time | datetime             | NO   |     | NULL    |       |
| via_time    | datetime             | NO   |     | NULL    |       |
| counter     | int(11) unsigned     | NO   |     | 0       |       |
| delay       | int(11)              | NO   |     | -1      |       |
+-------------+----------------------+------+-----+---------+-------+
*/

int CServerDB::insert_history_data_info(time_t i_timestamp,const db_key & key,const db_value & value)
{
	char buffer[buffer_len];
	bzero(buffer,buffer_len);
	in_addr addr;
	addr.s_addr = key.ip;
	char ip_buffer[16];
	bzero(ip_buffer,16);
	inet_neta(key.ip,ip_buffer,16);
	snprintf(buffer,buffer_len,
			" insert into quality_test.history_data_info values"
			" (from_unixtime(%ld),%u,'%s',%u,%u,%u,%d,from_unixtime(%ld),from_unixtime(%ld),%u,%d,%d) ",
			i_timestamp,
			key.bid,
			ip_buffer,
			value.port,
			value.aid,
			value.pid,
			value.active,
			value.update_time,
			value.via_time,
			value.counter,
			value.delay,
			value.protocol_type);

	int ret = mysql_real_query(&conn,buffer,strlen(buffer));
	if( 0 != ret )
	{
		printf("insert history data failed %s\n",mysql_error(&conn));
		return -1;
	}
	return 0;
}

mysql_res * CServerDB::do_query(const char * buffer,unsigned int length)
{
	mysql_res * temp = NULL;
	int ret = mysql_real_query(&this->conn,buffer,length);
	if( 0 != ret)
	{
		printf("mysql real query failed %s\n",mysql_error(&this->conn));
		return NULL;
	}
	temp = mysql_store_result(&this->conn);
	if(NULL != temp )
	{
		return temp;
	}
	printf("mysql store result failed %s\n",mysql_error(&this->conn));
	return NULL;
}

