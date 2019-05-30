/*************************************************************************
	> File Name: rio.h
	> Author: Dream9 
	> Brief:健壮的read/write
	> Created Time: 2019年05月07日 星期五 16时12分3秒
 ************************************************************************/
#ifndef _UTILIS_RIO_H_
#define _UTILIS_RIO_H_


#include<cstdlib>
#include<cstdio>
#include<errno.h>
#include<string.h>

#include<sys/select.h>
#include<time.h>
#include<unistd.h>
#include<fcntl.h>

#ifndef ssize_t 
typedef long ssize_t;
#endif


#define RIO_BUFFER_SIZE 8192
//rio_t结构体
//brief:作为读写的库缓冲
typedef struct {
    int rio_fd;  //文件描述符
    int rio_cnt; //缓冲未读取长度
    char *rio_bufptr;  //未读取缓冲区指针
    char rio_buf[RIO_BUFFER_SIZE];  //缓冲区
} rio_t;
#undef RIO_BUFFER_SIZE


 //文件描述符与rio缓冲区关联
 /*这里应该叫做rio_readinit更合适*/
//void rio_init(rio_t*rp, int fd) {
//	rp->rio_fd = fd;
//	rp->rio_cnt = 0;
//	rp->rio_bufptr = rp->rio_buf;
//}


//RIO的读写函数
//初始化rio_t结构
void Rio_init(rio_t *, int );

//带缓冲
ssize_t RioReadlineb(rio_t *rp, void*usrbuf, size_t maxlen);
ssize_t RioReadnb(rio_t *rp, void*usrbuf, size_t n);

//不带缓冲
ssize_t RioWrite(int , void *, size_t);
ssize_t RioRead(int , void *, size_t);


//以下为提供的非阻塞健壮的I/O操作
ssize_t RioRead_nonb(int, void *, size_t, int);
//todo
//ssize_t RioWrite_nonb(int, void *, size_t, int);

#endif
