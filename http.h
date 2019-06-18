/*************************************************************************
	> File Name: http.h
	> Author: Dream9
	> Brief:
	> Created Time: 2019��05��03�� ������ 21ʱ08��56��
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

//������Ϣ
const int kHTTP_ERROR = -1;//-1:����
const int kHTTP_OUTOFRANGE = -2;//-2:����IPBlock������Χ
const int kHTTP_INVALIDHOST = -3;//-3:��Ч����
const int kHTTP_IMG = -4;//-4:��ҳΪimg/xxx����
const int kHTTP_LOCATION = -300;//-300:�ض���
const int kHTTP_QUIT = -5;//û���ƶ�fileBuf,ֱ�Ӷ������յ�������


class Http {
private:
	string m_sUrl;
	int *m_psock;

private:
    //��װ����
	int _WrapHeader(const string &, Url &,void **);
	
	//��ȡ��Ӧ
	int ReadHeader(int, char *);
	int ReadBody(int, int, char **);
	int ReadBody_chunked(int, char **);

	//��������TCP������
	int CreateSocket(const char *, int);
	//������������
	int NonbConnect(int, sockaddr *, int);

	//realloc�ķ�װ
	int CheckBufSize(char **, int *, int);
	int CheckBufSize(char **, char **, char **, int);

public:
	Http();
	virtual ~Http();

	//����ҵ��
	int Fetch(const string &, char **, char **, char **, int *);
};

extern std::mutex mutexMemory;

#endif /*_UTILS_HTTP_H_*/

