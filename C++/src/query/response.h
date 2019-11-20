/*************************************************************************
	> File Name: response.h
	> Author: Dream9
	> Brief: 
	> Created Time: 2019��06��15�� ������ 15ʱ29��37��
 ************************************************************************/

#ifndef _INDEX_RESPONSE_H
#define _INDEX_RESPONSE_H
#include"query.h"
#include"document.h"

#include<fstream>
#include<cstring>

using std::cout;
class Response {
public:

	//�ֱ��Ӧ�����stdout��string�İ汾
	int ResponseTop(const char *);
	int ResponseTop(const char *, string&);
	
	int ResponseSelectPage(int, int, int, const char *);
	int ResponseSelectPage(int, int, int, const char *, string&);

	int ResponseResult(const char *str_query_word, score_container &query_ans,
		size_t page_number, const char *file_path, forward_index_type &doc_idx,
		vector<string> &words, vector<float> &idf);
	
	int ResponseResult(const char *str_query_word, score_container &query_ans,
		size_t page_number, const char *file_path, forward_index_type &doc_idx,
		vector<string> &words, vector<float> &idf, string& response);

private:

	int _abstract_and_highlight(const char *, std::vector<string> &, vector<float> &);
	int _abstract_and_highlight(const char *, std::vector<string> &, vector<float> &, string&);
};




#endif
