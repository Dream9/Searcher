/*************************************************************************
	> File Name: query_config.cpp
	> Author: Dream9
	> Brief: 
	> Created Time: 2019年06月17日 星期一 15时40分36秒
 ************************************************************************/
#include<cstdlib>
#include<cstdio>
//
//最长查询长度
extern const int kMAX_QUERY_LENGTH=50*2;

extern const char kSEPARATE_CHAR='/';
extern const char kSEPARATE_CHAR_INVERSE_FILE=' ';

/*response.cpp需要的*/
extern const size_t kNUMBER_OF_ANSWER_EACH_PAGE=4;
const char *kDEPLOYMENT_ADDRESS="/D9";
const char *kIMG_ADDRESS="/no.jpg";
const char *kCGI_QUERY_PATH="/D9/query.cgi";
const char *kCGI_LEFT_PAGE_PATH="/D9/left.cgi";

//brief:没有任何作用的日志
//todo
int _logger(const char *query_string, const char *time_info){
	
	printf("%s,%s",query_string, time_info);
	return 0;
}



