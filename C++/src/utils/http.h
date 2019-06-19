/*************************************************************************
	> File Name: http.h
	> Author: Dream9
	> Brief:
	> Created Time: 2019年05月03日 星期五 21时08分56秒
*************************************************************************/

#ifndef _UTILS_HTTP_H_
#define _UTILS_HTTP_H_


#include"strfun.h"
#include"url.h"
#include"errlog.h"
#include"rio.h"
#include"page.h"

#include<cstdlib>
#include<cstdio>
#include<string>
#include<map>
#include<string>
#include<mutex>

#include<errno.h>
#include<netdb.h>
#include<unistd.h>
#include<netinet/in.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<sys/time.h>
#include<arpa/inet.h>
#include<fcntl.h>

using std::string;

//错误信息
const int kHTTP_ERROR = -1;//-1:出错
const int kHTTP_OUTOFRANGE = -2;//-2:超出IPBlock搜索范围
const int kHTTP_INVALIDHOST = -3;//-3:无效域名
const int kHTTP_IMG = -4;//-4:网页为img/xxx类型
const int kHTTP_LOCATION = -300;//-300:重定向
const int kHTTP_QUIT = -5;//没有制定fileBuf,直接丢弃接收到的数据


class Http {
private:
	string m_sUrl;
	int *m_psock;

private:
    //封装请求
	int _WrapHeader(const string &, Url &,void **);
	
	//读取响应
	int ReadHeader(int, char *);
	int ReadBody(int, int, char **);
	int ReadBody_chunked(int, char **);

	//创建基于TCP的连接
	int CreateSocket(const char *, int);
	//非阻塞的连接
	int NonbConnect(int, sockaddr *, int);

	//realloc的封装
	int CheckBufSize(char **, int *, int);
	int CheckBufSize(char **, char **, char **, int);

public:
	Http();
	virtual ~Http();

	//核心业务
	int Fetch(const string &, char **, char **, char **, int *);
};

extern std::mutex mutexMemory;

#endif /*_UTILS_HTTP_H_*/

