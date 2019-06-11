/*************************************************************************
	> File Name: dictinary.h
	> Author: Dream9
	> Brief: ����ʵ�
	> Created Time: 2019��06��03�� ����һ 14ʱ42��25��
 ************************************************************************/

#ifndef _INDEX_DICTIONARY_H_
#define _INDEX_DICTIONARY_H_

#include"errlog.h"

#include<unordered_map>
#include<string>

using std::string;
using std::unordered_map;

//brief:�ʵ��࣬�ṩ��ѯ��ͳ��Ƶ������
class Dictinary {
public:
	Dictinary() {};
	int IsWord(const string &) const;

	//todo:ͳ��Ƶ��
	int GetFrequence(const string &);
	int AddFrequence(const string &);

private:
	unordered_map<string, int> hashDict;

public:
	int OpenDictinary(const char *, int flag);
	int OpenDictinary(const char *);
};



#endif

