/*************************************************************************
	> File Name: formatfile.cpp
	> Author: Dream9
	> Brief: 
	> Created Time: 2019年05月11日 星期六 15时08分02秒
 ************************************************************************/

#include"formatfile.h"

using std::endl;
static const int kLEN_DOWNLOAD_TIME=128;

extern std::unordered_map<string,string>mapCacheHostLookup;//存储缓存的ip，如果没有记录域名
extern const string kVERSION_ORIGINAL_PAGE;//版本号，
//同一目标编码
const string kTRAGET_CHARSET("gbk");

FormatFile::FormatFile(){}

FormatFile::FormatFile(const string &str) : FileEngine(str){}

FormatFile::~FormatFile(){
	m_ofsFile.close();
}

//brief:原始网页数据存储
//parameter:file_arg结构，记录两个指针，更多信息查阅FileEngine
//Becare:fstream不是线程安全的，需要自己做同步！！！！！
//可以考虑调用处负责对文件加锁
//2019.6.8更改：格式转换到同一格式
int FormatFile::Write(void *arg){
	if(!arg || !m_ofsFile){
		err_msg("No data or bad ofstream");
		return -1;
	}

	file_arg *ptrFile=(file_arg*)arg;

	Url *ptrUrl=ptrFile->ptrUrl;
	Page *ptrPage=ptrFile->ptrPage;

	char strTime[kLEN_DOWNLOAD_TIME];
	time_t tDate;

	//获取时间，GMT时间
	memset(strTime,0,kLEN_DOWNLOAD_TIME);
	time(&tDate);
	strftime(strTime,kLEN_DOWNLOAD_TIME,"%a, %d %b %Y %H:%M:%S GMT",gmtime(&tDate));

	/////////////////////////////////////////////////
	/////////////////////////////////////////////////
	///////////需要做同步，或者外部对文件加锁////////
	/////////////////////////////////////////////////
	/////////////////////////////////////////////////
	m_ofsFile<<"version:"<<kVERSION_ORIGINAL_PAGE<<'\n';
	if(ptrPage->m_sLocation.empty()){
		m_ofsFile<<"url:"<<ptrPage->m_sUrl;
	}
	else{
		m_ofsFile<<"url:"<<ptrPage->m_sLocation;
		m_ofsFile<<"\norigin:"<<ptrPage->m_sUrl;
	}
	m_ofsFile<<"\ndate:"<<strTime;
	auto iter=mapCacheHostLookup.find(ptrUrl->m_sHost);
	m_ofsFile<<"\nip:"
		     <<(iter==mapCacheHostLookup.end()?ptrUrl->m_sHost:iter->second);
    m_ofsFile<<"\nlength:"<<ptrPage->m_nLenContent+ptrPage->m_nLenHeader+1;
	//空行分割，写入头部和响应体
	CharsetTransfer::TransferPageCharset(ptrPage, kTRAGET_CHARSET.c_str());
	m_ofsFile<<"\n\n"<<ptrPage->m_sHeader<<"\n";
	m_ofsFile.write(ptrPage->m_sContent.c_str(),ptrPage->m_nLenContent);
	m_ofsFile<<endl;

	return 0;
}



