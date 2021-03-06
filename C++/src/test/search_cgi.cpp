/*************************************************************************
	> File Name: cgi_test.cpp
	> Author: Dream9
	> Brief:
	> Created Time: 2019年06月17日 星期一 14时53分43秒
 ************************************************************************/
#include"response.h"
#include"query.h"

#include<sys/time.h>
#include<time.h>
#include<cstdio>

int main() {
	//首先要更改工作目录。。。
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

	//初始化查询环境
	Query query_agent;
	query_agent.EnvironmentInit(doc_index,doc_index_path,inverse_index,inverse_index_file);
	int total_doc = doc_index.size();
	query_agent.GetDictinary(dict, dict_path);

	//实际中的循坏部分
	timeval begin_tv;
	gettimeofday(&begin_tv,nullptr);
	query_agent.GetInput();
	query_agent.HandleQueryString(dict);
	query_agent.GetAnswer(total_doc,inverse_index,ans,words,idf);
	timeval end_tv;
	gettimeofday(&end_tv,nullptr);
	float used_msec = (end_tv.tv_sec-begin_tv.tv_sec)*1000 
		+((float)(end_tv.tv_usec-begin_tv.tv_usec))/(float)1000; 

	//结果反馈
	Response feedback;
	feedback.ResponseTop(query_agent.m_query_string.c_str());
	feedback.ResponseResult(query_agent.m_query_string.c_str(), ans, page_no, rawpath, doc_index,words,idf);
	feedback.ResponseSelectPage(ans.size(),used_msec,1,"456asd1f98sdf");//todo:最后的字符串表示缓存的该词的信息id
	//结果序列化
	//
	//
	//清空结果
	score_container().swap(ans);
	//循环结束
}
