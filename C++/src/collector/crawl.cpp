/*************************************************************************
	> File Name: crawl.cpp
	> Author: Dream9
	> Brief: 
	> Created Time: 2019年05月12日 星期日 19时20分08秒
 ************************************************************************/

#include"crawl.h"
#include"crawl_config.h"
//todo
//#include<Mylist.h>//双向链表
//#include<Myhlink.h>//
//#include<Myuri.h>

using std::unique_lock;
using std::mutex;

extern const string kREPLICAS_FILE_NAME;//replicas file//保存page相似哈希和url
extern const string kUNVISITEDURL_FILE_NAME;
extern const string kWORK_DIRECTORY;//工作路径
extern const vector<string> kHTTP_PROTOCOLS;
extern const string kDATA_FORMAT_FILE_PREFIX;
extern const string kDATA_LINK4SE_FILE_PREFIX;
extern int isHandleImage;//是否处理img
extern const vector<string> kPLAIN_TEXT_TYPE;
extern const int kGZIP_LEN;
extern const string kURLMD5_FILE_NAME;
extern const string kPAGEMD5_FILE_NAME;///////注意：todo:要改成相似哈希的实现
extern const string kIPBLOCK_FILE_NAME;
extern const string kUNREACHHOST_FILE_NAME;
extern const string kISAM_INDEX_FILE_NAME;
extern const string kISAM_DATA_FILE_NAME;
extern const string kLINK4SE_FILE_NAME;
extern const string kLINK4HISTORY_FILE_NAME;

//各项数据访问的全局锁
mutex mutexVisitedPageMd5;// mutexReplicas;
//mutex mutexReplicas;
mutex mutexMapUrl;
mutex mutexLink4HistotyFile;
mutex mutexVisitedUrlMd5;
mutex mutexIsamFile;
mutex mutexVisitedUrlFile;
mutex mutexUnreachHost;
mutex mutexLink4SEFile;
mutex mutexMemory;

//multimap<string, string>mapUrl;//host-url//下一步的搜索link//错误的设计
map<string, string>mapUrl;//key:url value:host //存host的目的,意义不大

//unordered_multimap<string, string>replicas;//md5-url,
unordered_multimap<string, string>mapVisitedPageMd5;//md5-url,//取代setVisitedPageMd5，因此m_ofsVisitedPageMd5File负责导出该项数据
//unordered_set<string>setVisitedPageMd5;//////md5,这两者的设计也存在问题，重复存储了page的相似哈希值//但本项可以序列化记录

unordered_set<string>setVisitedUrlMd5;
map<uint32_t, uint32_t>mapIpBlock;//网络号-子网掩码的取反值,需要遍历，不适合hashtable
unordered_set<string>setUnreachHostMd5;


//错误信息
static const int kERROR_COLLECT=-1;
static const int kERROR_COLLECT_MEMORY=-2;
static const int kERROR_COLLECT_SYS=-3;
static const int kERROR_COLLECT_CONFIG = -4;
static const int kERROR_COLLECT_LINK = -5;
static const int kERROR_COLLECT_REPEATMD5 = -6;
static const int kERROR_CRITICAL = -999;


static const int kMAXFILELINEBUF = 2048;

static int _SaveReplicas(const char *);//保存重复页面信息
static void _SaveUnvisitedUrl();//保存尚未访问的link
void _start(void *);

//brief:捕捉相关信号，做退出前的清理工作
//主要针对：SIGQUIT，SIGTERM，SIGINT
//注意SIGKILL和SIGSTOP不可捕捉
void _SigHandle_quit(int signum){
	log_msg("Prepare to quit ...");
	//if(0!=SaveUnvisitedUrl()){
	//	err_msg("SaveUnvisitedUrl failed");
	//}
	_SaveUnvisitedUrl();
	if(0!=_SaveReplicas(kREPLICAS_FILE_NAME.c_str())){
		err_msg("SaveReplicas failed:%s",kREPLICAS_FILE_NAME);
	}
	log_msg("Shutdown");

	exit(EXIT_FAILURE);
}

//brief:保存未访问网页
//becare:mapUrl的结构为url-host
//       必须通过截断打开
void _SaveUnvisitedUrl(){
	ofstream ofsUnvisitedUrl;

	//注意通过截断打开
	ofsUnvisitedUrl.open(kUNVISITEDURL_FILE_NAME.c_str(),  ios::in | ios::binary | ios::trunc);
	if (!ofsUnvisitedUrl) {
		err_ret("Can't access :%s",kUNVISITEDURL_FILE_NAME);
		exit(EXIT_FAILURE);
	}

	for (auto cur : mapUrl) {
		ofsUnvisitedUrl << cur.first.c_str() << "\n";
	}

	ofsUnvisitedUrl << endl;
	ofsUnvisitedUrl.close();
}

//brief:保存网页内容重复的Url
//Becare:这里是通过Md5值来确定重复与否的，所有记录之间换行分隔
//       格式为page_md5+'\n'+url1+'\n+url2+'\n'....
//todo:相似性判断，基本一致的也应该记录
int _SaveReplicas(const char *file) {
	ofstream ofsReplicas;
	stringstream ss;
	string prevMd5;
	int count;

	//std::unique_lock<std::mutex> uqlock(mutexReplicas);
	std::unique_lock<std::mutex> uqlock(mutexVisitedPageMd5);
	ofsReplicas.open(file, ios::out | ios::binary | ios::app);

	if (!ofsReplicas) {
		err_ret("Can't access file :%s", file);
		return -1;
	}

	for (auto cur : mapVisitedPageMd5) {
		if (cur.second == "") {
			continue;
		}
		if (prevMd5.empty()) {
			prevMd5 = cur.first;
			count = 0;
		}
		else if (prevMd5 != cur.first) {
			if (count > 1) {
				//有重复
				ofsReplicas << ss.str();
			}
			prevMd5 = cur.first;
			ss.clear();
			ss.str(prevMd5);
			ss << "\n";
			count = 0;
		}
		ss << cur.second<<"\n";

		++count;
	}

	//处理一项特殊情况
	if (count > 1) {
		ofsReplicas << ss.str();
	}
	return 0;
}

//brief:转调接口
void Crawl::SaveUnvisitedUrl() {
	return _SaveUnvisitedUrl();
}

//brief:转调接口
int Crawl::SaveReplicas(const char *file) {
	return _SaveReplicas(file);
}

//brief:读取初始URL信息,同时填充协议名称（缺省时，默认http）
//return:没有正常到达文件尾时返回错误信息
int Crawl::_get_original_url_from_file(FILE* url_file) {
	if(!url_file){
		return 0;
	}
	//char buf[kMAX_URL_REFERENCES];
	Page page_parser;
	size_t pos;
	string buf;
	buf.resize(kMAX_URL_REFERENCES);

	//linux下不应该出现\r\n结尾，，，
	while(fgets(&buf[0],kMAX_URL_REFERENCES,url_file)){
		if (buf[0] == '\0' || buf[0] == '\n' || buf[0]=='\r'  || buf[0] == '#') {
			continue;
		}
		
		string urlbuf=buf.c_str();
		if(urlbuf.back()!='\n'){
			return kERROR_COLLECT;
		}
		urlbuf.pop_back();
		//http,https
		if (string::npos == StrFun::FindCase(urlbuf, "http")) {
			pos = urlbuf.find('\t');
			if (string::npos == urlbuf.find('/')) {
				urlbuf = string::npos == pos ? "http://" + urlbuf + "/" : "http://" +urlbuf.substr(pos) + "/";
			}
			else {
				urlbuf = string::npos == pos ? "http://" + urlbuf : "http://" + urlbuf.substr(pos);
			}
		}
		if (page_parser.IsFilter(urlbuf)) {
			continue;
		}
		AddUrl(urlbuf.c_str());
		buf[0]='\0';
	}
	//
	if (ferror(url_file)) {
		err_msg("Unexcepted error when fgets()");
		return - 1;
	}
	return 0;
}


//brief:收集程序的主入口，读入外部信息，多个线程运行，结果导出，异常终止时记录相关信息
int Crawl::Collect() {
	//信号函数处理
	auto old_sig_term = signal(SIGTERM, _SigHandle_quit);
	auto old_sig_int = signal(SIGINT, _SigHandle_quit);
	auto old_sig_quit = signal(SIGQUIT, _SigHandle_quit);
	auto old_sig_pipe = signal(SIGPIPE, SIG_IGN);
	auto old_sig_chld = signal(SIGCHLD, SIG_IGN);

	std::vector<thread> thread_pool;
	char strTime[128];
	time_t tDate;
	FILE * ifsFile;
	//ifstream ifsFile;
	int ret = 0;
	string strUrl;

	//记录开始时间
	memset(strTime, 0, sizeof strTime);
	time(&tDate);
	strftime(strTime, sizeof strTime, "%a, %d %b %Y %H:%M:%S GMT", gmtime(&tDate));
	log_msg("\n\nStart to collect data at %s", strTime);

	//读入外部配置信息
	//调整工作目录位置
	if ((ret = chdir(kWORK_DIRECTORY.c_str())) < 0) {
		err_msg("chdir() failed");
		goto ERROR_SIG;
	}
	if ((ret = GetVisitedPageMd5())!=0) {
		err_msg("GetVisitedPageMd5() failed");
		ret=kERROR_COLLECT_CONFIG;
		goto ERROR_SIG;
	}
	if ((ret = GetVisitedUrlMd5())!=0) {
		ret=kERROR_COLLECT_CONFIG;
		err_msg("GetVisitedUrlMd5() failed");
		goto ERROR_SIG;
	}
	if ((ret = GetIpBlock())!=0) {
		ret=kERROR_COLLECT_CONFIG;
		err_msg("GetIpBlock() failed");
		goto ERROR_SIG;
	}
	if ((ret = GetUnreachHostMd5())!=0) {
		ret=kERROR_COLLECT_CONFIG;
		err_msg("GetUnreachHostMd5() failed");
		goto ERROR_SIG;
	}

	//从两个文件中加载初始link
	//从初始seed
	ifsFile=fopen(m_sInputFileName.c_str(), "rb");
	if (!ifsFile) {
		err_msg("Can't access %s", m_sInputFileName.c_str());
	}
	if ((ret= _get_original_url_from_file(ifsFile))!=0) {
		ret=kERROR_COLLECT_CONFIG;
		goto ERROR_SIG;
	}
	else if(ifsFile){
		fclose(ifsFile);
	}
	//从为访问的文件中
	ifsFile = fopen(kUNVISITEDURL_FILE_NAME.c_str(), "rb");
	if (!ifsFile) {
		err_msg("Can't access %s", kUNVISITEDURL_FILE_NAME.c_str());
	}
	if ((ret= _get_original_url_from_file(ifsFile))!=0) {
		ret=kERROR_COLLECT_CONFIG;
		goto ERROR_SIG;
	}
	else if(ifsFile){
		fclose(ifsFile);
	}

	if(mapUrl.empty()){
		err_msg("Fatal Error:Empty Link Pool");
		ret=kERROR_COLLECT_CONFIG;
		goto ERROR_SIG;
	}


	if ((ret = OpenFilesForOutput()) != 0) {
		err_msg("OpenFilesForOutput()failed");
		goto ERROR_SIG;
	}
#ifndef DEBUG_TEST_SINGTHREAD_	
	//创建工作线程
	for (int i = 0; i < m_nThreadNums; ++i) {
		log_msg("%dth thread ready",i);
		thread_pool.emplace_back(_start,this);
	}

	//等待
	for_each(thread_pool.begin(), thread_pool.end(), mem_fn(&std::thread::join));

#else
	//测试
	_start(this);
#endif
	//清理工作
	log_msg("End.");
	SaveReplicas(kREPLICAS_FILE_NAME.c_str());
	SaveUnvisitedUrl();

	memset(strTime, 0, sizeof strTime);
	time(&tDate);
	strftime(strTime,128, "%a, %d %b %Y %H:%M:%S GMT", gmtime(&tDate));
	log_msg("\n\nEND at %s", strTime);
	ret = 0;

	//恢复信号处理
ERROR_SIG:
	if (SIG_ERR == signal(SIGTERM, old_sig_term)) {
		err_ret("signal(SIGTYERM)failed");
	}
	if (SIG_ERR == signal(SIGINT, old_sig_int)) {
		err_ret("signal(SIFINT)failed");
	}
	if (SIG_ERR == signal(SIGQUIT, old_sig_quit)) {
		err_ret("signal(SIGQUIT)failed");
	}
	if (SIG_ERR == signal(SIGPIPE, old_sig_pipe)) {
		err_ret("signal(SIGPIPE)failed");
	}
	if (SIG_ERR == signal(SIGCHLD, old_sig_chld)) {
		err_ret("signal(SIGCHLD)failed");
	}
	
	return ret;

}

Crawl::Crawl(){}

//brief:默认为10个线程数目
Crawl::Crawl(const string &strInFile, const string &strOutFile, int number_thread = kDefaultThread,
		     int number_page = kDefaultPageEachThread, int time_thread = kDefaultThreadRunTime) {
	m_sInputFileName = strInFile;
	m_sOutputFileName = strOutFile;
	m_nThreadNums = number_thread;
	m_nRunTimeThread = time_thread;
	m_nPageEachThread = number_page;
}

Crawl::~Crawl() {
	fclose(m_ofsLink4SEFile);
	fclose(m_ofsLink4HistoryFile);
	fclose(m_ofsVisitedUrlFile);
	fclose(m_ofsVisitedUrlMd5File);
	fclose(m_ofsVistitedPageMd5File);
}

//brief:工作线程
void _start(void *arg) {
	int ret;
	ret=((Crawl*)arg)->fetch(arg);
	if (0 != ret) {
		err_msg("Some errors :%d", ret);
	}
	return;
}

//brief:数据获取
//becare:每个线程单独建立FormatFile数据文件,命名为：前缀+时间戳+thread::id
//      由于每个线程都有自己的文件，因此这两个文件不需要做同步
//      对同一个网站的连接，不主动关闭连接
//      全部采用http协议通信，避免ssl握手
//return:错误信息，空转200次之后将会跳出循环
//todo:
//用list<pair<string,string>>记录
//深度记录
//当前复用的套接字是紧跟的两个套接字如果一样则复用，可以保留多个套接字
int Crawl::fetch(void *arg) {
	string strFormatFile;
	string strLink4SEFile;
	string strUrl;
	stringstream ss;
	time_t curTime;
	int ret=0;
	string prev_host = "";
	int prev_sockfd = -1;
	int empty_count = 0;
	int page_count = 0;

	std::unique_lock<std::mutex> visitedUrlMd5_lock(mutexVisitedUrlMd5, std::defer_lock);

	time(&curTime);
	ss << kDATA_FORMAT_FILE_PREFIX << "." << curTime << (this_thread::get_id());
	ss >> strFormatFile;
	ss.clear();//使流恢复有效位
	FormatFile rawformatfile(strFormatFile);
	ss << kDATA_LINK4SE_FILE_PREFIX << "." << curTime << (this_thread::get_id());
	ss >> strLink4SEFile;
	Link4SEFile rawlink4sefile(strLink4SEFile);

	std::unique_lock<std::mutex> url_lock(mutexMapUrl,std::defer_lock);
	while (empty_count<200 && ret!=kERROR_CRITICAL) {
		url_lock.lock();
		if (mapUrl.empty()) {
			empty_count++;
			url_lock.unlock();
			this_thread::sleep_for(chrono::seconds(600));
			continue;
		}
		auto iter = mapUrl.begin();
		strUrl = iter->first;
		log_msg("ready for %s",strUrl.c_str());
		////////////////////////
		//注意，确保采用的是http协议
		StrFun::ReplaceStr(strUrl, "https", "http");
		////////////////////////
		mapUrl.erase(iter);
		url_lock.unlock();
#ifdef DEBUG_1_
		log_debug("%s:Out of Queue", strUrl);
#endif
		//解析Url
	    Url url_parser;
		if (false == url_parser.ParseUrl(strUrl)) {
#ifdef DEBUG_1_
			log_debug("bad url");
#endif
			continue;
		}

		//先检查是否重复
		Md5 md5_parser;
		md5_parser.GenerateMd5((uint8_t*)url_parser.m_sUrl.c_str(), url_parser.m_sUrl.size());
		string strDigest(md5_parser.ToString());
		visitedUrlMd5_lock.lock();
		if (setVisitedUrlMd5.find(strDigest) != setVisitedUrlMd5.end()) {
			log_msg("Repeat UrlMd5:%s", strDigest);
			visitedUrlMd5_lock.unlock();
			continue;
		}
		visitedUrlMd5_lock.unlock();
		
		//是否续用之前的端口
		if (prev_host != url_parser.m_sHost) {
			prev_host = url_parser.m_sHost;
			if (prev_sockfd > 0) {
				close(prev_sockfd);
				prev_sockfd = -1;
			}
		}

		//发生致命错误时会退出循环
		ret=((Crawl*)arg)->DownloadFile(&rawformatfile, &rawlink4sefile, url_parser, prev_sockfd);
		page_count++;
		if(page_count>m_nPageEachThread){
			break;
		}
#ifdef DEBUG_TEST_SINGTHREAD_
		log_msg("ret=%d", ret);
		if(page_count>=10){
			break;
		}
#endif
	}

	log_msg("Collect %d pages",page_count);
	rawformatfile.Close();
	rawlink4sefile.Close();
	return ret;
}

//brief:处理gzip的辅助函数,创建一个.gz文件，然后解析之
//becare:需要外部库gzip,注意是apt-get install gzip1g-dev，没有gzip的包
//return:错误信息，并且解码结果通过引用返回
int _DealWithGzip(const string &str_content, string &out) {
	char UnzipContent[kGZIP_LEN];

	gzFile gzip;
    string gzipname;
	stringstream ss;
	int len;

	ss << this_thread::get_id() << ".gz";
	ss >> gzipname;
	FILE *file = fopen(gzipname.c_str(), "wb");
	fwrite(str_content.c_str(), 1, str_content.size(), file);
	fclose(file);

	gzip = gzopen(gzipname.c_str(), "rb");
	if (!gzip) {
		err_msg("gzopen() failed");
		return kERROR_CRITICAL;
	}
	len = gzread(gzip, UnzipContent, kGZIP_LEN);
	if (-1 == len) {
		err_msg("gzread() failed");
		return kERROR_CRITICAL;
	}
	UnzipContent[len] = '\0';
	out = UnzipContent;
	gzclose(gzip);
	return len;
}

//brief:辅助函数，网页补全功能
//针对相对路径的link,结合url已经分析出来的域名，端口等信息补全网页link
static int WebComplete(string &str_web,const Url &url_parser,const string &m_sUrl) {
	//url_parser
	if (str_web[0] == '/') {
		//形如  /index.html
		//相对路径,使用特殊端口的时候需要带上端口号
		str_web = (url_parser.m_nPort != kDEFAULT_HTTP_PORT) ? "http://" + url_parser.m_sHost + ":" + std::to_string(url_parser.m_nPort) + str_web
			:"http://" + url_parser.m_sHost + str_web ;
	}
	else if (str_web[0] != '/' && m_sUrl.back() != '/') {
		//形如  ../index.html,同时m_sUrl不以/结尾，需要退回到父目录
		auto pos_parent_directory = m_sUrl.rfind('/');
		if (string::npos != pos_parent_directory) {
			str_web = pos_parent_directory > sizeof "http://" - 1 ? m_sUrl.substr(0, pos_parent_directory + 1) + str_web
				: m_sUrl + "/" + str_web;
		}
		else {
			return -1;
		}
	}
	else {
		//
		str_web = m_sUrl.back() == '/' ? m_sUrl + str_web : m_sUrl + "/" + str_web;
	}
	return 0;
}

//brief:处理重定向的辅助函数
//return:错误信息
//becare:重定向次数超过阈值3将会出错
//todo:部分网站从http强制掉转到https,因此必须实现ssl握手通信
int _DealWithHTTPLoaction(char *strlocation, const Url &url_parser, char *body, char *header,int &status,
	                      Http &http_collector,int &sock) {
	if (!strlocation) {
		return kERROR_COLLECT_LINK;
	}
	int count = 0;

	while (count < 3) {
	    string strUrl = strlocation;
#ifdef DEBUG_L
		std::unique_lock<std::mutex> tmp_lock(mutexMemory);
#endif
		free(strlocation);
#ifdef DEBUG_L
		tmp_lock.unlock();
#endif
		strlocation=nullptr;
		if (strUrl.empty() || strUrl.size() > kURL_LEN - 1) {
			return kERROR_COLLECT_LINK;
		}
		if (0 != StrFun::FindCase(strUrl, "http")) {
			if (0 != WebComplete(strUrl, url_parser, url_parser.m_sHost)) {
				return kERROR_COLLECT_LINK;
			}
		}
		Page page_parser;
		if (page_parser.IsFilter(strUrl)) {
			return kERROR_COLLECT_LINK;
		}
		status = http_collector.Fetch(strUrl, &body, &header, &strlocation, &sock);
		if (status == kHTTP_LOCATION) {
			count++;
		}
		else {
			return status;
		}
	}
	return kERROR_COLLECT_LINK;
}
//brief:获取响应，处理异常，解析响应
//becare:释放资源
int Crawl::DownloadFile(FormatFile *ptrFormatFile, Link4SEFile *ptrLink4SEFile, Url &url_parser, int &sockfd) {
	//
	int response;
	string strunzipstring;
	string strLocation;
	char *body = nullptr;
	char *header = nullptr;
	char *location = nullptr;
	int sock = sockfd;
	Http http_collector;
	Page page_parser;
	int ret = 0;
	bool fliter_flag = true;
    Md5 md5_parser;
	string strDigest;
    std::unique_lock<std::mutex> visitedUrlMd5_lock(mutexVisitedUrlMd5,std::defer_lock);


	response = http_collector.Fetch(url_parser.m_sUrl, &body, &header, &location, &sock);

	//处理异常情况
	//特别的是重定向错误，需要重新fetch
	if (response == kHTTP_LOCATION) {
		ret = _DealWithHTTPLoaction(location, url_parser, body, header, response, http_collector, sock);
	}
	sockfd = sock;
	if (ret == kERROR_COLLECT_LINK) {
		goto ERROR_BUF;
	}
	//其他错误，注意只有重定向错误会多次重新fetch
	switch (response) {
	case kHTTP_ERROR:
		//err_msg("page error");
		goto ERROR_BUF;

	case kHTTP_OUTOFRANGE:
		SaveUnreachHost(url_parser.m_sHost);
		goto ERROR_BUF;

	case kHTTP_INVALIDHOST:
		//err_msg("invalidhost");
		goto ERROR_BUF;

	case kHTTP_IMG:
		//err_msg("img page");
		if (m_ofsLink4HistoryFile) {
			std::unique_lock<std::mutex> tmp_lock(mutexLink4HistotyFile);
			fputs(url_parser.m_sUrl.c_str(), m_ofsLink4HistoryFile);
			fputc('\n', m_ofsLink4HistoryFile);
			tmp_lock.unlock();
		}
		goto ERROR_BUF;

	default:
		if (response < 0) {
			err_msg("Unexcepted error");
			goto ERROR_BUF;
		}
		//正常时
		break;
	}
	if(location){
		strLocation = location;
	}

	//解析response
	if (!body || !header) {
		ret = kERROR_COLLECT_LINK;

		goto ERROR_BUF;
	}
	page_parser.SetPage(url_parser.m_sUrl, strLocation, header, body, response);
	page_parser.ParseHeaderInfo(page_parser.m_sHeader);
	if (page_parser.m_bConnectionState == false) {
		close(sockfd);
		sockfd = -1;
	}

	//检查网页内容
	if (isHandleImage == 0) {
		//不处理则需要过滤
		for (string cur : kPLAIN_TEXT_TYPE) {
			if (page_parser.m_sContentType == cur) {
				fliter_flag = false;
				break;
			}
		}
		if (fliter_flag) {
			err_msg("Unexcepted content type:%s", page_parser.m_sContentType);
			goto ERROR_BUF;
		}
	}

	//针对gzip解码
	//需要zlib库
	if (page_parser.m_sContentEncoding == "gzip" && page_parser.m_sContentType=="text/html") {
		if ((ret = (_DealWithGzip(page_parser.m_sContent, strunzipstring))) < 0) {
			goto ERROR_BUF;
		}
	}

    //提取数字摘要，相似哈希
	//todo:相似哈希
	//todo:实际上只有location了之后才有可能重复到
	md5_parser.GenerateMd5((uint8_t*)url_parser.m_sUrl.c_str(), url_parser.m_sUrl.size());
	strDigest = md5_parser.ToString();
	visitedUrlMd5_lock.lock();
	if (setVisitedUrlMd5.insert(strDigest).second == false) {
		visitedUrlMd5_lock.unlock();
		ret = kERROR_COLLECT_REPEATMD5;
		goto ERROR_BUF;
	}
	SaveVisitedUrlMd5(strDigest);
	visitedUrlMd5_lock.unlock();

	////////////////////////////////////
	////////////////////////////////////
	////////////////////////////////////
	////////////////////////////////////
	//////////todo:相似哈希/////////////
	////////////////////////////////////
	////////////////////////////////////
	////////////////////////////////////
	////////////////////////////////////
	////////////////////////////////////
	//Raw信息记录
	SaveFormatFile(ptrFormatFile, &url_parser, &page_parser);
	//todo:是否有必要？？
	//SaveIsamFile(&url_parser, &page_parser);

	//Url记录
	//if (page_parser.m_sLocation.empty()) {
	//	std::unique_lock<mutex> tmp_lock(mutexVisitedUrlMd5);
	//	SaveVisitedUrlMd5(url_parser.m_sUrl);
	//}
	//else {
	//	std::unique_lock<mutex> tmp_lock(mutexVisitedUrlMd5);
	//	SaveVisitedUrlMd5(page_parser.m_sLocation);
	//}

	//提取下一步的Url
	//todo:深度记录
	if (page_parser.m_sContentType != "text/html") {
		//无法提取link
		ret = 0;
		goto ERROR_BUF;
	}
	page_parser.ParseHyperLinks();
	if (0 != (ret=SaveLink4SE(&page_parser))) {
		goto ERROR_BUF;
	}
	if (0 != (ret=SaveLink4History(&page_parser))) {
		goto ERROR_BUF;
	}
	log_msg("link extract finished");
	ret = 0;

	//资源释放
ERROR_BUF:
#ifdef DEBUG_L
	std::unique_lock<std::mutex> tmp_lock(mutexMemory);
#endif
	if (body) {
		free(body);
		body=nullptr;
	}
	if (header) {
		free(header);
		header=nullptr;
	}
	if (location) {
		free(location);
		location=nullptr;
#ifdef DEBUG_L
		tmp_lock.unlock();
#endif
	}
	return ret;
}
//////////////////////////
//////////////////////////
//////////////////////////
//以上为主流程实现
//以下为各个过程函数的实现
//////////////////////////
//////////////////////////
//////////////////////////

//brief:GetVisitedUrlMd5的辅助函数
//becare:文件不存在不是错误
//Md5值理论上讲应该小于128位
static int _GetVisitedInfoMd5(const string &filename, unordered_set<string> &set_rec) {
	FILE *ifs = fopen(filename.c_str(), "rb");
	if (!ifs) {
		log_msg("No such file:%s", filename.c_str());
		return 0;
	}

	char buf[kMAXFILELINEBUF];
	memset(buf, 0, kMAXFILELINEBUF);

//	//getline在gnuc编译器下的扩展
//#if defined(__GNUC__)
//	int len_read = 0;
//	while ((len_read = getline(&buf, &kMAXFILELINEBUF, ifs)) != -1) {
//		//if (len_read > 128) {
//		//	;
//		//}
//		set_rec.insert(buf);
//}
//#else
//	//不是gnuc编译器时
	while (fgets(buf, kMAXFILELINEBUF, ifs)) {
		size_t len = strlen(buf);
		if (buf[len - 1] != '\n') {
			//这么大说明文件有问题
			fclose(ifs);
			return kERROR_COLLECT_CONFIG;
		}
		buf[len - 1] = '\0';
		set_rec.insert(buf);
	}
//#endif

	//没有正常到达文件eof
	if (ferror(ifs)) {
		fclose(ifs);
		return kERROR_COLLECT_SYS;
	}
	fclose(ifs);
	return 0;

}

//brief:GetVisitedPageMd5的辅助函数
//becare:文件不存在不是错误
static int _GetVisitedInfoMd5(const string &filename, unordered_multimap<string,string> &set_rec) {
	FILE *ifs = fopen(filename.c_str(), "rb");
	if (!ifs) {
		log_msg("No such file:%s", filename.c_str());
		return 0;
	}

	char buf[512];
	char buf_md5[128];
	char buf_url[256];
	memset(buf, 0, sizeof buf);
	memset(buf_md5, 0, sizeof buf_md5);
	memset(buf_url, 0, sizeof buf_url);

	while (fgets(buf, kMAXFILELINEBUF, ifs)) {
		size_t len = strlen(buf);
		if (buf[len - 1] != '\n') {
			//这么大说明文件有问题
			fclose(ifs);
			return kERROR_COLLECT_CONFIG;
		}
		sscanf(buf, "%128s %256s", buf_md5, buf_url);
		set_rec.insert(make_pair(buf_md5,buf_url));
	}

	//没有正常到达文件eof
	if (ferror(ifs)) {
		fclose(ifs);
		return kERROR_COLLECT_SYS;
	}
	fclose(ifs);
	return 0;

}
//brief:从配置指定的文件中读入urlMd5数据
int Crawl::GetVisitedUrlMd5() {
	return _GetVisitedInfoMd5(kURLMD5_FILE_NAME, setVisitedUrlMd5);
}

//brief:从配置指定的文件中读取pageMd5信息
int Crawl::GetVisitedPageMd5() {
	return _GetVisitedInfoMd5(kPAGEMD5_FILE_NAME, mapVisitedPageMd5);
}

//brief:从配置指定的文件中读取允许访问的网段地址，记录的时网络号+子网掩码的取反值（即主机掩码）
//becare:这是设计上的一个失误，导致每次还要取反
//       文件格式形如：  "211.71.54.11	0.0.0.0"，#表示注释
int Crawl::GetIpBlock() {
	FILE *ifs = fopen(kIPBLOCK_FILE_NAME.c_str(), "rb");
	if (!ifs) {
		log_msg("No such file:%s", kIPBLOCK_FILE_NAME.c_str());
		return 0;
	}
	char buf[kMAXFILELINEBUF];
	//最长为255.255.255.255
	char netip[16];
	char maskip[16];
	while (fgets(buf, kMAXFILELINEBUF, ifs)) {
        if (buf[0] == '\0' || buf[0] == '\n' || buf[0] == '#') {
			continue;
        }
		memset(netip, 0, sizeof netip);
		memset(maskip, 0, sizeof maskip);
		//注意最大长度为15
		sscanf(buf, "%15s %15s", netip, maskip);
		in_addr net_inaddr;
		in_addr mask_inaddr;
		if (0 == inet_aton(netip, &net_inaddr) || 0 == inet_aton(maskip, &mask_inaddr)) {
			log_msg("inet_aton() failed when get IPBlock,%s,%s", netip, maskip);
			continue;
		}
		uint32_t net_32t=*(uint32_t*)(&net_inaddr);
		uint32_t mask_32t=*(uint32_t*)(&mask_inaddr);
		mapIpBlock[net_32t] = mask_32t;

	}
	if (ferror(ifs)) {
		fclose(ifs);
		err_msg("Not EOF");
		return kERROR_COLLECT;
	}
	fclose(ifs);
	return 0;
}

//brief:从配置指定的文件中读取不可打的域名
//becare:需要转换为Md5,然后记录
int Crawl::GetUnreachHostMd5() {
    FILE *ifs = fopen(kUNREACHHOST_FILE_NAME.c_str(), "rb");
	if (!ifs) {
		log_msg("No such file:%s", kUNREACHHOST_FILE_NAME.c_str());
		return 0;
	}
	char buf[kMAXFILELINEBUF];
	while (fgets(buf, kMAXFILELINEBUF, ifs)) {
        if (buf[0] == '\0' || buf[0] == '\n' || buf[0] == '\r'  || buf[0] == '#') {
			continue;
        }
		//提取一行，注意去除\n
		string host;
		host = buf;
		if (host.back() != '\n') {
			err_msg("Invalid file format");
			return kERROR_COLLECT_CONFIG;
		}
		host.pop_back();
		StrFun::Str2Lower(host, host.size());

		Md5 md5_parser;
		md5_parser.GenerateMd5((uint8_t*)host.c_str(), host.size());

		setUnreachHostMd5.insert(md5_parser.ToString());
	}

	if (ferror(ifs)) {
		fclose(ifs);
		return kERROR_COLLECT;
	}
	fclose(ifs);
	return 0;
}

//brief:保存数据的辅助函数
static int _SaveFile(FileEngine *ptrFile, Url *url_parser, Page *page_parser) {
	if (!ptrFile || !url_parser || !page_parser) {
		return 0;
	}
	file_arg arg;
	arg.ptrUrl = url_parser;
	arg.ptrPage = page_parser;
	if (ptrFile->Write((void*)&arg) != 0) {
		return kERROR_COLLECT;
	}
	return 0;
}

//brief:保存原始网页数据，转调
//parameter:文件句柄，相关的url和page的parser
int Crawl::SaveFormatFile(FormatFile *ptrFormatFile, Url *url_parser, Page *page_parser) {
	return _SaveFile(ptrFormatFile, url_parser, page_parser);
}

//brief:保存Link数据，转调
//parameter:文件句柄，相关的url和page的parser
int Crawl::SaveLink4SEFile(Link4SEFile *ptrLink4SEFile, Url *url_parser, Page *page_parser) {
	return _SaveFile(ptrLink4SEFile, url_parser, page_parser);
}

//brief:按照ISAM方式记录数据
//底层应该采用b+tree
//becare:这个文件是线程共享的，需要同步
int Crawl::SaveIsamFile(Url *url_parser, Page *page_parser) {
	if (!url_parser || !page_parser) {
		return 0;
	}
	file_arg arg;
	arg.ptrPage = page_parser;
	arg.ptrUrl = url_parser;
	std::unique_lock<std::mutex> lock_isam(mutexIsamFile);
	m_isamFile.Write((void*)&arg);
	return 0;
}

//brief:记录已经访问过的Url
//becare:每记录10240条信息，刷新一次
int Crawl::SaveVisitedUrl(const string & url) {
	static int i = 0;
	if (m_ofsVisitedUrlFile) {
		std::unique_lock<std::mutex> lock_url(mutexVisitedUrlFile);
		fputs(url.c_str(), m_ofsVisitedUrlFile);
		fputc('\n', m_ofsVisitedUrlFile);
		i++;
		if (i == 10240) {
			fflush(m_ofsVisitedUrlFile);
		}
	}
	return 0;
}

//brief:记录不可达的Host,并加入到set中
//becare:10240条记录刷新一次
int Crawl::SaveUnreachHost(const string &host) {
	static int i = 0;
	Md5 md5_parser;
	md5_parser.GenerateMd5((uint8_t*)host.c_str(), host.size());
	if (setUnreachHostMd5.insert(md5_parser.ToString()).second && m_ofsUnreachHostFile) {
		std::unique_lock<std::mutex> lock_host(mutexUnreachHost);
		fputs(host.c_str(), m_ofsUnreachHostFile);
		fputc('\n', m_ofsUnreachHostFile);
		i++;
		if (i == 10240) {
			fflush(m_ofsUnreachHostFile);
			i=0;
		}
	}
	return 0;
}

//brief;把map数据记录出来即可,同时把新的连接加入到下一步的搜索池中
//      文件格式为当前page的Url信息，然后是这个网页中的链接及其说明性的anchor text
//becare:传进来时要求Page对象已经调用过解析函数了
//todo:增加深度记录
int Crawl::SaveLink4SE(Page *page_parser) {
	if (m_ofsLink4SEFile && page_parser->m_nRefLink4SENum > 0) {
		std::unique_lock<std::mutex> lock_link(mutexLink4SEFile);
		fprintf(m_ofsLink4SEFile, "root_url:%s\n"
			"charset:%s\nnumber:%d\n"
			"link-anchor\n", page_parser->m_sUrl.c_str(), page_parser->m_sCharset.c_str(),
			page_parser->m_nRefLink4SENum);
		for (auto cur : page_parser->m_mapLink4SE) {
			fprintf(m_ofsLink4SEFile, "%s\t%s\n", cur.first.c_str(), cur.second.c_str());
		}
		fflush(m_ofsLink4SEFile);
		lock_link.unlock();//减小锁的粒度

		//添加到待搜索
		for (auto cur : page_parser->m_mapLink4SE) {
			this->AddUrl(cur.first.c_str());
		}
	}
	return 0;
}

//brief:与SaveLink4SE相似，用于保存img链接的信息
int Crawl::SaveLink4History(Page *page_parser) {
	if (m_ofsLink4HistoryFile && page_parser->m_nRefLink4HistoryNum > 0) {
		std::unique_lock<std::mutex> lock_link(mutexLink4HistotyFile);
		for (string cur : page_parser->m_vecLink4History) {
			fprintf(m_ofsLink4HistoryFile, "%s\n", cur.c_str());
		}
		fflush(m_ofsLink4HistoryFile);
		lock_link.unlock();//减小锁的粒度
	}
	return 0;
}

//brief:保存访问过的Url Md5,记录结构为 md5+'\n'
int Crawl::SaveVisitedUrlMd5(const string &md5) {
	static int i=0;
	if (m_ofsVisitedUrlMd5File) {
		++i;
		fprintf(m_ofsVisitedUrlMd5File, "%s\n", md5.c_str());
	}
	if(i==10240){
		fflush(m_ofsVisitedUrlMd5File);
		i=0;
	}
	return 0;
}

//brief:保存访问过的age Md5,记录结构为 md5+' '+url+'\n'
//toto:改为相似哈希值
int Crawl::SaveVisitedPageMd5(const string &md5, const string &url) {
	static int i=0;
	if (m_ofsVistitedPageMd5File) {
		++i;
		fprintf(m_ofsVistitedPageMd5File, "%s %s\n", md5.c_str(),url.c_str());
	}
    if(i==10240){
		fflush(m_ofsVisitedUrlMd5File);
		i=0;
	}
	return 0;
}

//brief:初始化各项文件指针
#define ASSERT_ZERO_(x) if(!x){err_msg("fopen() failed:"#x);}
int Crawl::OpenFilesForOutput() {
//  IsamFile m_isamFile;//ISAM存储
//
//    //连接存储
//	FILE * m_ofsLink4SEFile;-
//	FILE * m_ofsLink4SEHistoryFile;-
//
//	//访问过得Url及page的Md5存储
//	FILE * m_ofsVisitedUrlFile;-
//	FILE * m_ofsVisitedUrlMd5File;-
//	FILE * m_ofsVistitedPageMd5File;-
//
//	//无法访问的Url及host存储
//	FILE * m_ofsUnreachHostFile;-
	m_isamFile.Open(kISAM_DATA_FILE_NAME, kISAM_INDEX_FILE_NAME);

	m_ofsLink4SEFile = fopen(kLINK4SE_FILE_NAME.c_str(), "ab");
	ASSERT_ZERO_(m_ofsLink4SEFile);
	m_ofsLink4HistoryFile = fopen(kLINK4HISTORY_FILE_NAME.c_str(), "ab");
	ASSERT_ZERO_(m_ofsLink4HistoryFile);

	m_ofsVisitedUrlFile = fopen(m_sOutputFileName.c_str(), "ab");
	ASSERT_ZERO_(m_ofsVisitedUrlFile);
	m_ofsVisitedUrlMd5File = fopen(kURLMD5_FILE_NAME.c_str(), "ab");
	ASSERT_ZERO_(m_ofsVisitedUrlMd5File);
	m_ofsVistitedPageMd5File = fopen(kPAGEMD5_FILE_NAME.c_str(), "ab");
	ASSERT_ZERO_(m_ofsVistitedPageMd5File);

	m_ofsUnreachHostFile = fopen(kUNREACHHOST_FILE_NAME.c_str(), "ab");
	ASSERT_ZERO_(m_ofsUnreachHostFile);

	return 0;
}
#undef ASSERT_ZERO_

//brief:添加到下一步的搜索池中
//becare:
//todo:调度模式的改进
int Crawl::AddUrl(const char *url) {
	string strUrl=url;
	if (strUrl.size() < kMIN_INFO_LEN) {
		log_msg("Bad Url,too small:%s", url);
		return kERROR_COLLECT_LINK;
	}
	//简化路径
	Page page_parser;
	if (page_parser.NormalizeUrl(strUrl) != 0) {
		log_msg("Bad Url,NormalizeUrl() failed:%s", url);
		return kERROR_COLLECT_LINK;
	}
	//IMAGE网页处理
	Url url_parser;
	if (isHandleImage == 0) {
		if (url_parser.IsImageUrl(strUrl) && m_ofsLink4HistoryFile) {
			std::unique_lock<std::mutex> lock_history(mutexLink4HistotyFile);
			fprintf(m_ofsLink4HistoryFile, "%s\n", strUrl.c_str());
			return 0;
		}
	}
	//其他网页
	if (false == url_parser.ParseUrl(strUrl)) {
		log_msg("Bad url,ParseUrl() failed:%s", strUrl.c_str());
		return kERROR_COLLECT_LINK;
	}
	if (false == url_parser.IsValidHost(url_parser.m_sHost.c_str())) {
		log_msg("Bad url,IsValidHost() failed:%s",url_parser.m_sHost);
		return kERROR_COLLECT_LINK;
	}
	if (url_parser.IsForeignHost(url_parser.m_sHost)) {
		log_msg("Bad url,IsForeignHost() failed:%s", url_parser.m_sHost);
		return kERROR_COLLECT_LINK;
	}

	//可达性
	StrFun::Str2Lower(url_parser.m_sHost, url_parser.m_sHost.size());
	Md5 md5_parser;
	md5_parser.GenerateMd5((uint8_t*)url_parser.m_sHost.c_str(), url_parser.m_sHost.size());
	string strMd5 = md5_parser.ToString();
	if (setUnreachHostMd5.find(strMd5) != setUnreachHostMd5.end()) {
		log_msg("Bad url,UnreachHost");
		return kERROR_COLLECT_LINK;
	}

	//判重
    StrFun::Str2Lower(url_parser.m_sUrl, url_parser.m_sUrl.size());
	md5_parser.GenerateMd5((uint8_t*)url_parser.m_sUrl.c_str(), url_parser.m_sUrl.size());
	strMd5 = md5_parser.ToString();
	if (setVisitedUrlMd5.find(strMd5) != setVisitedUrlMd5.end()) {
		log_msg("Bad url,visitedUrl");
		return kERROR_COLLECT_LINK;
	}
	
	//todo:dfs和bfs结合的调度
	std::unique_lock<std::mutex> lock_mapurl(mutexMapUrl);
	mapUrl.insert(make_pair(url_parser.m_sUrl,url_parser.m_sHost));
	lock_mapurl.unlock();
	//if (ans.second == false) {
	//	log_msg("Repeat Url");
	//}
	return 0;
}
