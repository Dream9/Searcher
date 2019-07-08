/*************************************************************************
	> File Name: page_sort.h
	> Author: Dream9
	> Brief:
	> Created Time: 2019年07月04日 星期四 14时42分16秒
 ************************************************************************/

#ifndef _INDEX_PAGERANK_PAGESORT_H
#define _INDEX_PAGERANK_PAGESORT_H

#include"pagerank_sparse.h"
#include"errlog.h"
#include"page.h"
#include"md5.h"

#include<map>
#include<string>
#include<cstdio>
#include<cstdlib>
#include<fstream>
#include<vector>
#include<ctime>

#include<unistd.h>
#include<fcntl.h>
#include<sys/time.h>


using std::map;
using std::string;
using std::pair;
using std::ios;

//brief:根据PR值为网页排名
//     先读取url_index文件，记录每个url MD5对应的docid,以此作为url的ID
//     然后读取raw数据文件，提取链接并统计出度
class Pagesort {
public:
	int SortPage(const char *url_index, const char *raw_data, const char *doc_idx, const char *out_put);

private:
	map<string, int> _m_mapUrlId;
	int _load_url_indx(const char *url_index);
	int _get_outdegree(const char *raw_data, const char *doc_idx, FILE *tempfile);
	int _sort_page(FILE *temp, const char *out_put);
};

#endif
