/*
 * CpConfig.cc
 *
 *  Created on: 2014-10-23
 *      Author: hgw
 */
#include "CpConfig.h"

//#define DEBUG

using namespace std;

#ifdef DEBUG
string Configure::dstUser = "root";
string Configure::dstGroup = "root";

const string Configure::HdfsPrefix = "/home/hgw/test";
const string Configure::DayuPrefix = "/usr/local";
#else
string Configure::dstUser = "hunantv";
string Configure::dstGroup = "hunantv";

const string Configure::HdfsPrefix = "/mnt/hdfs/hunantv/源素材库";
const string Configure::DayuPrefix = "/mnt/dayu";
#endif

std::string Configure::LogDir = "/var/log/DistCp";
char Configure::startTime[200] = {0};

vector<string> Configure::HOSTS;
