/*************************************************************************
	> File Name: strfun.cpp
	> Author: Dream9
	> Brief: 
	> Created Time: 2019年04月21日 星期日 18时15分48秒
 ************************************************************************/

#include"strfun.h"

//#define ERRMSG(x) ""#x

//brief:忽略大小写寻找
//转调用search
size_t StrFun::FindCase(const string &haystack,const string &needle){
	if(haystack.empty() || needle.empty()){
		return string::npos;
	}
	auto pos=std::search(haystack.begin(),haystack.end(),
			             needle.begin(),needle.end(),
		[](char a, char b) {return toupper(a) == toupper(b); });//注意这里忽略大小写进行比较
	return pos==haystack.end()?string::npos:pos-haystack.begin();
}

//brief:忽略大小写寻找
//重载
size_t StrFun::FindCase(const char *strBegin, int len, const char *targetBegin, int len_target){
	if(!strBegin || !targetBegin || len<=0 || len_target<=0){
		return string::npos;
	}
	const char *strEnd = strBegin + len;
	const char *targetEnd = targetBegin + len_target;
	auto pos=std::search(strBegin,strEnd,
			             targetBegin,targetEnd,
		[](char a, char b) {return toupper(a) == toupper(b); });//注意这里忽略大小写进行比较
	return pos == strEnd ? string::npos : pos - strBegin;
}


//brief:指定位置忽略大小写寻找
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

//brief:删除所有匹配的字串
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

//brief:替换字串
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

//brief:URL中特殊字符的翻译
//parameter:ptrUrl:待转换URL，结果也将存储于此，，left_space:剩余未使用长度
//becare:出错时*ptrUrl为'\0'，left_space=原始长度
void StrFun::TranslateUrl(char *ptrUrl, int *left_space) {
	//本质就是16进制的转换
	//因为“%B8”将会转换成一字节0184，所以原始空间肯定式够用的
	//在不需转换的情况下，二者空间正好一样
    
	//特例
	if (!ptrUrl) {
		return;
	}

	char *ptrStr;
	char *ptrCur;
	char *leader = ptrUrl;
	int len = strlen(ptrUrl);
	int use_space = 0;
	unsigned int tmp_num;
	
	ptrStr = (char *)calloc(sizeof(char), len + 1);
	if (!ptrStr) {
		//err_ret("calloc failed when %s", __FUNCTION__);
		//return -1;
		*ptrUrl = '\0';
		if (left_space) {
			*left_space = len;
		}
		return;
	}

	ptrCur = ptrStr;
	while (*ptrUrl) {
		if (*ptrUrl != '%') {
			*ptrCur++ = *ptrUrl++;
			continue;
		}
		//处理百分号编码，字符范围：[0-9,A-F]
		if (*(ptrUrl + 1) && *(ptrUrl + 2)) {
			char tmp_buf[3];
			tmp_buf[0] = *(++ptrUrl);
			tmp_buf[1] = *(++ptrUrl);
			tmp_buf[2] = '\0';
			if (1 != sscanf(tmp_buf, "%3x", &tmp_num)) {
				//err_ret("Sscanf failed:%s", tmp_buf);
				//ret = -1;
				*ptrUrl = '\0';
				if (left_space) {
					*left_space = len;
				}
				goto FREE_BUF;
			}
			*ptrCur++ = tmp_num;
		}
		else {
			//err_msg("Encode Url failed");
			//ret = -1;
			*ptrUrl = '\0';
			if (left_space) {
				*left_space = len;
			}
			goto FREE_BUF;
		}
	}
	*ptrCur = '\0';
	use_space = ptrCur - ptrStr;
	if (*left_space) {
		*left_space = len - use_space;
	}
	memmove(leader, ptrStr, use_space + 1);

FREE_BUF:
	if (ptrStr) {
		free(ptrStr);
		ptrStr = nullptr;
	}
}
