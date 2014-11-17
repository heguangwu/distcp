/*
 * Configure.h
 *
 *  Created on: 2014-10-23
 *      Author: hgw
 */

#ifndef CP_CONFIG_H_
#define CP_CONFIG_H_

#include <string>
#include <vector>

class Configure
{
public:
	static std::string dstUser;
	static std::string dstGroup;

	static const std::string HdfsPrefix;
	static const std::string DayuPrefix;

	static std::string LogDir;

	static char startTime[200];

	static int taskId;

};

#endif /* CONFIGURE_H_ */
