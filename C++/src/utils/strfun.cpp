/*************************************************************************
	> File Name: strfun.cpp
	> Author: Dream9
	> Brief: 
	> Created Time: 2019年04月21日 星期日 18时15分48秒
 ************************************************************************/

#include"strfun.h"

//#define ERRMSG(x) ""#x

//忽略大小写寻找
//转调用
size_t StrFun::FindCase(const string &haystack,const string &needle){
	if(haystack.empty() || needle.empty()){
		return string::npos;
	}
	auto pos=std::search(haystack.begin(),haystack.end(),
			             needle.begin(),needle.end(),
		[](char a, char b) {return toupper(a) == toupper(b); });//注意这里忽略大小写进行比较
	return pos==haystack.end()?string::npos:pos-haystack.begin();
}

//指定位置忽略大小写寻找
//通过search实现
size_t StrFun::FindCaseFrom(const string &haystack,const string &needle,string::size_type nFrom){
    if(haystack.empty() || needle.empty() || nFrom>=haystack.size()){
		return string::npos;
	}

	if(nFrom<0){
		err_quit("(%s:%d:%s]:nFrom<0!",__FILE__,__LINE__,__FUNCTION__);
	}

	auto pos=std::search(haystack.begin()+nFrom,haystack.end(),
			             needle.begin(),needle.end(),
						 [](char a,char b){return toupper(a)==toupper(b);});
    return pos==haystack.end()?string::npos:(pos-haystack.begin());
}

//删除所有匹配的字串
//通过循环find-erase实现
void StrFun::EraseStr(string &str,const string &substr){
	if(str.empty() || substr.empty()){
		return;
	}
	auto pos=str.find(substr);
	auto len_substr=substr.size();
	while(pos!=string::npos){
		str.erase(pos,len_substr);
		pos=str.find(substr);
	}
}

//替换字串
//find-replace循环完成
void StrFun::ReplaceStr(string &str,const string &target,const string &dest){
	if(str.empty() ||target.empty()){
		return ;
	}

	auto pos=str.find(target);
	auto len_target=target.size();
	auto len_dest=dest.size();

	while(pos!=string::npos){
		str.replace(pos,len_target,dest);
		pos=str.find(target,pos+len_dest);
	}
}

