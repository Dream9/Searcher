/*
*cgi 环境测试
*cgi 环境测试
*cgi 环境测试
*/
#include<strfun.h>
#include<iostream>
#include<string.h>
#include<cstdio>
#include<cstdlib>
using namespace std;
const int kMAX_QUERY_LENGTH=1024;
const char kHTML_PARAMETER_EQUAL_CHAR='=';
const char kHTML_PARAMETER_DELIM_CHAR='&';

int main(){
	string m_query_string;
	cout<<"Content-type:text/html\r\n\r\n";
	cout<<"<html><head><meta charset=\"gbk\"><title>test</title></head>\n";
	const char *method = getenv("REQUEST_METHOD");
	if (!method) {
		cout<<"bad method ,nullptr"<<endl;
	}
	cout<<method<<endl;
	int length;
	int ret=0;
	char parameter[kMAX_QUERY_LENGTH + 1];
	memset(parameter, 0, sizeof parameter);
	if (memcmp(method, "POST", sizeof "POST" -1) == 0) {
		//从stdin中获取
		length = atoi(getenv("CONTENT_LENGTH"));
		if (0 == length) {
			cout<<"0 length !"<<endl;
		}
		else if (length > kMAX_QUERY_LENGTH) {
			//过长截断
			length = kMAX_QUERY_LENGTH;
		}
		/*
		if (length != RioRead(STDIN_FILENO, parameter, length)) {
			cout<<"not match"<<endl;
		}
		*/
	}
	else if (memcmp(method, "GET", sizeof "GET" - 1) == 0) {
		//GET需要从环境变量中提取
		const char *ptr = getenv("QUERY_STRING");
		length = strlen(ptr);
		cout<<"len:"<<length<<endl;
		if (0 == length) {
			cout<<"o"<<endl;
		}
		else if (length > kMAX_QUERY_LENGTH) {
			length = kMAX_QUERY_LENGTH;
		}
		memcpy(parameter, ptr, length);
		cout<<"thi word is :"<<parameter<<endl;
	}
	else{
		cout<<"bad method type";
		return -1;
	}
	//解析参数串，格式形如 word=六六六&sdf=sss
	//但考虑到扩展性可以假设有不确定个参数需要提取
	/*假设有多个参数：
	vector<string> vec_query;
	ParameterSplit(parameter, kHTML_PARAMETER_DELIM_CHAR, vec_query);
	for (string &str : vec_query) {
		vector<string> tmp;
		ParameterSplit(str.c_str(), kHTML_PARAMETER_EQUAL_CHAR, tmp);
		if (tmp.size() != 2) {
			return kERROR_QUERY_PARAMETER;
		}
		m_query_string.emplace_back((tmp[0], tmp[1]));
	}
	*/

	//对于本程序，仅第一个参数为word，其他的无用
	char *pos_equal_char = strchr(parameter, kHTML_PARAMETER_EQUAL_CHAR);
	if (!pos_equal_char) {
		cout<<"not find =";
		return -1;
	}
	StrFun::TranslateUrl(pos_equal_char,&length);//length用于返回剩余长度
	cout<<"after translate :"<<pos_equal_char<<endl;
	char *pp=pos_equal_char;
	while(pp && *pp){
		uint8_t tmp=uint8_t(*pp);
		cout<<hex<<tmp<<'\t';
		++pp;
	}
	cout<<endl;
	char *pos_end=strchr(pos_equal_char,kHTML_PARAMETER_DELIM_CHAR);
	
	if(pos_end){
		m_query_string.assign(pos_equal_char+1,pos_end) ;//= (pos_equal_char + 1);
	}
	else{
		m_query_string.assign(pos_equal_char+1);
	}
	if (m_query_string.empty()) {
		cout<<"empty query";
		return -1;
	}
	cout<<m_query_string;
	cout<<"</html>";
	return ret;
}
