/*
 * Logging.h
 *
 *  Created on: 2014-10-23
 *      Author: hgw
 */

#ifndef LOGGING_H_
#define LOGGING_H_

#include "MutexLock.h"
#include <stdio.h>
#include <pthread.h>
#include <string>
#include "macro_define.h"

typedef enum LogLevel {
	LL_DEBUG = 1,
	LL_TRACE = 2,
	LL_INFO = 3,
	LL_WARNING = 4,
	LL_ERROR = 5,
}LogLevel;


class Logging
{
public:
	Logging() : level_(LL_INFO), fp_ (NULL), isSync_(false), isAppend_(true), logFile_(), mutex_()
	{
	}
	~Logging()
	{
		closeLogFile();
	}

	bool logInit(LogLevel l, std::string& filename, bool append = true, bool issync = false);
	bool log(LogLevel l,const char *logformat,...);

	LogLevel getLevel() const {return level_;}
	bool closeLogFile();

private:
	inline const char* logLevelToString(LogLevel l);
	inline bool checkLevel(LogLevel l);
	int premakestr(char* m_buffer, LogLevel l);
	bool writeLog(char *_pbuffer, int len);

private:
	enum LogLevel level_;
	FILE* fp_;
	bool isSync_;
	bool isAppend_;
	std::string logFile_;
	MutexLock mutex_;

	static const int LOG_BUFFSIZE  = 1024*1024*4;
	static const int SYS_BUFFSIZE = 1024*1024*8;
	static const size_t LOG_PATH_LEN = 250;
	static const int LOG_MODULE_NAME_LEN = 32;

	static __thread char buffer[LOG_BUFFSIZE];


//noncopyable
private:
	Logging( const Logging& );
	Logging& operator=( const Logging& );
};

extern Logging WARN_W;
extern Logging INFO_W;


bool LOG_INIT(LogLevel l, const char* moduleName, const char* logDir);

#endif /* LOGGING_H_ */
