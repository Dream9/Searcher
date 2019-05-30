/*************************************************************************
	> File Name: crawl.h
	> Author: Dream9
	> Brief: 
	> Created Time: 2019年05月12日 星期日 18时43分10秒
 ************************************************************************/

#ifndef _CRAWL_CRAWL_H_
#define _CRAWL_CRAWL_H_

//
#include"url.h"
#include"page.h"
#include"http.h"
#include"strfun.h"
#include"formatfile.h"
#include"link4sefile.h"
#include"isamfile.h"
#include"errlog.h"
#include"md5.h"

#include<chrono>
#include<unordered_map>
#include<unordered_set>
#include<functional>
#include<sstream>
#include<thread>

#include<arpa/inet.h>
#include<signal.h>
#include<unistd.h>
#include<zlib.h>///sudo apt-get install zlib1g-dev


//brief:爬虫类
//Becare:采用C的FILE流，没有使用fstream
class Crawl{
private:
	string m_sInputFileName;//输入初始化Link文件
	string m_sOutputFileName;//输出解析过的Link文件
	int m_nThreadNums;//线程数目
	int m_nPageEachThread;//每个线程下载个数
    int m_nRunTimeThread;//每个线程运行时间

public:
	IsamFile m_isamFile;//ISAM存储

    //连接存储
	FILE * m_ofsLink4SEFile;
	FILE * m_ofsLink4HistoryFile;

	//访问过得Url及page的Md5存储
	FILE * m_ofsVisitedUrlFile;
	FILE * m_ofsVisitedUrlMd5File;
	FILE * m_ofsVistitedPageMd5File;

	//无法访问的Url及host存储
	//FILE * m_ofsUnreachUrlFile;//弃用//
	FILE * m_ofsUnreachHostFile;

	//弃用
	//ofstream m_ofsLink4SEFile;
	//ofstream m_ofsLink4SEHistoryFile;

	////访问过得Url及page的Md5存储
	//ofstream m_ofsVisitedUrlFile;
	//ofstream m_ofsVisitedUrlMd5File;
	//ofstream m_ofsVistitedPageMd5File;

	////无法访问的Url及host存储
	//ofstream m_ofsUnreachUrlFile;
	//ofstream m_ofsUnreachHostFile;

private:
	//默认数目:默认线程数，默认每个线程爬取页面数，默认线程运行时间
	//注意后两者视不同策略发挥作用
	enum {
		kDefaultThread=10,
		kDefaultPageEachThread=1000,
		kDefaultThreadRunTime=3600
	};

public:
	Crawl();
	Crawl(const string &, const string &, int, int, int);
	~Crawl();
    
	int Collect();

	int DownloadFile(FormatFile *, Link4SEFile *,
			         Url &, int &);

	int fetch(void *);

	int AddUrl(const char *);

	//用于初始化setVisitedUrlMd5以及mapVisitedPageMd5
	int GetVisitedUrlMd5();
	int GetVisitedPageMd5();
	//用于单个ulr/page的Md5记录
	int SaveVisitedUrlMd5(const string &);//url md5
	int SaveVisitedPageMd5(const string &,const string &);//page md5,url
    //退出时候的保存
	void SaveUnvisitedUrl();//为访问的url记录出来
	int SaveReplicas(const char *file);//重复页面信息记录，是SaveVisitedPageMd5的上位



	int GetIpBlock();

	int GetUnreachHostMd5();
	int OpenFilesForOutput();

	//Raw数据记录
	int SaveFormatFile(FormatFile *, Url *, Page *);
	int SaveLink4SEFile(Link4SEFile *, Url *, Page *);
	int SaveIsamFile(Url *, Page *);

	//link数据记录
	int SaveUnreachHost(const string &);//输出到setUnreachHost
	int SaveVisitedUrl(const string &);//输出到用户指定文件
	int SaveLink4SE(Page *);//输出到mapUrl,并记录出度信息
	int SaveLink4History(Page *);//输出到文件

	//旧版本实现，弃用
	//int SaveLink4SE(void *);

	

private:
	int _get_original_url_from_file(FILE *);
};

#endif
