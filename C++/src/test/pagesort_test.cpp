/*************************************************************************
	> File Name: page_sort_test.cpp
	> Author: Dream9
	> Brief: 
	> Created Time: 2019年07月05日 星期五 15时24分05秒
 ************************************************************************/

#include"page_sort.h"

int main(){
	const char *url_index="../doc/url.index.test";
	const char *raw_data="../doc/raw.test";
	const char *doc_idx="../doc/doc.index.test";

	const char *out_file="../doc/pr.out.test";
	Pagesort pagesort_parser;
	pagesort_parser.SortPage(url_index,raw_data,doc_idx,out_file);
}
