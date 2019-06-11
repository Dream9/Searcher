/*************************************************************************
	> File Name: dictinary.h
	> Author: Dream9
	> Brief: 管理词典
	> Created Time: 2019年06月03日 星期一 14时42分25秒
 ************************************************************************/

#ifndef _INDEX_DICTIONARY_H_
#define _INDEX_DICTIONARY_H_

#include"errlog.h"

#include<unordered_map>
#include<string>

using std::string;
using std::unordered_map;

//brief:词典类，提供查询和统计频率作用
class Dictinary {
public:
	Dictinary() {};
	int IsWord(const string &) const;

	//todo:统计频率
	int GetFrequence(const string &);
	int AddFrequence(const string &);

private:
	unordered_map<string, int> hashDict;

public:
	int OpenDictinary(const char *, int flag);
	int OpenDictinary(const char *);
};



#endif

