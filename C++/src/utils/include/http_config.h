/*************************************************************************
	> File Name:http_config.h
	> Author: Dream9
	> Brief: 
	> Created Time: 2019年05月13日 星期一 19时20分25秒
 ************************************************************************/

#ifndef _HTTP_CONFIG_H_
#define _HTTP_CONFIG_H_

#include<string>
#include<vector>

using std::string;
using std::vector;

//与Http有关的
const int kMAX_MEMORY_LENGTH = 5 * 1024 * 1024;//最大内存配额
const int kDEFAULT_TIMEOUT = 30;//超时时间
const string kHTTP_VERSION("HTTP/1.0");
const int kHIDE_USER_AGENT=0;//0:匿名,非0:不匿名
const string UA_POOL[] = {/*safari 5.1 – Windows*/ 
	"Mozilla/5.0 (Windows; U; Windows NT 6.1; en-us) AppleWebKit/534.50 (KHTML, like Gecko) Version/5.1 Safari/534.50",
	/*IE 9.0*/
	"Mozilla/5.0 (compatible; MSIE 9.0; Windows NT 6.1; Trident/5.0",
	/*Firefox 4.0.1 – Windows*/
	"Mozilla/5.0 (Windows NT 6.1; rv:2.0.1) Gecko/20100101 Firefox/4.0.1",
	/*Opera 11.11 – MAC*/
	"Opera/9.80 (Macintosh; Intel Mac OS X 10.6.8; U; en) Presto/2.8.131 Version/11.11",
	/*Chrome 17.0 – MAC*/
	"Mozilla/5.0 (Macintosh; Intel Mac OS X 10_7_0) AppleWebKit/535.11 (KHTML, like Gecko) Chrome/17.0.963.56 Safari/535.11",
	/*世界之窗（The World） 3.x */
	"Mozilla/4.0 (compatible; MSIE 7.0; Windows NT 5.1; The World)",
	/*360浏览器 */
	"Mozilla/4.0 (compatible; MSIE 7.0; Windows NT 5.1; 360SE)",
	/*Green Browser */
	"Mozilla/4.0 (compatible; MSIE 7.0; Windows NT 5.1)" };
const int kLEN_UA_POOL = end(UA_POOL) - begin(UA_POOL);
const int kHEADER_BUF_SIZE = 1024;
int isHandleImage=0;//0:不解析image网页类型;非0:解析image网页
int kDEFAULT_PAGE_BUFSIZE = 1024 * 200;
int kMAX_PAGE_BUFSIZE = kMAX_MEMORY_LENGTH;

int Timeout=-1;// 大于0：超时计算采用这个值;否则，使用默认值
char *userAgent=nullptr;//nullptr:采用UA_POOL中的值；否则采用此值



#endif
