/*
 * MysqlConn.cc
 *
 *  Created on: 2014-10-30
 *      Author: hgw
 */
#include "MysqlConn.h"
#include "Logging.h"
#include "ScopeGuard.h"
#include "CpFileInfo.h"
#include "CpConfig.h"
#include <iostream>
#include <sstream>
#include "CpThread.h"
#include <stdlib.h>

using namespace std;

string MysqlConn::queryHosts_ = "SELECT host_name, task_max_num, src_mount_path, dest_mount_path FROM fm_host WHERE status = 1;";

MysqlConn::MysqlConn() : conn_(mysql_init(NULL))
{
}

MysqlConn::~MysqlConn()
{
	mysql_close(conn_);
}

bool MysqlConn::connect(const char *dbname, const char *user, const char *passwd, const char *host, unsigned int port)
{
	if(mysql_real_connect(conn_, host, user, passwd, dbname, 0, NULL, 0) == NULL) {
		LOG_ERROR("connect MySQL fail, user=%s,password=%s,database=%s,host=%s,port=%d",user,passwd,dbname,host,port);
		return false;
	}
	if (mysql_set_character_set(conn_, "utf8") != 0) {
		LOG_ERROR("mysql_set_character_set failed");
		return false;
	}
	return true;
}

bool MysqlConn::queryMkDirs(CpFileInfo& cpinfo)
{
	string queryBasedir = "SELECT local_hdfs_path FROM fm_config WHERE id=1";
	string baseDir;
	{
		if( mysql_query(conn_, queryBasedir.c_str()) != 0) {
			LOG_ERROR("mysql_query fail, query : %s", queryBasedir.c_str());
			return false;
		}
		MYSQL_RES* res = mysql_store_result(conn_); //mysql_use_result
		if(res == NULL) {
			LOG_ERROR("mysql_store_result fail");
			return false;
		}
		ON_SCOPE_EXIT(std::bind(mysql_free_result, res));

		MYSQL_ROW row;
		if( (row = mysql_fetch_row(res)) != NULL) {
			unsigned int num_fields = mysql_num_fields(res);
			if(num_fields != 1) {
				LOG_ERROR("mysql_fetch_row fetch mkdir information error, field number: %d is not equal 2", num_fields);
				return false;
			}
			baseDir = row[0];
		}
	}

	string queryTaskdir = "SELECT dest_path FROM  fm_task_pack WHERE pack_id = ";
	stringstream s1;
	s1<<queryTaskdir<<Configure::taskId;
	string queryTaskdir_ = s1.str();
	string Taskdir;
	{
		if( mysql_query(conn_, queryTaskdir_.c_str()) != 0) {
			LOG_ERROR("mysql_query fail, query : %s", queryTaskdir_.c_str());
			return false;
		}
		MYSQL_RES* res = mysql_store_result(conn_); //mysql_use_result
		if(res == NULL) {
			LOG_ERROR("mysql_store_result fail");
			return false;
		}
		ON_SCOPE_EXIT(std::bind(mysql_free_result, res));

		MYSQL_ROW row;
		while( (row = mysql_fetch_row(res)) != NULL) {
			unsigned int num_fields = mysql_num_fields(res);
			if(num_fields != 1) {
				LOG_ERROR("mysql_fetch_row fetch mkdir information error, field number: %d is not equal 1", num_fields);
				return false;
			}
			Taskdir = row[1];
		}
	}

	string mkdirComm = "mkdir -p ";
	string parentDir =  baseDir + Taskdir;
	CpThread::strConvert(parentDir);
	mkdirComm += parentDir;
	int status = system(mkdirComm.c_str());
	if( status == -1) {
		LOG_ERROR("execute command failed: %s", mkdirComm.c_str());
		return false;
	} else {
		if(WIFEXITED(status)) {
			if(WEXITSTATUS(status) == 0) {
			} else {
				LOG_ERROR("execute command failed: %s", mkdirComm.c_str());
				return false;
			}
		} else {
			LOG_ERROR("execute command failed: %s", mkdirComm.c_str());
			return false;
		}
	}

	string queryMkdirs = "SELECT task_id, dest_path FROM fm_task WHERE file_type = 1 AND pack_id = ";
	stringstream ss;
	ss<<queryMkdirs<<Configure::taskId;
	string queryMkdirs_ = ss.str();
	if( mysql_query(conn_, queryMkdirs_.c_str()) != 0) {
		LOG_ERROR("mysql_query fail, query : %s", queryMkdirs_.c_str());
		return false;
	}
	MYSQL_RES* res = mysql_store_result(conn_); //mysql_use_result
	if(res == NULL) {
		LOG_ERROR("mysql_store_result fail");
		return false;
	}
	ON_SCOPE_EXIT(std::bind(mysql_free_result, res));

	MYSQL_ROW row;
	while( (row = mysql_fetch_row(res)) != NULL) {
		unsigned int num_fields = mysql_num_fields(res);
		if(num_fields != 2) {
			LOG_ERROR("mysql_fetch_row fetch mkdir information error, field number: %d is not equal 2", num_fields);
			return false;
		}
		//unsigned long *lengths = mysql_fetch_lengths(res);
		mkdirinfo* p = new mkdirinfo();
		p->taskId_ = atoi(row[0]);
		p->path_ = baseDir + "/" + row[1];
		cpinfo.addMkDirInfo(p);
	}
	return true;
}

bool MysqlConn::queryCpFiles(CpFileInfo& cpinfo)
{
	int retry_num = 1;
	do {
		string query = "SELECT retry_num FROM fm_config WHERE id=1";
		if( mysql_query(conn_, query.c_str()) != 0) {
			LOG_WARN("mysql_query fail, query : %s", query.c_str());
			break;
		}
		MYSQL_RES* res = mysql_store_result(conn_); //mysql_use_result
		if(res == NULL) {
			LOG_WARN("mysql_store_result get retry_num fail");
			break;
		}
		ON_SCOPE_EXIT(std::bind(mysql_free_result, res));

		MYSQL_ROW row;
		if( (row = mysql_fetch_row(res)) != NULL) {
			unsigned int num_fields = mysql_num_fields(res);
			if(num_fields != 1) {
				LOG_WARN("mysql_fetch_row fetch retry_num information error, field number: %d is not equal 1", num_fields);
				break;
			}
			retry_num = atoi(row[0]);
		}
	}while(false);

	string queryCpFiles_1 = "SELECT task_id, src_path, dest_path, retry_num, status FROM fm_task WHERE file_type = 0 AND retry_num < ";
	string queryCpFiles_2 = "  AND pack_id = ";
	string queryCpFiles_3 =	" AND status IN (0,3) ORDER BY add_time ASC LIMIT 1000";
	stringstream ss;
	ss<<queryCpFiles_1<<retry_num<<queryCpFiles_2<<Configure::taskId<<queryCpFiles_3;
	string queryCpFiles_ = ss.str();
	if( mysql_query(conn_, queryCpFiles_.c_str()) != 0) {
		LOG_ERROR("mysql_query fail, query : %s", queryCpFiles_.c_str());
		return false;
	}
	MYSQL_RES* res = mysql_store_result(conn_); //mysql_use_result
	if(res == NULL) {
		LOG_ERROR("mysql_store_result fail");
		return false;
	}
	ON_SCOPE_EXIT(std::bind(mysql_free_result, res));

	MYSQL_ROW row;
	while( (row = mysql_fetch_row(res)) != NULL) {
		unsigned int num_fields = mysql_num_fields(res);
		if(num_fields != 5) {
			LOG_ERROR("mysql_fetch_row fetch mkdir information error, field number: %d is not equal 2", num_fields);
			return false;
		}
		//unsigned long *lengths = mysql_fetch_lengths(res);
		dirpair* p = new dirpair();
		p->taskId_ = atoi(row[0]);
		p->srcPath_ = row[1];
		p->dstPath_ = row[2];
		p->retryNum_ = atoi(row[3]);
		p->status_ = atoi(row[4]);
		cpinfo.addCpFileInfo(p);
	}
	return true;
}

bool MysqlConn::queryHosts(CpFileInfo& cpinfo)
{
	if( mysql_query(conn_, queryHosts_.c_str()) != 0) {
		LOG_ERROR("mysql_query fail, query : %s", queryHosts_.c_str());
		return false;
	}
	MYSQL_RES* res = mysql_store_result(conn_); //mysql_use_result
	if(res == NULL) {
		LOG_ERROR("mysql_store_result fail");
		return false;
	}
	ON_SCOPE_EXIT(std::bind(mysql_free_result, res));

	MYSQL_ROW  row;
	while( (row = mysql_fetch_row(res)) != NULL) {
		unsigned int num_fields = mysql_num_fields(res);
		if(num_fields != 4) {
			LOG_ERROR("mysql_fetch_row fetch host information record error, field number: %d is not equal 2", num_fields);
			return false;
		}
		//unsigned long *lengths = mysql_fetch_lengths(res);
		hostinfo* p = new hostinfo();
		p->host_ = row[0];
		p->threads_ = atoi(row[1]);
		p->dayu_mount_path = row[2];
		p->hdfs_mount_path = row[3];
		cpinfo.addCpHostInfo(p);
	}
	return true;
}

bool MysqlConn::executeSQL(std::string& sql)
{
	if( mysql_query(conn_, sql.c_str()) != 0) {
		LOG_ERROR("updateStatus failed, sql statement: %s",sql.c_str());
		return false;
	}
	LOG_DEBUG("updateStatus SUCCESS, sql statement: %s",sql.c_str());
	return true;
}

