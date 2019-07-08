/*************************************************************************
	> File Name: page_sort.h
	> Author: Dream9
	> Brief:
	> Created Time: 2019��07��04�� ������ 14ʱ42��16��
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

//brief:����PRֵΪ��ҳ����
//     �ȶ�ȡurl_index�ļ�����¼ÿ��url MD5��Ӧ��docid,�Դ���Ϊurl��ID
//     Ȼ���ȡraw�����ļ�����ȡ���Ӳ�ͳ�Ƴ���
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
