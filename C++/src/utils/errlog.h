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


#define kMAXLINE 2048

const int kLOG_DEBUG = 1;
const int kLOG_INFO = 2;
const int kLOG_WARN = 3;
const int kLOG_FATAL = 4;

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

//多线程下的日志管理
//级别，时间戳，线程id，文件名，行号，信息
//todo:
//extern void loggger(int level, int timestamp, const string &threadid, const string &file, int line, const string &message);

#endif

