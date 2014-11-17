/*
 * main.cc
 *
 *  Created on: 2014-10-23
 *      Author: hgw
 */
#include "CpFileInfo.h"
#include "CpConfig.h"
#include "CpThread.h"
#include "FileOperator.h"
#include "Logging.h"
#include "MysqlConn.h"
#include <iostream>
#include <sys/klog.h>
#include <vector>
#include <unistd.h>
#include <sys/types.h>
#include <pwd.h>
#include <getopt.h>
#include <sstream>
#include <memory>

using namespace std;

//#define DEBUG

void changeugid(void) {
	uid_t wrk_uid;
	gid_t wrk_gid;

	if (geteuid()==0) {
		char pwdgrpbuff[16384];
		struct passwd pwd,*pw;

		cout<<"change to user "<<Configure::dstUser<<endl;

		getpwnam_r(Configure::dstUser.c_str(), &pwd, pwdgrpbuff, 16384, &pw);
		if (pw == NULL) {
			cout<<"ERROR: "<< Configure::dstUser.c_str()<<" no such user - "<<endl;
			exit(1);
		}
		wrk_uid = pw->pw_uid;
		wrk_gid = pw->pw_gid;

		if (setgid(wrk_gid) < 0) {
			cout<<"ERROR: can't set gid to "<< (int)wrk_gid<<endl;
			exit(1);
		} else {
			cout<<"INFO: set gid to"<<(int)wrk_gid<<endl;
		}
		if (setuid(wrk_uid)<0) {
			cout<<"ERROR: can't set uid to "<<(int)wrk_uid;
			exit(1);
		} else {
			cout<<"INFO: set uid to "<<(int)wrk_uid<<endl;
		}
	}
}

void Usage(const char* appname)
{
	cout<<"Usage: "<<appname<<" -l [DEBUG|INFO|WARN|ERROR] -d database-name -t taskid"<<endl;
	cout<<"Usage: "<<appname<<" --loglevel [DEBUG|INFO|WARN|ERROR] --database database-name  --taskid taskid"<<endl;
	cout<<"For example: "<<appname<<" -l DEBUG -d filemover -t 2"<<endl;
}

int main(int argc, char* argv[])
{
	changeugid();
	string level;
	string database = "filemover";
	const char* short_options = "l:d:t:";
	struct option long_options[] = {
		 { "loglevel",  1,   NULL,    'l'     },
		 { "database",  1,   NULL,    'd'     },
		 { "taskid",    1,   NULL,    't'     },
		 {      0,     0,     0,     0},
	};
	int c;
	Configure::taskId = -1;
	while((c = getopt_long (argc, argv, short_options, long_options, NULL)) != -1) {
		switch (c) {
		case 'd':
			database = optarg;
			break;
		case 'l':
			level = optarg;
			break;
		case 't':
			Configure::taskId  = atoi(optarg);
			break;
		case '?':
			cout<<"Error arguments\n";
			Usage(argv[0]);
			exit(0);
		}
	}
	LogLevel l = LL_WARNING;
	if(level == "DEBUG") {
		l = LL_DEBUG;
	} else if(level == "WARN") {
		l = LL_WARNING;
	} else if(level == "ERROR") {
		l = LL_ERROR;
	} else if(level == "INFO") {
		l = LL_INFO;
	}
	LOG_INIT(l, "DistCp", Configure::LogDir.c_str());
	if(Configure::taskId <= 0) {
		LOG_WARN("Task Id = %d is invalid, exiting", Configure::taskId);
		return -1;
	}

	uid_t wrk_uid = geteuid();
	LOG_WARN("Running user id = %d, task id = %d", wrk_uid, Configure::taskId);

	//std::shared_ptr<MysqlConn> conn(new MysqlConn);
	MysqlConn* conn = new MysqlConn();
	std::shared_ptr<MysqlConn> mysqlPtr(conn);
	if( !conn->connect(database.c_str()) ){
		LOG_ERROR("Connect MySQL failed");
		return -1;
	}

	CpFileInfo cfinfo(conn);

	if( !conn->queryMkDirs(cfinfo) ){
		LOG_ERROR("Get mkdir information from MySQL failed");
		return -3;
	}
#ifdef DEBUG
	const vector< mkdirinfo* >& dirs = cfinfo.getMkdirsInfo();
	for(size_t i = 0; i < dirs.size(); i++) {
		cout<<"taskid="<<dirs[i]->taskId_<<", path="<<dirs[i]->path_<<endl;
	}
#endif
	LOG_INFO("Get mkdir information SUCCESS");
	if( !cfinfo.mkAlldirs() ) {
		LOG_ERROR("Create directory failed");
		return -4;
	}
	LOG_INFO("Create directory SUCCESS");

	while(true) {
		if( !conn->queryHosts(cfinfo) ){
			LOG_ERROR("Get host information from MySQL failed");
			return -2;
		}
		LOG_INFO("Get host list information SUCCESS");
		const vector< hostinfo* >& hosts = cfinfo.getHostsInfo();
		if( hosts.empty()) {
			LOG_ERROR("No host");
			exit(0);
		}
#ifdef DEBUG
		for(size_t i = 0; i < hosts.size(); i++) {
			cout<<"hostname="<<hosts[i]->host_<<", threads="<<hosts[i]->threads_<<endl;
		}
#endif

		//get copy files from MySql
		if( !conn->queryCpFiles(cfinfo) ) {
			LOG_ERROR("Get copy files information from MySQL failed");
			return -5;
		}

		if(cfinfo.isComplete()) {
			LOG_INFO("All tasks are complete!");
			break;
		}

		vector< shared_ptr<CpThread> > threads;
		for(size_t i = 0; i < hosts.size(); i++) {
			for(int pt = 0; pt < hosts[i]->threads_; pt++) {
				shared_ptr<CpThread> th(new CpThread(cfinfo, hosts[i]) );
				threads.push_back(th);
			}
		}
		LOG_INFO("start %d threads ... ", threads.size());
		for(size_t i = 0; i < threads.size(); i++) {
			if( !threads[i]->start() ) {
				LOG_ERROR("Start threads failed");
				exit(1);
			}
		}

		for(size_t i = 0; i < threads.size(); i++) {
			threads[i]->join();
		}
		threads.clear();
		cfinfo.clearHostInfo();
	}

	time_t end = time(NULL);
	string updateStatus("UPDATE `fm_task_pack` SET `status`=3, `end_time`=");
	stringstream ss;
	ss<<updateStatus<<end<<" WHERE pack_id="<<Configure::taskId;
	string sql = ss.str();
	if(!conn->executeSQL(sql)) {
		LOG_ERROR("execute sql: %s failed", updateStatus.c_str());
	}

	return 0;
}
