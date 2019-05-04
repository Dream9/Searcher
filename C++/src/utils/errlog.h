/*************************************************************************
	> File Name: errlog.h
	> Author: Dream9
	> Brief: 
	> Created Time: 2019年04月23日 星期二 16时59分19秒
 ************************************************************************/

#ifndef _UTILS_ERRLOG_H_
#define _UTILS_ERRLOG_H_

#include<cstdlib>
#include<cstdio>
#include<string.h>

#include<stdarg.h>
#include<errno.h>
#include<syslog.h>

//最大信息长度
#define kMAXLINE 2048
//主程序负责定义，为0时表示采用syslog，否则输出至stderr
extern const int  log_to_stderr;

//分为四类错误输出
//最终调用err_doit实现

//与系统有关的致命错误
void err_sys(const char *fmt, ...);

//与系统有关的非致命错误
void err_ret(const char*fmt, ...);

//与系统无关的致命错误
void err_quit(const char *fmt, ...);

//与系统无关的非致命错误
void err_msg(const char *fmt, ...);

//日志输出
//普通log
void log_msg(const char *fmt, ...);

//调试用
void log_debug(const char *fmt, ...);

#endif

