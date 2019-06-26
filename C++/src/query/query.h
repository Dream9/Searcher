/*************************************************************************
	> File Name: query.h
	> Author: Dream9
	> Brief:
	> Created Time: 2019��06��12�� ������ 14ʱ39��17��
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

//����ѯ������
extern const int kMAX_QUERY_LENGTH;
//�ָ���
extern const char kSEPARATE_CHAR;
extern const char kSEPARATE_CHAR_INVERSE_FILE;
//�ⲿ��־��¼
extern int _logger(const char *, const char *);


const char kHTML_PARAMETER_DELIM_CHAR = '&';
const char kHTML_PARAMETER_EQUAL_CHAR = '=';


//δ�������
const int kERROR_QUERY = -1;
//��ѯ�ִ�����(�ض�),������󲢲����жϵ���
const int kERROR_QUERY_EXCEED_SIZE = 1;
//��������
const int kERROR_QUERY_PARAMETER = -3;
//�������ݳ���
const int kERROR_QUERY_LOAD_DATA = -4;
//ԭʼ���ݸ�ʽ����
const int kERROR_QUERY_DATA = -5;
//��־����
const int kERROR_LOG = -6;


//brief:���ּ�¼�ṹ
//becare��
//dataΪmap<������docid>�ṹ
//containerΪunordered_map<docid,��docidλ��data�ĵ�����>�ṹ
//���ս��������data�е����ݣ���ɻ������õ�docid
//Ĭ�Ͻ�������ϵ���ҳ����ǰ��
typedef map<float, int, std::greater<float>> score_type;
class _score {
public:
	typedef score_type::iterator mapped_type;
	typedef std::pair<float,int> data_value_type;

	//���캯��
	_score(score_type *_data) :data(_data) {
		//data->clear();
	}
	
	//��������
	//�����δ��¼docid,��ֱ��insert,���¼�¼�ĵ�����
	//������Ҫ�Ȼ�õ�ǰֵ��Ȼ��ɾ��֮��insert��ֵ�������¼�¼�ĵ�����
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


//�����ѯ
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
	
	//�洢�������
	//container m_query_string;
	////�洢һ������
	string m_query_string;
	
private:
    int _load_forward_data(unordered_map<int,int> &, const char *);
	int _load_inverse_data(unordered_map<string,string> &, const char *);
	int _log_query();


};



#endif
