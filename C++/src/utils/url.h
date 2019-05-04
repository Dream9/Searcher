/*************************************************************************
	> File Name: url.h
	> Author: Dream9
	> Brief: 
	> Created Time: 2019年04月17日 星期三 20时36分33秒
 ************************************************************************/

#ifndef _UTILS_URL_H_
#define _UTILS_URL_H_
#include<string>
#include<mutex>


using std::string;

const int16_t kURL_LEN=256;
const int16_t kHOST_LEN=256;
const int16_t kPROTOCOL_LEN=10;
const int16_t kREQUEST_LEN=256;
const int16_t kDEFAULT_HTTP_PORT=80;
const int16_t kDEFAULT_FTP_PORT=21;
const int16_t kMIN_HOST_LEN = 6;

enum url_scheme{
	kSchemeHttp,
	kSchemeFtp,
	kSchemeInvalid
};

class Url{
public:
	string m_sUrl;
	url_scheme m_eScheme;

	string m_sHost;
	int32_t m_nPort;
	string m_sPath;      //请求信息
	//m_sPath将包含一下信息
	//1.Path
	//2.Params ?A=B&C=D
	//3.Query
	//4.Fragment #A  也就是锚部分
public:
	Url();
	~Url();

	//解析ms_url，结果存放在类中
	bool ParseUrl(const string &);

    void ParseUrl(const char *, char *, int, char *, int, char *, int, int *);
	//通过域名得到ip
    char* GetIpByHost(const char *);

	bool IsValidHost(const char *);
	bool IsValidIp(const char *);
	bool IsForeignHost(const string &);
	bool IsImageUrl(const string &);
	//bool IsUnreachedUrl(const char *);
	bool IsVisitedUrl(const char *);
	bool IsValidHostChar(char ch);

private:
	void ParseScheme(const char *url);
};

extern std::mutex mutexMemory;
//extern pthread_mutex_t mutexMemory;

#endif
//

