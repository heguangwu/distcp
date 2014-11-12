/*
 * main.cc
 *
 *  Created on: 2014-10-23
 *      Author: hgw
 */
#include "CpFileInfo.h"
#include "CpConfig.h"
#include "CpThread.h"
#include "FileOperator.h"
#include "Logging.h"
#include <iostream>
#include <sys/klog.h>
#include <vector>
#include <unistd.h>
#include <sys/types.h>
#include <pwd.h>
#include <getopt.h>
#include <sstream>

using namespace std;

void changeugid(void) {
	uid_t wrk_uid;
	gid_t wrk_gid;

	if (geteuid()==0) {
		char pwdgrpbuff[16384];
		struct passwd pwd,*pw;

		cout<<"change to user "<<Configure::dstUser<<endl;

		getpwnam_r(Configure::dstUser.c_str(), &pwd, pwdgrpbuff, 16384, &pw);
		if (pw == NULL) {
			cout<<"ERROR: "<< Configure::dstUser.c_str()<<" no such user - "<<endl;
			exit(1);
		}
		wrk_uid = pw->pw_uid;
		wrk_gid = pw->pw_gid;

		if (setgid(wrk_gid) < 0) {
			cout<<"ERROR: can't set gid to "<< (int)wrk_gid<<endl;
			exit(1);
		} else {
			cout<<"INFO: set gid to"<<(int)wrk_gid<<endl;
		}
		if (setuid(wrk_uid)<0) {
			cout<<"ERROR: can't set uid to "<<(int)wrk_uid;
			exit(1);
		} else {
			cout<<"INFO: set uid to "<<(int)wrk_uid<<endl;
		}
	}
}

void Usage(char* appname)
{
	cout<<"Usage: "<<appname<<" [-t threads_per_host] [-l logdir] [-f load_srcdir_from_file] [-h host_list] [-L loglevel] copydir"<<endl;
	cout<<"Usage: "<<appname<<" [--threads threads_per_host] [--logdir logdir] [--file load_srcdir_from_file] [--hosts host_list] [--loglevel loglevel] copydir"<<endl;
	cout<<"for example: "<<appname<<" -h h1,h2,h3 -L DEBUG|INFO|WARN|ERROR "<<" /mnt/dayu/hunantv"<<endl;
	exit(0);
}

int main(int argc, char* argv[])
{
	if(argc < 2) {
		Usage(argv[0]);
		return 1;
	}

	string hosts;
	int threadsPerHost = 1;
	const char* short_options = "t:h:l:f:";
	struct option long_options[] = {
		 { "threads",  1,   NULL,    't'     },
		 { "hosts",    1,   NULL,    'h'     },
		 { "logdir",   1,   NULL,    'l'     },
		 { "file", 	   1,   NULL,    'f'     },
		 { "loglevel", 1,   NULL,    'L'     },
		 {      0,     0,     0,     0},
	};

	int c;
	string loadFileName;
	string logLevel;
	while((c = getopt_long (argc, argv, short_options, long_options, NULL)) != -1) {
		switch (c) {
		case 't':
			threadsPerHost = atoi(optarg);
			break;
		case 'h':
			hosts = optarg;
			break;
		case 'l':
			Configure::LogDir = optarg;
			break;
		case 'f':
			loadFileName = optarg;
			break;
		case 'L':
			logLevel = optarg;
			break;
		case '?':
			cout<<"Error arguments\n";
			Usage(argv[0]);
			exit(0);
		}
	}

	if(hosts.empty()) {
		cout<<"ERROR: must configure host list use -h\n";
		Usage(argv[0]);
		return 1;
	}

	stringstream ss(hosts);
	string buf;
	const char delim = ',';
	while ( std::getline(ss, buf, delim) ) {
		Configure::HOSTS.push_back(buf);
	}

	LogLevel l = LL_DEBUG;
	if(logLevel == "INFO") {
		l = LL_INFO;
	} else if(logLevel == "WARN") {
		l = LL_WARNING;
	} else if(logLevel == "ERROR") {
		l = LL_ERROR;
	}
	LOG_INIT(l, "DistCp", Configure::LogDir.c_str());

	CpFileInfo cfinfo;
	if(loadFileName.empty()) {
		LOG_INFO("Copy path:%s", argv[optind]);
		cfinfo.setSrcFile(argv[optind]);
		if(cfinfo.readCpFileList()) {
			LOG_INFO("readCpFileList Success");
		}
		if( !cfinfo.mkAlldirs() ) {
			LOG_ERROR("mkAlldirs failed");
			return 3;
		}
		LOG_INFO("make copy directory Success");
	} else {
		if( !cfinfo.loadSrcFiles(loadFileName.c_str()) ) {
			LOG_ERROR("Load src files failed");
			return 4;
		}
	}

	vector<CpThread*> threads;
	for(size_t i = 0; i < Configure::HOSTS.size(); i++) {
		for(int pt = 0; pt < threadsPerHost; pt++) {
			string s = Configure::HOSTS[i];
			CpThread* th = new CpThread(cfinfo, Configure::HOSTS[i]);
			threads.push_back(th);
		}
	}

	LOG_INFO("start %d threads ... ", threads.size());
	for(size_t i = 0; i < threads.size(); i++) {
		if( !threads[i]->start() ) {
			LOG_ERROR("Start threads failed");
			return 2;
		}
	}

	for(size_t i = 0; i < threads.size(); i++) {
		threads[i]->join();
	}

	cfinfo.printResult();
	return 0;
}
