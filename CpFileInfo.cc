/*
 * CpFileInfo.cc
 *
 *  Created on: 2014-10-23
 *      Author: hgw
 */

#include "CpFileInfo.h"
#include "CpConfig.h"
#include "Logging.h"
#include "ScopeGuard.h"
#include "FileOperator.h"
#include <iostream>
#include <sstream>
#include <unistd.h>
#include <string.h>
#include "CpThread.h"

using namespace std;

CpFileInfo::CpFileInfo(MysqlConn* conn): lock_(), resLock_(), mkDirs_(), cpFiles_(), cpHosts_(), mysqlConn_(conn)
{
}

CpFileInfo::~CpFileInfo()
{
	while(!mkDirs_.empty()){
		mkdirinfo* p = mkDirs_.back();
		mkDirs_.pop_back();
		delete p;
	}
	while(!cpFiles_.empty()){
		dirpair* p = cpFiles_.back();
		cpFiles_.pop_back();
		delete p;
	}
	cpFiles_.clear();
	while(!cpHosts_.empty()) {
		hostinfo* h = cpHosts_.back();
		cpHosts_.pop_back();
		delete h;
	}
	cpHosts_.clear();
}

dirpair* CpFileInfo::popDirPair()
{
	MutexLockGuard g(lock_);
	if(cpFiles_.empty())
		return 0;
	dirpair* p = cpFiles_.front();
	cpFiles_.pop_front();
	return p;
}

void CpFileInfo::updateResult(int taskId, bool success, time_t start, time_t end, const char* host, short retry)
{
	MutexLockGuard g(resLock_);
	stringstream ss;
	int status = (success ? 2 : 3);
	int retryNum = retry + 1;
	ss<<"UPDATE fm_task SET status = "<<status
			<<",retry_num = "<<retryNum
			<<", start_time = "<<start
			<<", end_time = "<<end
			<<", task_ip = "<<" \' "<<host<<" \' "
			<<" WHERE task_id = "<<taskId<<";";
	string sql = ss.str();
	mysqlConn_->executeSQL(sql);
}

bool CpFileInfo::mkAlldirs()
{
	char host[32] = {0};
	if( gethostname(host,sizeof(host)) != 0) {
		memcpy(host, "localhost", strlen("localhost"));
	}
	for(size_t i = 0; i < mkDirs_.size(); i++) {
		time_t start = time(NULL);
		string str = mkDirs_[i]->path_;
		CpThread::strReplace(str, " ", "\\ ");
		if( !FileOperator::dirIsExist(str.c_str()) ) {
			LOG_DEBUG("mkDirR %s",str.c_str());
			if( !FileOperator::mkDirR(str.c_str())) {
				time_t end = time(NULL);
				LOG_ERROR("mkDirR failed, pathname: %s", str.c_str());
				updateResult(mkDirs_[i]->taskId_, false, start, end, host);
				return false;
			}
		}
		time_t end = time(NULL);
		updateResult(mkDirs_[i]->taskId_, true, start, end, host);
	}
	return true;
}
