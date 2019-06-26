/*************************************************************************
	> File Name: cgi_test.cpp
	> Author: Dream9
	> Brief:
	> Created Time: 2019��06��17�� ����һ 14ʱ53��43��
 ************************************************************************/
#include"response.h"
#include"query.h"

#include<sys/time.h>
#include<time.h>
#include<cstdio>

int main() {
	//����Ҫ���Ĺ���Ŀ¼������
	chdir("/mnt/share/Dreamn/DreamSE/src/test/");
	int page_no = 1;
	const char *rawpath= "../doc/raw.test";

	unordered_map<int, int>doc_index;
	const char *doc_index_path = "../doc/doc.index.test";
	unordered_map<string, string>inverse_index;
	const char *inverse_index_file = "../doc/inverse.index.test";
	Dictinary dict;
	const char *dict_path = "../doc/word.dict";
	score_container ans;
	vector<string> words;
	vector<float> idf;

	//��ʼ����ѯ����
	Query query_agent;
	query_agent.EnvironmentInit(doc_index,doc_index_path,inverse_index,inverse_index_file);
	int total_doc = doc_index.size();
	query_agent.GetDictinary(dict, dict_path);

	//ʵ���е�ѭ������
	timeval begin_tv;
	gettimeofday(&begin_tv,nullptr);
	query_agent.GetInput();
	query_agent.HandleQueryString(dict);
	query_agent.GetAnswer(total_doc,inverse_index,ans,words,idf);
	timeval end_tv;
	gettimeofday(&end_tv,nullptr);
	float used_msec = (end_tv.tv_sec-begin_tv.tv_sec)*1000 
		+((float)(end_tv.tv_usec-begin_tv.tv_usec))/(float)1000; 

	//�������
	Response feedback;
	feedback.ResponseTop(query_agent.m_query_string.c_str());
	feedback.ResponseResult(query_agent.m_query_string.c_str(), ans, page_no, rawpath, doc_index,words,idf);
	feedback.ResponseSelectPage(ans.size(),used_msec,1,"456asd1f98sdf");//todo:�����ַ�����ʾ����ĸôʵ���Ϣid
	//������л�
	//
	//
	//��ս��
	score_container().swap(ans);
	//ѭ������
}
