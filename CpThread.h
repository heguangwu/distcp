/*
 * CpThread.h
 *
 *  Created on: 2014-10-23
 *      Author: hgw
 */

#ifndef CPTHREAD_H_
#define CPTHREAD_H_

#include "CpFileInfo.h"
#include <pthread.h>
#include <string>


class CpThread
{
public:
	CpThread(CpFileInfo& fileInfo, hostinfo* host);

	bool start();
	int join();

	inline dirpair* getCpFileInfo() {return fileInfo_.popDirPair();}
	inline void putSuccCpFileInfo(int taskId, time_t start, time_t end, short retry) {fileInfo_.updateResult(taskId, true, start, end, host_->host_.c_str(), retry);}
	inline void putFailCpFileInfo(int taskId, time_t start, time_t end, short retry) {fileInfo_.updateResult(taskId, false, start, end, host_->host_.c_str(), retry);}

	std::string buildCommand(dirpair* p);

public:
	static void strReplace(std::string& str, std::string strFind, std::string strReplace);

private:
	CpFileInfo& fileInfo_;
	hostinfo* host_;
	pthread_t   pthreadId_;

private:
	CpThread( const CpThread& );
	CpThread& operator=( const CpThread& );
};

#endif /* CPTHREAD_H_ */
