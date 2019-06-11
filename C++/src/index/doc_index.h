/*************************************************************************
	> File Name: doc_index.h
	> Author: Dream9
	> Brief: 建立url-docid-doc的索引 
	> Created Time: 2019年05月31日 星期五 15时38分24秒
 ************************************************************************/
#ifndef _INDEX_DOC_INDEX_H_
#define _INDEX_DOC_INDEX_H_

#include"document.h"
#include"errlog.h"
#include"Md5.h"
#include"index_config.h"

#include<cstdio>
#include<fstream>
#include<vector>
#include<algorithm>

#include<sys/stat.h>
#include<unistd.h>
#include<sys/types.h>

int Rename(const char *old_name, const char *new_name);

int CreateDocIndex(const char *filepath, const char *urlidx__file, const char *doc_idx_file);

int KMerge(const char **files, int number);


#endif

