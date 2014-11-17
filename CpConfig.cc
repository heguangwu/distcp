/*
 * CpConfig.cc
 *
 *  Created on: 2014-10-23
 *      Author: hgw
 */
#include "CpConfig.h"

using namespace std;

string Configure::dstUser = "hunantv";
string Configure::dstGroup = "hunantv";

std::string Configure::LogDir= "/var/log/DistCp";
char Configure::startTime[200] = {0};

int Configure::taskId = -1;
