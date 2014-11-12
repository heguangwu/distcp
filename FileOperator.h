/*
 * FileOperator.h
 *
 *  Created on: 2014-10-23
 *      Author: hgw
 */

#ifndef FILEOPERATOR_H_
#define FILEOPERATOR_H_
#include <vector>
#include <string>

class FileOperator
{
public:
	static bool dirIsExist(const char* dirname);
	static bool fileIsExist(const char* filename);
	static bool mkDirR(const char* dirname);
};

#endif /* FILEOPERATOR_H_ */
