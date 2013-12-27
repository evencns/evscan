#ifndef __DB_H__
#define __DB_H__

#include <mysql/mysql.h>
#include <string>
#include "common_header.h"

typedef MYSQL_RES mysql_res;

class CServerDB
{
public:
	static CServerDB *  get_instance();
	~CServerDB();

	int connect_db();
	//for quick,must connect the db
	int get_all_cur_data_info(ip_map_t & o_ip_map);
	int insert_cur_data_info(const db_key & key,const db_value & value);
	int insert_history_data_info(time_t i_timestamp,const db_key & key,const db_value & value);
	mysql_res * do_query(const char * buffer,unsigned int length);

private:
public:
	CServerDB();
	int init();
	MYSQL conn;
	std::string str_user_name;
	std::string str_pass;
	std::string str_db;
	std::string str_host;
	int port;
	static CServerDB * p_instance ;
};


#endif
