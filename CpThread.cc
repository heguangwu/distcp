/*
 * CpThread.cc
 *
 *  Created on: 2014-10-23
 *      Author: hgw
 */
#include "CpThread.h"
#include "Logging.h"
#include <iostream>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <memory>

using namespace std;

static int execCmd(string& cmd)
{
	return system(cmd.c_str());
}

static void* runThread(void* arg)
{
	CpThread* th = (CpThread*) arg;
	while(true) {
		dirpair* d = th->getCpFileInfo();
		if( d == 0) return 0;
		std::shared_ptr<dirpair> ptr(d);
		time_t start_time = time(NULL);
		string cmd = th->buildCommand(d);
		LOG_DEBUG("COMMAND: %s",cmd.c_str());
		int status = execCmd(cmd);

		time_t end_time = time(NULL);
		if( status == -1) {
			th->putFailCpFileInfo(d->taskId_, start_time, end_time, d->retryNum_);
			LOG_ERROR("execute command failed: %s", cmd.c_str());
		} else {
			if(WIFEXITED(status)) {
				if(WEXITSTATUS(status) == 0) {
					th->putSuccCpFileInfo(d->taskId_, start_time, end_time, d->retryNum_);
				} else {
					th->putFailCpFileInfo(d->taskId_, start_time, end_time, d->retryNum_);
					LOG_ERROR("execute command failed: %s", cmd.c_str());
				}
			} else {
				th->putFailCpFileInfo(d->taskId_, start_time, end_time, d->retryNum_);
				LOG_ERROR("execute command failed: %s", cmd.c_str());
			}
		}
	}
}

CpThread::CpThread(CpFileInfo& fileInfo, hostinfo* host) : fileInfo_(fileInfo), host_(host), pthreadId_(0)
{}

bool CpThread::start()
{
	if (pthread_create(&pthreadId_, NULL, runThread, (void* )this)) {
		LOG_ERROR("Failed in pthread_create");
		return false;
	}
	return true;
}

int CpThread::join()
{
	return pthread_join(pthreadId_, NULL);
}

void CpThread::strReplace(std::string& str, std::string strFind, std::string strReplace)
{
	string::size_type pos=0;
	string::size_type a=strFind.size();
	string::size_type b=strReplace.size();
	while((pos=str.find(strFind,pos))!=string::npos) {
		str.replace(pos,a,strReplace);
		pos+=b;
	}
}

//ssh dn1 "cp /srcfile /dstfile"
std::string CpThread::buildCommand(dirpair* p)
{
	string command("ssh ");
	command += host_->host_;
	command += " \" cp ";
	command += host_->dayu_mount_path;
	//command += "/";
	//command += p->srcPath_;
	string srcStr = p->srcPath_;
	strReplace(srcStr, " ", "\\ ");
	command += srcStr;

	command += " ";
	command += host_->hdfs_mount_path;
	//command += "/";
	//command += p->dstPath_;
	string dstStr = p->dstPath_;
	strReplace(dstStr, " ", "\\ ");
	command += dstStr;
	command += " \"";
	return command;
}

