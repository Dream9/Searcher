/************************************************************************
	> File Name: strfun.h
	> Author: Dream9
	> Brief: 扩展string部分功能
	> Created Time: 2019年04月21日 星期日 16时31分38秒
 ************************************************************************/
#ifndef _UTILS_STRFUN_H_
#define _UTILS_STRFUN_H_

#include"errlog.h"
#include<string>
#include<algorithm>
#include<iostream>

#ifndef ssize_t
#define ssize_t int
#endif


using std::string;

class StrFun{
public:
	StrFun() {}
	virtual ~StrFun() {}

	static void Str2Lower(string &sSource,int nLen){
		//std::transform(sSource.begin(),sSource.end(),sSource.begin(),std::tolower);
		std::transform(sSource.begin(),sSource.begin()+nLen,sSource.begin(),::tolower);
	}
	static void Str2Upper(string &sSource,int nLen){
		//std::transform(sSource.begin(),sSource.end(),sSource.begin(),std::topper);
		std::transform(sSource.begin(),sSource.begin()+nLen,sSource.begin(),::toupper);
	}

	//static string itos(long long i){//转调用to_string
	//  return to_string(i);
	//
	//	stringstream s;
	//	s<<i;
	//	return s.str();
	//}

	static size_t FindCase(const string &haystack,
			                          const string &needle);//等同于search
    static size_t FindCaseFrom(const string &haystack,
			                              const string &needle,
										  string::size_type From );
	static void ReplaceStr(string &str,const string &srstr,const string &dsstr);//等同于replace
	static void EraseStr(string &str,const string &substr);
};







#endif /*_UTILS_STRFUN_H_*/
