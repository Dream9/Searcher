/*************************************************************************
	> File Name: url.cpp
	> Author: Dream9
	> Brief: 
	> Created Time: 2019年04月18日 星期四 19时28分46秒
 ************************************************************************/
#include"url.h"
#include"strfun.h"
#include"md5.h"
#include"errlog.h"

#include<iostream>
#include<cstring>
#include<mutex>
#include<map>
#include<vector>
#include<set>

#include<sys/socket.h>
#include<netdb.h>
#include<arpa/inet.h>

//#ToDo
//#include"http.h"

//注意，mutex的移植问题


#define ERROR_OUT(x) printf("[error][%s-%d-%s]"#x":%s",__FILE__,__LINE__,__FUNCTION__,x);
#define ENSURE(x) if(x){}else{ERROR_OUT(x);exit(EXIT_FAILURE);}
#define DEBUG_L

std::map<string,string> mapCacheHostLookup;//扩展DNS缓存
std::mutex mutexCacheHost;//
extern std::map<uint32_t,uint32_t> mapIpBlock;//允许访问IP段,注意这里存储的是网络号和主机掩码
extern std::set<string> setVisitedUrlMd5;//记录已经访问的URL的MD5  
extern std::vector<string> vsUnreachHost;//记录不可访问的域名
using valTypeCHL=std::map<string,string>::value_type;

static const char *kHome_Host[] = {//国内域名后缀
	"cn","com","net","org","info","biz","tv","cc","hk","tw"
};
static const char *kImage_Type[] = {//过滤Image后缀
	"gif","jpg","jpeg","png","bmp","tif","psd"
};

struct scheme_data{
	const char* type_string;
	int32_t port;
	int32_t enabled;
};

//用于判断Url所属类型、合法性
static scheme_data known_scheme[]={
    {"http://",kDEFAULT_HTTP_PORT,1},
	{"ftp://",kDEFAULT_FTP_PORT,1},
	{nullptr,-1,0}                       //作为终结判断的标志，也作为非法标志
};

//根据已知类型确定url的m_eScheme
void Url::ParseScheme(const char* url){
    int i=0;
	for(;known_scheme[i].type_string;++i){
		if(0==strncasecmp(url,known_scheme[i].type_string,strlen(known_scheme[i].type_string))){
			if(known_scheme[i].enabled){
				this->m_eScheme=url_scheme(i);
				return;
			}else{
				this->m_eScheme=kSchemeInvalid;
				return;
			}
		}
	}
	this->m_eScheme=kSchemeInvalid;
}

//提取url中的信息
bool Url::ParseUrl(const string& strUrl){
    char protocol[kPROTOCOL_LEN];
	char host[kHOST_LEN];
	char request[kREQUEST_LEN];
	int port =-1;

	memset(protocol,0,sizeof protocol);
	memset(host,0,sizeof host);
    memset(request,0,sizeof request);

	//提取scheme
	this->ParseScheme(strUrl.c_str());
	if(this->m_eScheme!=kSchemeHttp){
		return false;
	}

    this->ParseUrl(strUrl.c_str(),protocol,sizeof protocol,
			          host,sizeof host,request,sizeof request,&port);
	this->m_sHost=host;
	this->m_sUrl=strUrl;
	this->m_sPath=request;
	if(port>0){
		m_nPort=port;
	}
	return true;
}

//brief:详细解析URL的各个部分
void Url::ParseUrl(const char *url, char *protocol,int protocol_len, char *host,int host_len,
	               char *request,int requset_len,int *port){
    char *work,*ptr,*ptr2;
	*protocol=*host=*request=0;
	*port=80;

	int len=strlen(url);
#ifdef DEBUG_L
	std::unique_lock<std::mutex> tmp_lock(mutexMemory);
#endif
	work=new char[len+1];
#ifdef DEBUG_L
	tmp_lock.unlock();
#endif
	memset(work,0,len+1);
	std::copy_n(url,len,work);

    //提取http
	ptr=strchr(work,':');//得到：后面的指针
	if(ptr){
		*(ptr++)=0;        //添加一个终结符，全部拷贝，通过‘/0’判断结束
		std::copy_n(work,protocol_len,protocol);
	}
	else{
		//*protocol="HTTP";
		memcpy((void*)protocol, "HTTP", protocol_len);
		ptr=work;
	}

	//跳过//符号
	if(*ptr=='/' && *(ptr+1)=='/')
		ptr+=2;
	ptr2=ptr;
	
	//解析域名
	while(*ptr2 && IsValidHostChar(*ptr2))
		++ptr2;
	*ptr2=0;  //添加一个终结符
    std::copy(ptr,ptr2+1,host);//注意终结符也要拷贝进去

	//剩余部分置于request
	int offset=ptr2-work;
	std::copy(url+offset,url+len+1,request);
	
	ptr=strchr(host,':');
	if(ptr){
		*ptr=0;
		*port=atoi(ptr+1);
	}
#ifdef DEBUG_L
	tmp_lock.lock();
#endif
	delete []work;
#ifdef DEBUG_L
	tmp_lock.unlock();
#endif
	work=nullptr;
}


Url::Url(){
	this->m_sUrl="";
	this->m_eScheme=kSchemeInvalid;
	this->m_sHost="";
	this->m_nPort=kDEFAULT_HTTP_PORT;
	this->m_sPath="";
}


Url::~Url(){
	//ToDo
}


//结合DNS缓存计算域名的IP,返回的是一个点分十进制的IP的char*
//注意内存分配在动态空间，需要释放
char* Url::GetIpByHost(const char * host){
	//针对空指针或者非法host
	if(!host || !IsValidHost(host)){
		return nullptr;
	}

	in_addr inaddr;
	char *result=nullptr;
	int len=0;

	//inaddr=(uint32_t)inet_addr(host);//转换为struct in_addr，然后转换一下
	//errno=0;
	if(0!=inet_aton(host,&inaddr)){
	//ENSURE(int(inaddr)!=-1);
	//if(INADDR_NONE!=inaddr)
		//转换成功时：意味着这个host本身就是点分十进制形式
		int len=strlen(host);
#ifdef DEBUG_L
		std::unique_lock<std::mutex> tmp_lock(mutexMemory);
#endif
		result=new char[len+1];
#ifdef DEBUG_L
	    tmp_lock.unlock();
#endif
		memset(result,0,len+1);
		memcpy(result,host,len);

		return result;
	}
	else{
		//host不是合法的点分十进制格式
		//首先从扩展DNS中查找
		//
		auto iter=mapCacheHostLookup.find(host);
		if(iter!=mapCacheHostLookup.end()){
			//已经缓存了
			const char *strHostIp;
			strHostIp=iter->second.c_str();

			//inaddr=(uint32_t)inet_addr(strHostIp);
			//if(INADDR_NONE!=inaddr)
			if(0!=inet_aton(strHostIp,&inaddr)){
				len=strlen(strHostIp);
#ifdef DEBUG_L
				std::unique_lock<std::mutex> tmp_lock(mutexMemory);
#endif
				result=new char[len+1];
#ifdef DEBUG_L 
				tmp_lock.unlock();
#endif
				memset(result,0,len+1);
				memcpy(result,strHostIp,len);
				return result;
			}
		}
	}

	//如果运行到这里说明扩展缓存未命中
	//需要调用gethostbyname
    hostent *hp;
	hp=gethostbyname(host);
	if(hp==nullptr){
		return nullptr;
	}
	//需要缓存这个host
	memcpy(&inaddr,*(hp->h_addr_list),hp->h_length);
	char ipv4_buf[INET_ADDRSTRLEN];//AF_INET ipv4点分十进制长度
	if(nullptr==inet_ntop(AF_INET,(void*)&inaddr,ipv4_buf,sizeof ipv4_buf)){
		//出错
		err_ret("[%s-%d-%s]:inet_ntop()failed",__FILE__,__LINE__,__FUNCTION__);
		return nullptr;
	}
	else{
		//追加缓存
		//锁
		std::unique_lock<std::mutex> tmp_host_lock(mutexCacheHost);
		if(false==mapCacheHostLookup.insert(valTypeCHL(host,ipv4_buf)).second){
			err_msg("[%s-%d-%s]:update map faild",__FILE__,__LINE__,__FUNCTION__);
		}
		tmp_host_lock.unlock();

		len=strlen(ipv4_buf);
#ifdef DEBUG_L
		std::unique_lock<std::mutex>tmp_lock(mutexMemory);
#endif
		result=new char[len+1];
#ifdef DEBUG_L
		tmp_lock.unlock();
#endif
		memset(result,0,len+1);
		memcpy(result,ipv4_buf,len);
		return result;
	}
}
	



//判断是否为合法的host字符,注意包含 :-_.
inline bool Url::IsValidHostChar(char ch){
	return (isdigit(ch) || isalpha(ch) || ':'==ch
		   || '.'==ch || '-'==ch || '_'==ch);
}


//初步判断是否为合法host
bool Url::IsValidHost(const char *host){
	if(!host || strlen(host)<kMIN_HOST_LEN){   //kMIN_HOST_LEN取6
		return false;
	}
	int len=strlen(host);
	for(int i=0;i<len;++i){
		if(!IsValidHostChar(*(host+i))){
			return false;
		}
	}
	return true;
}


//通过Md5记录查询，判断是否访问过
bool Url::IsVisitedUrl(const char *url){
	if(!url){
		return true;
	}
	Md5 check_md5;
	check_md5.GenerateMd5((unsigned char*)url,strlen(url));
	string strToken=check_md5.ToString();
		return  (setVisitedUrlMd5.find(strToken)!=setVisitedUrlMd5.end())?true:false;
}


//判断IP是否合法，附带限制IP访问范围
bool Url::IsValidIp(const char *ip){
	if(!ip){
		return false;
	}
    //判断方式就是利用inet_aton功能，查看返回
	in_addr inaddr;
	if(0==inet_aton(ip,&inaddr)){
		return false;
	}
	in_addr *tmpptr=&inaddr;
	uint32_t long_inaddr=*(uint32_t*)tmpptr;
		
	//是否位于允许访问block范围
	if(!mapIpBlock.empty()){
		for(auto i:mapIpBlock){
			//注意这里存储的网络地址和主机掩码
			if(i.first==(long_inaddr & ~(i.second))){
				return true;
			}
		}
		return false;
	}
	return true;//没有限制
}


//粗略判断是否为国外域名
bool Url::IsForeignHost(const string& host){
	if(host.empty()) return true;

	struct in_addr inaddr;
	if(0==inet_aton(host.c_str(),&inaddr)){
		return false;
	}

	size_t index=host.find_last_of('.');
	string tmp_host;
	if(index==string::npos){
		return true;//没有后缀的默认无效
	}
	tmp_host=host.substr(index+1);
	StrFun::Str2Lower(tmp_host,tmp_host.size());//利用transform实现

    int kHome_Host_len=std::end(kHome_Host)-std::begin(kHome_Host);//c++11
	for(int i=0;i<kHome_Host_len;++i){
		if(tmp_host==kHome_Host[i])
			return false;
	}

	return true;
}

//排除部分不符合条件的链接，比如图片，或者太长了
bool Url::IsImageUrl(const string& url){
    if(url.empty() || url.size()>kHOST_LEN){
		return false;
	}

	size_t index=url.find_last_of('.');
	string tmp_url;
	if(index==string::npos){
		return false;//没有后缀
	}
	tmp_url=url.substr(index+1);
	StrFun::Str2Lower(tmp_url,tmp_url.size());

	int kImage_Type_len=std::end(kImage_Type)-std::begin(kImage_Type);
	//for(auto iter=std::begin();iter!=std::end();++iter)
	for(int i=0;i<kImage_Type_len;++i){
        if(tmp_url==kImage_Type[i])
			return true;
	}
	return false;
}
