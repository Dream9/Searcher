/*************************************************************************
	> File Name:http_config.h
	> Author: Dream9
	> Brief: 
	> Created Time: 2019��05��13�� ����һ 19ʱ20��25��
 ************************************************************************/

#ifndef _HTTP_CONFIG_H_
#define _HTTP_CONFIG_H_

#include<string>
#include<vector>

using std::string;
using std::vector;

//��Http�йص�
const int kMAX_MEMORY_LENGTH = 5 * 1024 * 1024;//����ڴ����
const int kDEFAULT_TIMEOUT = 30;//��ʱʱ��
const string kHTTP_VERSION("HTTP/1.0");
const int kHIDE_USER_AGENT=0;//0:����,��0:������
const string UA_POOL[] = {/*safari 5.1 �C Windows*/ 
	"Mozilla/5.0 (Windows; U; Windows NT 6.1; en-us) AppleWebKit/534.50 (KHTML, like Gecko) Version/5.1 Safari/534.50",
	/*IE 9.0*/
	"Mozilla/5.0 (compatible; MSIE 9.0; Windows NT 6.1; Trident/5.0",
	/*Firefox 4.0.1 �C Windows*/
	"Mozilla/5.0 (Windows NT 6.1; rv:2.0.1) Gecko/20100101 Firefox/4.0.1",
	/*Opera 11.11 �C MAC*/
	"Opera/9.80 (Macintosh; Intel Mac OS X 10.6.8; U; en) Presto/2.8.131 Version/11.11",
	/*Chrome 17.0 �C MAC*/
	"Mozilla/5.0 (Macintosh; Intel Mac OS X 10_7_0) AppleWebKit/535.11 (KHTML, like Gecko) Chrome/17.0.963.56 Safari/535.11",
	/*����֮����The World�� 3.x */
	"Mozilla/4.0 (compatible; MSIE 7.0; Windows NT 5.1; The World)",
	/*360����� */
	"Mozilla/4.0 (compatible; MSIE 7.0; Windows NT 5.1; 360SE)",
	/*Green Browser */
	"Mozilla/4.0 (compatible; MSIE 7.0; Windows NT 5.1)" };
const int kLEN_UA_POOL = end(UA_POOL) - begin(UA_POOL);
const int kHEADER_BUF_SIZE = 1024;
int isHandleImage=0;//0:������image��ҳ����;��0:����image��ҳ
int kDEFAULT_PAGE_BUFSIZE = 1024 * 200;
int kMAX_PAGE_BUFSIZE = kMAX_MEMORY_LENGTH;

int Timeout=-1;// ����0����ʱ����������ֵ;����ʹ��Ĭ��ֵ
char *userAgent=nullptr;//nullptr:����UA_POOL�е�ֵ��������ô�ֵ



#endif
