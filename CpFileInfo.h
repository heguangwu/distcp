/*
 * CpFileInfo.h
 *
 *  Created on: 2014-10-23
 *      Author: hgw
 */

#ifndef CPFILEINFO_H_
#define CPFILEINFO_H_

#include "MutexLock.h"
#include "MysqlConn.h"
#include "TaskInfo.h"
#include <string>
#include <deque>
#include <vector>
#include <stdint.h>


class CpFileInfo
{
public:
	explicit CpFileInfo(MysqlConn* conn);
	~CpFileInfo();

	inline const std::vector< hostinfo* >& getHostsInfo() const {return cpHosts_;}
	inline const std::vector< mkdirinfo* >& getMkdirsInfo() const {return mkDirs_;}

	inline void clearHostInfo() {
		while(!cpHosts_.empty()) {
			hostinfo* h = cpHosts_.back();
			cpHosts_.pop_back();
			delete h;
		}
		cpHosts_.clear();
	}

	dirpair* popDirPair();
	void updateResult(int taskId, bool success, time_t start, time_t end, const char* host, short retry = 0);
	void updateProcess(int taskId);

	inline void addMkDirInfo(mkdirinfo* d) {mkDirs_.push_back(d);}
	inline void addCpFileInfo(dirpair* d) {cpFiles_.push_back(d);}
	inline void addCpHostInfo(hostinfo* d) {cpHosts_.push_back(d);}

	bool mkAlldirs();
	inline bool isComplete() {return cpFiles_.empty();}

private:
	MutexLock lock_;
	MutexLock resLock_;

	std::vector< mkdirinfo* > mkDirs_;
	std::deque< dirpair* > cpFiles_;
	std::vector< hostinfo* > cpHosts_;
	MysqlConn* mysqlConn_;

private: //noncopyable
	CpFileInfo( const CpFileInfo& );
	CpFileInfo& operator=( const CpFileInfo& );
};

#endif /* CPFILEINFO_H_ */
