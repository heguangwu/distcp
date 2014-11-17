/*
 * TaskInfo.h
 *
 *  Created on: 2014-10-30
 *      Author: hgw
 */

#ifndef TASKINFO_H_
#define TASKINFO_H_
#include <string>
using namespace std;

struct hostinfo
{
	std::string host_;
	int threads_;
	std::string dayu_mount_path;
	std::string hdfs_mount_path;
};

struct mkdirinfo
{
	unsigned int taskId_;
	std::string path_;
};

struct dirpair
{
	unsigned int taskId_;
	std::string srcPath_;
	std::string dstPath_;
	short retryNum_;
	short status_;
};

#endif /* TASKINFO_H_ */
