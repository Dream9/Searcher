/*************************************************************************
	> File Name: isamfile.h
	> Author: Dream9
	> Brief: 
	> Created Time: 2019年05月12日 星期日 15时09分39秒
 ************************************************************************/
#ifndef _UTILS_ISAMFILE_H_
#define _UTILS_ISAMFILE_H_

#include"file_engine.h"

#include<cstdio>

//brief:方便查找的数据组织方式，分成两个文件，索引文件和数据文件
//todo:注意当前采用的线性记录索引的方式
//     应当改成使用b+tree的索引组织形式
class IsamFile:public FileEngine{
public:
    string m_sIndexFileName;//索引文件路径

	FILE *fpDataFile;
	FILE *fpIndexFile;

public:
	IsamFile();
	IsamFile(const string &);
	IsamFile(const string &, const string &);
	virtual ~IsamFile();

	int GetFileType() const {
		return kISAM;
	}
	virtual int Write(void *);
	int Open(const string &, const string &);
	virtual void Close();

	int Open(const string &){
		return -1;
	}
};


#endif

