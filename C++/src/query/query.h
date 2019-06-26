/*************************************************************************
	> File Name: query.h
	> Author: Dream9
	> Brief:
	> Created Time: 2019年06月12日 星期三 14时39分17秒
 ************************************************************************/

#ifndef _QUERY_QUERY_H_
#define _QUERY_QUERY_H_

#include"rio.h"
#include"strfun.h"
#include"word_segment.h"
#include"dictinary.h"

#include<cstdlib>
#include<cstring>
#include<vector>
#include<unordered_map>
#include<map>
#include<set>
#include<fstream>
#include<cmath>
#include<ctime>
#include<algorithm>

#include<unistd.h>

using std::string;
using std::vector;
using std::unordered_map;
using std::map;
using std::ios;
using std::set;

//最大查询串长度
extern const int kMAX_QUERY_LENGTH;
//分隔符
extern const char kSEPARATE_CHAR;
extern const char kSEPARATE_CHAR_INVERSE_FILE;
//外部日志记录
extern int _logger(const char *, const char *);


const char kHTML_PARAMETER_DELIM_CHAR = '&';
const char kHTML_PARAMETER_EQUAL_CHAR = '=';


//未分类错误
const int kERROR_QUERY = -1;
//查询字串过长(截断),这类错误并不会中断调用
const int kERROR_QUERY_EXCEED_SIZE = 1;
//参数错误
const int kERROR_QUERY_PARAMETER = -3;
//加载数据出错
const int kERROR_QUERY_LOAD_DATA = -4;
//原始数据格式问题
const int kERROR_QUERY_DATA = -5;
//日志问题
const int kERROR_LOG = -6;


//brief:评分记录结构
//becare：
//data为map<分数，docid>结构
//container为unordered_map<docid,该docid位于data的迭代器>结构
//最终结束后遍历data中的数据，便可获得排序好的docid
//默认降序，最符合的网页在在前面
typedef map<float, int, std::greater<float>> score_type;
class _score {
public:
	typedef score_type::iterator mapped_type;
	typedef std::pair<float,int> data_value_type;

	//构造函数
	_score(score_type *_data) :data(_data) {
		//data->clear();
	}
	
	//更新数据
	//如果尚未记录docid,则直接insert,更新记录的迭代器
	//否则需要先获得当前值，然后删除之，insert新值，并更新记录的迭代器
	void update(data_value_type v) {
		 auto iter = container.find(v.first);
		 if (iter==container.end()) {
			 mapped_type data_iter=(data->insert(v)).first;
			 container.insert({ v.second, data_iter });
		 }
		 else {
			 float current = (*iter->second).first;
			 data->erase(iter->second);
			 v.first += current;
			 iter->second = (data->insert(v)).first;
		 }
	}

private:
	unordered_map<int, mapped_type> container;
	score_type *data;
};
//typedef _score score_container;
typedef vector<int> score_container;
typedef unordered_map<int, int> forward_index_type;


//代理查询
class Query {
public:
    typedef std::pair<string, string> parameter_type;
	typedef vector<parameter_type> container;

public:
	int EnvironmentInit(unordered_map<int,int> &, const char *, unordered_map<string,string> &, const char *);
	//int QueryStringInit(Dictinary &);
    int GetInput();
	int GetDictinary(Dictinary &, const char *);
	int HandleQueryString(Dictinary &);
	int GetAnswer(int, unordered_map<string,string> &, score_container &,vector<string> &, vector<float> &);

	int ParameterSplit(const char *, const char, vector<string> &);
	
	//存储多个参数
	//container m_query_string;
	////存储一个参数
	string m_query_string;
	
private:
    int _load_forward_data(unordered_map<int,int> &, const char *);
	int _load_inverse_data(unordered_map<string,string> &, const char *);
	int _log_query();


};



#endif
