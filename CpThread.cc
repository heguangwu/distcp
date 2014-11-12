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
		string cmd = th->buildCommand(d);
		LOG_DEBUG("COMMAND: %s",cmd.c_str());
		int status = -1;

		if( (status = execCmd(cmd) ) == -1) {
			th->putFailCpFileInfo(d);
			LOG_ERROR("execute command failed: %s", cmd.c_str());
		} else {
			if(WIFEXITED(status)) {
				if(WEXITSTATUS(status) == 0) {
					th->putSuccCpFileInfo(d);
				} else {
					th->putFailCpFileInfo(d);
					LOG_ERROR("execute command failed: %s", cmd.c_str());
				}
			} else {
				th->putFailCpFileInfo(d);
				LOG_ERROR("execute command failed: %s", cmd.c_str());
			}
		}
	}
}

CpThread::CpThread(CpFileInfo& fileInfo, const std::string& hostname) : fileInfo_(fileInfo)
	,hostname_(hostname),pthreadId_(0)
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

//ssh dn1 "cp /srcfile /dstfile"
std::string CpThread::buildCommand(dirpair* p)
{
	string command("ssh ");
	command += hostname_;
	command += " \" cp ";
	command += p->srcPath_;
	command += " ";
	command += p->dstPath_;
	command += " \"";
	return command;
}
