/*************************************************************************
	> File Name: rio.h
	> Author: Dream9 
	> Brief:��׳��read/write
	> Created Time: 2019��05��07�� ������ 16ʱ12��3��
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
//rio_t�ṹ��
//brief:��Ϊ��д�Ŀ⻺��
typedef struct {
    int rio_fd;  //�ļ�������
    int rio_cnt; //����δ��ȡ����
    char *rio_bufptr;  //δ��ȡ������ָ��
    char rio_buf[RIO_BUFFER_SIZE];  //������
} rio_t;
#undef RIO_BUFFER_SIZE


 //�ļ���������rio����������
 /*����Ӧ�ý���rio_readinit������*/
//void rio_init(rio_t*rp, int fd) {
//	rp->rio_fd = fd;
//	rp->rio_cnt = 0;
//	rp->rio_bufptr = rp->rio_buf;
//}


//RIO�Ķ�д����
//��ʼ��rio_t�ṹ
void Rio_init(rio_t *, int );

//������
ssize_t RioReadlineb(rio_t *rp, void*usrbuf, size_t maxlen);
ssize_t RioReadnb(rio_t *rp, void*usrbuf, size_t n);

//��������
ssize_t RioWrite(int , void *, size_t);
ssize_t RioRead(int , void *, size_t);


//����Ϊ�ṩ�ķ�������׳��I/O����
ssize_t RioRead_nonb(int, void *, size_t, int);
//todo
//ssize_t RioWrite_nonb(int, void *, size_t, int);

#endif
