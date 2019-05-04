#include"../utils/page.h"

#include<cstring>
#include<cstdlib>
#include<cstdio>
#include<mutex>
#include<map>
#include<set>
#include<string>

//:Page(const string &strUrl, const string &strLocation, const char *header, const char *body, int nLenBody)
const int kMAX_BODY = 1024 * 40;
const string url = "http://movie.douban.com/review/9431885/";
//注意需要扩展到https！！！！！！！！
//const string url = "https://movie.douban.com/review/9431885/";
const string location = "";
const string header = "HTTP/1.1 200 OK\r\nDate: Thu, 02 May 2019 06:19:26 GMT\r\n"
"Content-Type: text/html; charset=utf-8\r\nTransfer-Encoding: chunked\r\n"
"Connection: keep-alive\r\nKeep-Alive: timeout=30\r\n"
"Vary: Accept-Encoding\r\nX-Xss-Protection: 1; mode=block\r\n"
"X-Douban-Mobileapp: 0\r\nExpires: Sun, 1 Jan 2006 01:00:00 GMT\r\n"
"Pragma: no-cache\r\nCache-Control: must-revalidate, no-cache, private\r\n"
"X-DAE-Node: fram12\r\nX-DAE-App: zerkalo\r\n"
"Server: dae\r\nX-Content-Type-Options: nosniff\r\n"
"Content-Encoding: gzip\r\n\r\n";
//const string body = "";

std::mutex mutexMemory;
std::map<uint32_t,uint32_t>mapIpBlock;
std::set<std::string> setVisitedUrlMd5;
std::vector<std::string> vsUnreachHost;

int main() {
	printf("Test For Page:[%s]",__FUNCTION__);
	fflush(stdout);
	auto f = fopen("./page_test.html", "rb");
	char cbody[kMAX_BODY];
	char *ptr_cbody = cbody;
	char c;
	int num = 0;
	while (num<kMAX_BODY-1 &&(c = fgetc(f)) != EOF) {
		*ptr_cbody++ = c;
	}
	*ptr_cbody='\0';
	int lenbody = strlen(cbody);
	printf("测试网页长度%d",lenbody);
	fflush(stdout);
	Page p_t(url, location, header.c_str(), cbody, lenbody);
	p_t.ParseHyperLinks();
	auto fout = fopen("./page_test.out.log", "wb");
	fprintf(fout, p_t.m_sLinkInfo4SE.c_str());
	fprintf(fout, p_t.m_sLinkInfo4History.c_str());
	fclose(fout);
}
