/*
 * MysqlConn.h
 *
 *  Created on: 2014-10-30
 *      Author: hgw
 */

#ifndef MYSQLCONN_H_
#define MYSQLCONN_H_
#include <mysql/mysql.h>
#include <string>

class CpFileInfo;
class MysqlConn
{
public:
	MysqlConn();
	~MysqlConn();
	bool connect(const char *dbname = "filemover", const char *user = "root", const char *passwd = "root", const char *host = "localhost", unsigned int port = 3306);

	bool queryMkDirs(CpFileInfo& cpinfo);
	bool queryCpFiles(CpFileInfo& cpinfo);
	bool queryHosts(CpFileInfo& cpinfo);

	bool executeSQL(std::string& sql);

private:
	MYSQL* conn_;

	//static std::string queryMkdirs_;
	//static std::string queryCpFiles_;
	static std::string queryHosts_;

private: //noncopyable
	MysqlConn(MysqlConn const&);
	MysqlConn& operator=(MysqlConn const&);
};

#endif /* MYSQLCONN_H_ */
