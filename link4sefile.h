/*************************************************************************
	> File Name: link4sefile.h
	> Author: Dream9
	> Brief: 
	> Created Time: 2019年05月11日 星期六 17时09分37秒
 ************************************************************************/

#ifndef _UTILS_LINK4SEFILE_H_
#define _UTILS_LINK4SEFILE_H_

#include"file_engine.h"

#include<unordered_map>
#include<vector>

const string kVERSION_LINK4SE("1.0");

class Link4SEFile: public FileEngine{
public:
	Link4SEFile();
	Link4SEFile(const string &);
	virtual ~Link4SEFile();

	int GetFileType() const {
		return kLINK4SE;
	}

	virtual int Write(void *);
};


#endif
