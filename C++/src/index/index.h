/*************************************************************************
	> File Name: index.h
	> Author: Dream9
	> Brief: 
	> Created Time: 2019年06月06日 星期四 15时28分45秒
 ************************************************************************/

#ifndef _INDEX_INDEX_H_
#define _INDEX_INDEX_H_

#include"doc_index.h"
#include"dictinary.h"
#include"word_segement.h"
#include"charset_transfer.h"
#include"errlog.h"
#include"md5.h"
#include"strfun.h"
#include"index_config.h"

#include<fstream>
#include<cstdlib>

#include<sys/wait.h>
#include<unistd.h>

//brief:负责索引信息的生成
//     流程顺序为1，2，3
class Index {
public:
	//1.通过原始文档生成正向索引
	int CreateForwardIndex(const char *, const char *, const char *);
    //2.结合正向索引进行文档切词
	int DocumentSegment(const char *, const char *, const char *, Dictinary &);
	//3.通过切词文档生成反向索引
	int CreateInverseIndex(const char *, const char*);

	//todo:
	int MergeInverseFile(const char **, int);
private:
	int _group_and_sort_segment_file(const char *, const char *);
};



#endif
