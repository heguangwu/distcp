/*
 * CpFileInfo.h
 *
 *  Created on: 2014-10-23
 *      Author: hgw
 */

#ifndef CPFILEINFO_H_
#define CPFILEINFO_H_

#include <string>
#include <vector>
#include "CpConfig.h"
#include "MutexLock.h"
#include <stdint.h>

struct dirpair
{
	//uint64_t srcFileSize_;
	std::string srcPath_;
	std::string dstPath_;
};

class CpFileInfo
{
public:
	explicit CpFileInfo();
	~CpFileInfo();

	void setSrcFile(const char* srcDir) {srcDir_ = Configure::DayuPrefix + srcDir;}
	bool loadSrcFiles(const char* loadFile);

	bool readCpFileList();
	const std::vector< std::string >& getMkDirs() {
		return mkDirs_;
	}

	dirpair* popDirPair();
	void pushSuccessPair(dirpair* p);
	void pushFailedPair(dirpair* p);

	void printResult();

	bool mkAlldirs();

private:
	bool readFileList(const char* basePath);

private:
	std::string srcDir_;
	MutexLock lock_;
	MutexLock slock_;
	MutexLock flock_;

	std::vector< std::string > mkDirs_;
	std::vector< dirpair* > cpFiles_;
	FILE* cpAll_;
	FILE* cpSucc_;
	FILE* cpFail_;

	size_t allNum_;
	size_t succNum_;
	//uint64_t cpFileSizeSum_;
	//uint64_t cpFinishedSize_;
	std::vector< dirpair* > cpSuccFiles_;
	std::vector< dirpair* > cpFailFiles_;


//noncopyable
private:
	CpFileInfo( const CpFileInfo& );
	CpFileInfo& operator=( const CpFileInfo& );
};

#endif /* CPFILEINFO_H_ */
