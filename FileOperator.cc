/*
 * FileOperator.cc
 *
 *  Created on: 2014-10-23
 *      Author: hgw
 */
#include "FileOperator.h"
#include "Logging.h"
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <dirent.h>
#include <iostream>
#include <sys/stat.h>
#include <string.h>

using namespace std;

bool FileOperator::dirIsExist(const char* dirname)
{
	DIR *d = NULL;
	if( (d = opendir(dirname)) == NULL) {
		if (errno == ENOENT)
			return false;
	}
	closedir(d);
	return true;
}

bool FileOperator::fileIsExist(const char* filename)
{
	if ( filename == NULL )
		return false;
	if( access(filename, F_OK) == 0) {
		return true;
	}
	return false;
}

bool FileOperator::mkDirR(const char* dirname)
{
	int len = strlen(dirname);
	char* path = new char[len+1];
	memcpy(path, dirname, len);
	path[len] = '\0';
	for(int i = 1;i < len;i++) {
		if(path[i] == '/') {
			path[i] = '\0';
			if(access(path, NULL) != 0) {
				LOG_DEBUG("Create path : %s",path);
				if(mkdir(path, 0755) == -1) {
					int e = errno;
					LOG_ERROR("Create path %s failed, %s", path, strerror(e));
					return false;
				}
			}
			path[i] = '/';
		}
	}
	if(access(path, NULL) != 0) {
		LOG_DEBUG("create path : %s",path);
		if(mkdir(path, 0755) == -1) {
			int e = errno;
			LOG_ERROR("Create path %s failed, %s", path, strerror(e));
			return false;
		}
	}
	return true;
}

