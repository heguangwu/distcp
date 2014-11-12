/*
 * CpFileInfo.cc
 *
 *  Created on: 2014-10-23
 *      Author: hgw
 */

#include "CpFileInfo.h"
#include <dirent.h>
#include <unistd.h>
#include "CpConfig.h"
#include "Logging.h"
#include "ScopeGuard.h"
#include <string.h>
#include "FileOperator.h"
#include <iostream>
#include <fstream>
#include <stdio.h>
#include <time.h>
#include <sstream>
#include <sys/types.h>
#include <sys/stat.h>
#include "CpConfig.h"

using namespace std;

CpFileInfo::CpFileInfo(): lock_(), slock_(), flock_(), mkDirs_(), cpFiles_(),allNum_(0), succNum_(0)  //, cpFileSizeSum_(0), cpFinishedSize_(0)
{
	time_t t = time(NULL);
	struct tm* ft = localtime(&t);
	memset(Configure::startTime, 0, 200);
	if (strftime(Configure::startTime, sizeof(Configure::startTime), "%Y-%m-%d-%H-%M-%S", ft) == 0) {
		LOG_WARN("strftime convert failed");
		stringstream st;
		st<<t;
		memcpy(Configure::startTime, st.str().c_str(), st.str().length());
	}
	stringstream all;
	all<<Configure::LogDir<<"/"<<"ALL_"<<Configure::startTime<<".txt";
	stringstream succ;
	succ<<Configure::LogDir<<"/"<<"SUCCESS_"<<Configure::startTime<<".txt";
	stringstream fail;
	fail<<Configure::LogDir<<"/"<<"FAIL_"<<Configure::startTime<<".txt";
	string allFile = all.str() ;
	string succFile = succ.str() ;
	string failFile = fail.str() ;

	cpAll_ = fopen(allFile.c_str(), "w+");
	if(cpAll_ == NULL) {
		LOG_ERROR("cannot open task record file,file location is %s",allFile.c_str());
		exit(3);
	}
	cpSucc_ = fopen(succFile.c_str(), "w+");
	if(cpSucc_ == NULL) {
		LOG_ERROR("cannot open success record file,file location is %s",succFile.c_str());
		exit(3);
	}
	cpFail_ = fopen(failFile.c_str(), "w+");
	if(cpFail_ == NULL) {
		LOG_ERROR("cannot open failed record file,file location is %s",failFile.c_str());
		exit(3);
	}
}

CpFileInfo::~CpFileInfo()
{
	mkDirs_.clear();
	while(!cpFiles_.empty()){
		dirpair* p = cpFiles_.back();
		cpFiles_.pop_back();
		delete p;
	}
	cpFiles_.clear();
	if(cpAll_ != NULL) {
		fclose(cpAll_);
		cpAll_ = NULL;
	}
	if(cpSucc_ != NULL) {
		fclose(cpSucc_);
		cpSucc_ = NULL;
	}
	if(cpFail_ != NULL) {
		fclose(cpFail_);
		cpFail_ = NULL;
	}
}

bool CpFileInfo::readCpFileList()
{
	bool ret = readFileList(srcDir_.c_str());
	allNum_ = cpFiles_.size();
	for(size_t i = 0; i < cpFiles_.size(); i++) {
		string s = cpFiles_[i]->srcPath_ + "\n";
		if(1 != fwrite((void *)s.c_str(), s.length(), 1, cpAll_)) {
			LOG_ERROR("write failed for all task filename:%s",s.c_str());
		}
	}
	fflush(cpAll_);
	fclose(cpAll_);
	cpAll_ = NULL;
	return ret;
}

bool CpFileInfo::readFileList(const char* basePath)
{
	DIR *dir;
	struct dirent *ptr;
	char base[1000];

	if ((dir = opendir(basePath)) == NULL)
	{
		int e = errno;
		LOG_ERROR("opendir failed, %s",strerror(e));
		exit(1);
	}

	ON_SCOPE_EXIT(std::bind(closedir, dir));

	while ((ptr = readdir(dir)) != NULL)
	{
		if(strcmp(ptr->d_name,".")==0 || strcmp(ptr->d_name,"..")==0)    ///current dir OR parrent dir
			continue;
		else if(ptr->d_type == 8)  {    ///file
			LOG_DEBUG("file name: %s/%s",basePath,ptr->d_name);
			dirpair* cf = new dirpair();
			cf->srcPath_ = basePath;
			cf->srcPath_ += "/";
			cf->srcPath_ += ptr->d_name;

			/*struct stat fileStatu;
			if(stat(cf->srcPath_.c_str(), &fileStatu) != 0) {
				LOG_WARN("cann't get file stat, filename: %s",cf->srcPath_.c_str());
				exit(0);
			}
			cf->srcFileSize_ = fileStatu.st_size;
			cpFileSizeSum_ += fileStatu.st_size;*/

			cf->dstPath_ = cf->srcPath_;
			std::string::size_type   pos(0);
			if( (pos=cf->dstPath_.find(Configure::DayuPrefix)) != std::string::npos ) {
				cf->dstPath_.replace(pos, Configure::DayuPrefix.length(), Configure::HdfsPrefix);
				cpFiles_.push_back(cf);
			} else {
				LOG_WARN("cann't find prefix %s, %s be ignore and not copy",Configure::DayuPrefix.c_str(), cf->srcPath_.c_str());
				delete cf;
			}
		}
		else if(ptr->d_type == 10)    ///link file
			LOG_WARN("LINK file, filename:%s/%s",basePath,ptr->d_name);
		else if(ptr->d_type == 4)    ///dir
		{
			memset(base,'\0',sizeof(base));
			strcpy(base,basePath);
			strcat(base,"/");
			strcat(base,ptr->d_name);
			string dstDir = base;
			string::size_type pos(0);
			if( (pos = dstDir.find(Configure::DayuPrefix)) != std::string::npos ) {
				dstDir.replace(pos, Configure::DayuPrefix.length(), Configure::HdfsPrefix);
				mkDirs_.push_back(dstDir);
			} else {
				LOG_WARN("cann't find prefix %s, %s be ignore and not mkdir",Configure::DayuPrefix.c_str(), dstDir.c_str());
			}
			readFileList(base);
		}
	}
	return true;
}

bool CpFileInfo::loadSrcFiles(const char* loadFile)
{
	fstream fin( loadFile);
	if( !fin ) {
		LOG_ERROR("open file %s failed", loadFile);
		return false;
	}
	string srcDir;
	while( getline(fin, srcDir) ) {
		dirpair* cf = new dirpair();
		if( srcDir.find(Configure::DayuPrefix) == string::npos) {
			srcDir = Configure::DayuPrefix + "/" + srcDir;
		}

		cf->srcPath_ = srcDir;
		/*struct stat fileStatu;
		if(stat(cf->srcPath_.c_str(), &fileStatu) != 0) {
			LOG_WARN("cann't get file stat, filename: %s",cf->srcPath_.c_str());
			exit(0);
		}
		cf->srcFileSize_ = fileStatu.st_size;
		cpFileSizeSum_ += fileStatu.st_size;*/

		cf->dstPath_ = srcDir;
		std::string::size_type   pos(0);
		if( (pos=cf->dstPath_.find(Configure::DayuPrefix)) != std::string::npos ) {
			cf->dstPath_.replace(pos, Configure::DayuPrefix.length(), Configure::HdfsPrefix);
			cpFiles_.push_back(cf);
		} else {
			LOG_WARN("cann't find prefix %s, %s be ignore and not copy",Configure::DayuPrefix.c_str(), cf->srcPath_.c_str());
			delete cf;
		}
	}
	allNum_ = cpFiles_.size();
	return true;
}

dirpair* CpFileInfo::popDirPair()
{
	MutexLockGuard g(lock_);
	if(cpFiles_.empty())
		return 0;
	dirpair* p = cpFiles_.back();
	cpFiles_.pop_back();
	return p;
}

void CpFileInfo::pushSuccessPair(dirpair* p)
{
	MutexLockGuard g(slock_);
#ifdef DEBUG
	cpSuccFiles_.push_back(p);
#endif
	string s = p->srcPath_ + "\n";
	if(1 == fwrite((void *)s.c_str(), s.length(), 1, cpSucc_)) {
		fflush(cpSucc_);
	} else {
		LOG_ERROR("write failed for execute success filename:%s",p->srcPath_.c_str());
	}
	++succNum_;
	cout<<"Complete progress : "<<succNum_*100 / allNum_<<"%"<<endl;
	//cout<<"cpFinishedSize : "<<cpFinishedSize_<<" ,cpFileSizeSum:"<<cpFileSizeSum_<<endl;
	//cpFinishedSize_ += p->srcFileSize_;
	//cout<<"Complete progress : "<<cpFinishedSize_*100/cpFileSizeSum_<<"%"<<endl;
	//cout.flush();
}

void CpFileInfo::pushFailedPair(dirpair* p)
{
	MutexLockGuard g(flock_);
#ifdef DEBUG
	cpFailFiles_.push_back(p);
#endif
	string s = p->srcPath_ + "\n";
	if(1 == fwrite((void *)s.c_str(), s.length(), 1, cpFail_)) {
		fflush(cpFail_);
	} else {
		LOG_ERROR("write failed for execute failed filename:%s",p->srcPath_.c_str());
	}
}

void CpFileInfo::printResult()
{
#ifdef DEBUG
	cout<<"================SUCCESS====================\n";
	for(size_t i = 0; i < cpSuccFiles_.size(); i++) {
		cout<<cpSuccFiles_[i]->srcPath_<<endl;
	}
	cout<<"================FAILED====================\n";
	for(size_t i = 0; i < cpFailFiles_.size(); i++) {
		cout<<cpFailFiles_[i]->srcPath_<<endl;
	}
#endif
}

bool CpFileInfo::mkAlldirs()
{
	for(size_t i = 0; i < mkDirs_.size(); i++) {
		if( !FileOperator::dirIsExist(mkDirs_[i].c_str()) ) {
			LOG_DEBUG("mkDirR %s",mkDirs_[i].c_str());
			if( !FileOperator::mkDirR(mkDirs_[i].c_str())) {
				LOG_ERROR("mkDirR failed, pathname: %s", mkDirs_[i].c_str());
				return false;
			}
		}
	}
	return true;
}
