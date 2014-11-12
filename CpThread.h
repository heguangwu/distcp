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
	CpThread(CpFileInfo& fileInfo, const std::string& hostname);

	bool start();
	int join();

	inline dirpair* getCpFileInfo() {return fileInfo_.popDirPair();}
	inline void putSuccCpFileInfo(dirpair* p) {fileInfo_.pushSuccessPair(p);}
	inline void putFailCpFileInfo(dirpair* p) {fileInfo_.pushFailedPair(p);}

	std::string buildCommand(dirpair* p);

private:
	CpFileInfo& fileInfo_;
	std::string hostname_;
	pthread_t   pthreadId_;

private:
	CpThread( const CpThread& );
	CpThread& operator=( const CpThread& );
};

#endif /* CPTHREAD_H_ */
