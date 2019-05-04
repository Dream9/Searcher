/*************************************************************************
	> File Name: errlog.cpp
	> Author: Dream9
	> Brief: 
	> Created Time: 2019年04月24日 星期三 20时18分45秒
 ************************************************************************/

#include"errlog.h"

static void err_doit(int ,int ,const char *,va_list);
static void log_doit(int, int, int, const char *, va_list);

//与系统有关的致命错误
void err_sys(const char *fmt, ...){
	va_list ap;

	va_start(ap,fmt);
	err_doit(1,errno,fmt,ap);
	va_end(ap);
}

//与系统有关的非致命错误
void err_ret(const char *fmt, ...){
	va_list ap;

	va_start(ap,fmt);
	err_doit(1,errno,fmt,ap);
	va_end(ap);

	exit(1);
}

//与系统无关的致命错误
void err_quit(const char *fmt, ...){
	va_list ap;

	va_start(ap,fmt);
	err_doit(0,0,fmt,ap);
	va_end(ap);

	exit(1);
}

//与系统无关的非致命错误
void err_msg(const char *fmt, ...){
    va_list ap;

	va_start(ap,fmt);
	err_doit(0,0,fmt,ap);
	va_end(ap);
}

//调试信息
void log_msg(const char *fmt, ...) {
	//err_msg("")
	va_list ap;

	va_start(ap, fmt);
	err_doit(0, 0, fmt, ap);
	va_end(ap);
}

void log_debug(const char *fmt, ...) {
	va_list ap;

	va_start(ap, fmt);
	log_doit(0, 0, LOG_ERR, fmt, ap);
	va_end(ap);
}

//最终调用
static void err_doit(int errnoflag,int error,const char *fmt,va_list ap){
	char buf[kMAXLINE];

	int len=vsnprintf(buf,kMAXLINE,fmt,ap);
	if(len<0){
		fprintf(stderr,"another error occurs when dealing with one error,%s",strerror(errno));
		exit(1);
	}
	if(errnoflag){
		snprintf(buf+len,kMAXLINE-1-len,":%s",strerror(error));
	}

	strcat(buf,"\n");
	fflush(stdout); //防止stdout和stderr被重定向在一起了
	fputs(buf,stderr);
	fflush(nullptr);//刷新所有缓冲
}

static void log_doit(int errnoflag, int error, int priority, const char *fmt, va_list ap) {
	char buf[kMAXLINE];

    int len=vsnprintf(buf,kMAXLINE,fmt,ap);
	if(len<0){
		fprintf(stderr,"another error occurs when dealing with one error,%s",strerror(errno));
		exit(1);
	}
	if(errnoflag){
		snprintf(buf+len,kMAXLINE-1-len,":%s",strerror(error));
	}

	strcat(buf,"\n");
	if (log_to_stderr) {
		fflush(stdout); //防止stdout和stderr被重定向在一起了
		fputs(buf, stderr);
		fflush(nullptr);//刷新所有缓冲
	}
	else {
		//由syslogd进程负责处理
		syslog(priority, "%s", buf);
	}
}

