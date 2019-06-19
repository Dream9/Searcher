/*************************************************************************
	> File Name: link4sefile.cpp
	> Author: Dream9
	> Brief: 
	> Created Time: 2019年05月11日 星期六 17时13分36秒
 ************************************************************************/
#include"link4sefile.h"

using std::vector;
using std::unordered_map;

#define _TIME_LEN_ 128
extern unordered_map<string,string>mapCacheHostLookup;//dns缓存
//extern vector<string> vsParsedLinks;//弃用
extern const string kVERSION_LINK4SE;//版本号

//routine

Link4SEFile::Link4SEFile(){}

Link4SEFile::Link4SEFile(const string &str):FileEngine(str){}

Link4SEFile::~Link4SEFile(){
    m_ofsFile.close();
}

///brief:解析过的Url存储
//parameter:file_arg结构，记录两个指针，更多信息查阅FileEngine
//Becare:fstream不是线程安全的，需要自己做同步
//       要求arg中的ptrPage已经调用过ParseHyperLink()
int Link4SEFile::Write(void *arg){
	if(!arg || !m_ofsFile){
		err_msg("No data or bad ofstream");
		return -1;
	}

	file_arg *ptrFile=(file_arg*)arg;
	Url *ptrUrl=ptrFile->ptrUrl;
	Page *ptrPage=ptrFile->ptrPage;
    
	if(ptrPage->m_mapLink4SE.empty()){
		err_msg("No parsedLinks to write");
		return -2;
	}
	char strTime[_TIME_LEN_];
	time_t tDate;

	//获取时间，GMT时间
	memset(strTime,0,_TIME_LEN_);
	time(&tDate);
	strftime(strTime,_TIME_LEN_,"%a, %d %b %Y %H:%M:%S GMT",gmtime(&tDate));

	//记录所有链接
	string links;
    for(auto cur:ptrPage->m_mapLink4SE){
		links+=cur.first+"\n";
	}

	/////////////////////////////////////////////////
	/////////////////////////////////////////////////
	///////////需要做同步，或者外部对文件加锁////////
	/////////////////////////////////////////////////
	/////////////////////////////////////////////////
	m_ofsFile<<"version:"<<kVERSION_LINK4SE<<'\n';
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
	m_ofsFile<<"\noutdegree:"<<ptrPage->m_mapLink4SE.size();
    m_ofsFile<<"\nlength:"<<links.size()+ptrPage->m_nLenHeader+1;

	//空行分割，写入头部和链接信息
	m_ofsFile<<"\n\n"<<ptrPage->m_sHeader<<"\n";
	m_ofsFile<<links;
	m_ofsFile<<std::endl;

	return 0;
}

#undef _TIME_LEN_
