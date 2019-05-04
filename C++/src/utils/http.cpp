/*************************************************************************
	> File Name: http.cpp
	> Author: Dream9
	> Brief: 
	> Created Time: 2019年05月03日 星期五 21时14分23秒
 ************************************************************************/

#include"http.h"
#include"strfun.h"
#include"url.h"
#include"errlog.h"
//Todo
//#include"dnse.h"
//#include"commondef.h"

#include<cstdlib>
#include<cstdio>
#include<string>
#include<errno.h>
#include<netdb.h>
#include<unistd.h>
#include<netinet/in.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<sys/time.h>
#include<fcntl.h>

//应当定义在commondef.h中去
const int kDEFAULT_TIMEOUT = 60;


int timeout = kDEFAULT_TIMEOUT;
char *userAgent = nullptr;
int hideUserAgent = 0;

//错误信息
const int kHTTP_ERROR = -1;//-1:出错
const int kHTTP_OUTOFRANGE = -2;//-2:超出IPBlock搜索范围
const int kHTTP_INVALIDHOST = -3;//-3:无效域名
const int kHTTP_IMG = -4;//-4:网页为img/xxx类型
const int kHTTP_LOCATION = -300;//-300:重定向

//brief:建立tcp连接
//return:描述符或则出错信息
//主要在于拼接sockaddr_in所需要的信息，然后进行connect
int Http::CreateSocket(const char *host, int port) {
	int sockfd;
	sockadd_in sa;
	in_addr inaddr;
	int ret;

	Url url_parser;
	char *ip = url_parser.GetIpByHost(host);
	if (!ip) {
		log_debug("空域名或者无效域名:%s",host);
		return kHTTP_INVALIDHOST;
	}
	if (!url_parser.IsValidIp(ip)) {
		//释放内存
		log_debug("超范围IP:%s", ip);
#ifdef DEBUG_L
		std::unique_guard<std::mutex> tmp_guard(&mutexMemory);
#endif
		delete[]ip;
#ifdef DEBUG_L
		tmp_guard.unlock();
#endif
		ip = nullptr;
		return kHTTP_OUTOFRANGE;
	}
	if (0 == inet_aton(ip, &inaddr)) {
		//转换失败，说明非法
		log_debug("非法IP:%s", ip);
#ifdef DEBUG_L
		std::unique_guard<std::mutex> tmp_guard(&mutexMemory);
#endif
		delete[]ip;
#ifdef DEBUG_L
		tmp_guard.unlock();
#endif	
		ip = nullptr;
		return kHTTP_ERROR;
	}
	//运行到这里，说明是合法连接地址，初始化sockaddr_in结构信息
	memcpy((void*)&sa.sin_addr, (void*)inaddr, sizeof inaddr);
	sa.sin_family = AF_INET;
	sa.sin_port = htons(port);
	//创建一个描述符
	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
		err_ret("create socket failed");
		return kHTTP_ERROR;
	}
	//开启这个描述符的重用功能
	//又不是服务器，这样子做意义？
	//可能是短时间内可能建立太多连接，提高复用吧
	int optval = 1;
	if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, (const void*)&optval, sizeof optval) != 0) {
		err_ret("Reuse failed");
		return kHTTP_ERROR;
	}
	
	//建立连接
	if (ret = NonbConnect(sockfd, (sockaddr *)&sa, kDEFAULT_TIMEOUT) == -1) {
		err_msg("nonb-connect failed");
		close(sockfd);
		return kHTTP_ERROR;
	}
	return sockfd;
}


//brief:非阻塞的连接远程
//parameter：套接字描述符，sockaddr，超时信息
//利用select进行非阻塞的I/O操作,同时把sockfd设置为NONBLOCK状态
int Http::NonbConnect(int sockfd, sockaddr *sa, int timewait) {
	fd_set multiplexing;
	int respose_status;
	timeval time_to_out;
	int flags;

	//设置file status
	if ((flags = fcntl(sockfd, F_GETFL, 0)) == -1) {
		err_ret("%d getfl failed", sockfd);
		return kHTTP_ERROR;
	}
	flags |= O_NONBLOCK;
	if (fcntl(sockfd, F_SETFL, flags) < 0) {
		err_ret("%d setfl failed", sockfd);
		return kHTTP_ERROR;
	}
	//注意这里并没有实现重试
	//todo:增加重连，考虑到指数补偿，以及可迁移性
	if (0 == connect(sockfd, sa, sizeof *sa)) {
		//在非阻塞模式下，不能成功会立刻返回
		flags &= O_NONBLOCK;
		if (fcntl(sockfd, F_SETFL, flags) < 0) {
			err_ret("%d reset to blocking failed", sockfd);
			return kHTTP_ERROR;
		}
		return sockfd;
	}
	//通过select等待连接结果
	FD_ZERO(&multiplexing);
	FD_SET(sockfd, &multiplexing);
	time_to_out.tv_sec = timewait;
	time_to_out.tv_usec = 0;
	//只等待到可以写即可
	respose_status = select(sockfd + 1, nullptr, &multiplexing, nullptr, &time_to_out);

	switch (respose_status) {
	case -1:
		//连接失败
		;
		return kHTTP_ERROR;
	case 0:
		//超时
		return kHTTP_ERROR;
	default:
		break;
	}
	//连接成功



}
