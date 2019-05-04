/*************************************************************************
	> File Name: page.h
	> Author: Dream9
	> Brief: 声明Page，功能在于解析response,并提取进一步搜索的信息
	> Created Time: 2019年04月26日 星期五 15时45分46秒
 ************************************************************************/
#ifndef _UTILS_PAGE_H_
#define _UTILS_PAGE_H_

#include"url.h"

#include<string>
#include<map>
#include<vector>
#include<list>

const int kANCHOR_TEXT_LEN = 256;
const int kMAX_URL_REFERENCES = 1024;
const int kURL_REFERENCE_LEN = (kURL_LEN+kANCHOR_TEXT_LEN)*kMAX_URL_REFERENCES>>1;
const int kMAX_TAG_NUMBERS = 10240;
const int kMIN_CONTENTLINKINFO = 20;
const int kMIN_INFO_LEN = 8;
const int kMIN_URL_LEN = 4;

using std::string;
using std::map;
using std::vector;

//标识文本或其他网页
enum page_type{
	kPlain_Text,
	kOther
};

struct RefLink4SE{
	char *link;///记录链接
	char *anchor_text;//记录对外显示的锚点内容
	string strCharset;//标识字符集类型
};

struct RefLink4History{
	char *link;
};

class Page{
public:
	enum page_type m_eType;//该网页类型
	string m_sUrl;//url最初
	
	//header部分解析结果
	string m_sHeader;//响应头
	int m_nLenHeader;//头长
	
	int m_nStatusCode;//相应状态
	int m_nContentLength;//长度，接受数据超过此长度的网页应该丢弃
	string m_sLocation;//url重定向
	bool m_bConnectionState;//是否keep alive
	string m_sContentEncoding;//解压缩方式，，  比如Content-Encoding：gzip
	string m_sContentType;//网页类型,,   比如Content-Type: image/jpeg
	string m_sCharset;//字符集
	string m_sTransferEncoding;//传输编码方式,是一种content-length的替代功能

	//body部分解析结果
	string m_sContent;//响应体
	int m_nLenContent;//体长
	string m_sContentNoTags;

	//
	string m_sContentLinkInfo;//link,处于lash up状态


	//将一个网页中的链接提取出来然后分到2大类，4小类之中
	//信息分开，如下：
	//为了进一步搜索准备的链接
	////////，但出于lash-up状态
	string m_sLinkInfo4SE;
	int m_nLenLinkInfo4SE;
	////////,处于good 状态
	RefLink4SE m_RefLink4SE[kMAX_URL_REFERENCES];
	int m_nRefLink4SENum;

	//为了之后存档准备的链接，比如图片
	////////，但出于lash-up状态
	string m_sLinkInfo4History;
	int m_nLenLinkInfo4History;
	////////,处于good状态
	RefLink4History m_RefLink4History[kMAX_URL_REFERENCES>>1];
	int m_nRefLink4HistoryNum;


	map<string,string>m_mapLink4SE;//形式为： link - anchor_text
	vector<string>m_vecLink4History;

private:
	//主要分为两大功能
	//解析响应头，
	//解析响应体（网页主体部分）
	
	//解析响应头
	void GetStatusCode(string &header);
	void GetContentLength(string &header);
	void GetConnectionState(string &header);
	void GetLocation(string &header);
	void GetCharset(string &);
	void GetContentEncoding(string &);
	void GetContentType(string &);
	void GetTransferEncoding(string &);

	//解析响应体
	bool GetContentLinkInfo();
	bool GetLinkInfo4SE();
	bool GetLinkInfo4History();
	bool FindRefLink4SE();
	bool FindRefLink4History();

public:
	Page();
	Page(const string &,const string &,const char *,const char *,int);

	void ParseHeaderInfo(string &);//外部接口，调用需要的解析响应头的函数
	bool ParseHyperLinks();//外部接口，解析响应体，也就是网页
	//bool NormalizeUrl(string &);
	//bool IsFilterLink(string &);
};

int NormalizeUrl(string &);
int IsFilterLink(string &);


#endif

