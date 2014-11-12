/*
 * Logging.cc
 *
 *  Created on: 2014-10-24
 *      Author: hgw
 */
#include "Logging.h"
#include <sys/file.h>
#include <stdarg.h>
#include <time.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/stat.h>
#include <iostream>
#include "CpConfig.h"

using namespace std;

Logging WARN_W;
Logging INFO_W;

__thread char Logging::buffer[LOG_BUFFSIZE];


bool LOG_INIT(LogLevel l, const char* moduleName, const char* logDir)
{
	if (access (logDir, 0) == -1) {
		if (mkdir (logDir, S_IREAD | S_IWRITE ) < 0) {
			cout<<"ERROR: create log folder failed\n";
			exit(1);
		}
	}
	string localtion_info = logDir;
	localtion_info += "/";
	localtion_info += moduleName;
	localtion_info += "-";
	localtion_info += Configure::startTime;
	localtion_info += ".access";
	INFO_W.logInit(l, localtion_info);

	string localtion_err = logDir;
	localtion_err += "/";
	localtion_err += moduleName;
	localtion_err += "-";
	localtion_err += Configure::startTime;
	localtion_err += ".error";
	if(l > LL_WARNING)
		WARN_W.logInit(l, localtion_err);
	else
		WARN_W.logInit(LL_WARNING, localtion_err);
	return true;
}

const char* Logging::logLevelToString(LogLevel l) {
	switch ( l ) {
		case LL_DEBUG:
			return "DEBUG";
		case LL_TRACE:
			return "TRACE";
		case LL_INFO:
			return "INFO";
		case LL_WARNING:
			return "WARN" ;
		case LL_ERROR:
			return "ERROR";
		default:
			return "UNKNOWN";
	}
}

bool Logging::checkLevel(LogLevel l)
{
	bool ret = (l >= level_  ? true : false);
	return ret;
}

bool Logging::logInit(LogLevel l, string& filename, bool append, bool issync)
{
	MACRO_RET(NULL != fp_, false);
    level_ = l;
    isAppend_ = append;
    isSync_ = issync;
	if( filename.length() >= Logging::LOG_PATH_LEN ) {
		cout<<"ERROR: the path of log file length is too long "<<LOG_PATH_LEN<<endl;
		exit(0);
	}

	logFile_ = filename;

	fp_ = fopen(logFile_.c_str(), append ? "a":"w");
	if(fp_ == NULL) {
		cout<<"ERROR: cannot open log file,file location is "<< logFile_<<endl;
		exit(0);
	}

	setvbuf (fp_,  (char *)NULL, _IOLBF, 0);
	cout<<"INFO: now all the running-information are going to the file "<< logFile_<<endl;
	return true;
}

int Logging::premakestr(char* m_buffer, LogLevel l)
{
    time_t now;
	now = time(&now);;
	struct tm vtm;
    localtime_r(&now, &vtm);
    return snprintf(m_buffer, LOG_BUFFSIZE, "%s: %02d-%02d %02d:%02d:%02d ", logLevelToString(l),
            vtm.tm_mon + 1, vtm.tm_mday, vtm.tm_hour, vtm.tm_min, vtm.tm_sec);
}

bool Logging::log(LogLevel l, const char* logformat,...)
{
	MACRO_RET(!checkLevel(l), false);

	char* star = buffer;
	int prestrlen = premakestr(star, l);
	star += prestrlen;

	va_list args;
	va_start(args, logformat);
	int size = vsnprintf(star, LOG_BUFFSIZE - prestrlen, logformat, args);
	va_end(args);

	if(NULL == fp_)
		fprintf(stderr, "%s", buffer);
	else
		writeLog(buffer, prestrlen + size);
	return true;
}

bool Logging::writeLog(char *buf, int len)
{
	if(0 != access(logFile_.c_str(), W_OK)) {
		MutexLockGuard lock(mutex_);
		if(0 != access(logFile_.c_str(), W_OK)) {
			closeLogFile();
			logInit(level_, logFile_, isAppend_, isSync_);
		}
	}

	if(1 == fwrite(buf, len, 1, fp_)) { //only write 1 item
		if(isSync_)
          	fflush(fp_);
		*buf='\0';
    } else {
        int x = errno;
	    cout<<"ERROR: Failed to write to logfile. errno:"<<strerror(x)<<endl;
	    return false;
	}
	return true;
}

bool Logging::closeLogFile()
{
	if(fp_ == NULL)
		return false;
	fflush(fp_);
	fclose(fp_);
	fp_ = NULL;
	return true;
}

