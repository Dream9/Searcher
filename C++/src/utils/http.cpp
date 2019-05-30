/*************************************************************************
	> File Name: http.cpp
	> Author: Dream9
	> Brief: 
	> Created Time: 2019年05月03日 星期五 21时14分23秒
 ************************************************************************/

#include"http.h"
#include"http_config.h"

//从配置文件中获得
extern const int kMAX_MEMORY_LENGTH;//最大内存配额
extern const int kDEFAULT_TIMEOUT;//超时
extern const string kHTTP_VERSION;//版本
extern const int kHIDE_USER_AGENT;//匿名？
extern const string UA_POOL[];
extern const int kLEN_UA_POOL;
extern const int kHEADER_BUF_SIZE;//
extern int isHandleImage;//是否解析网页类型为image的
extern int kDEFAULT_PAGE_BUFSIZE;
extern int kMAX_PAGE_BUFSIZE;

//用户编辑
extern int Timeout;// = kDEFAULT_TIMEOUT;
extern char *userAgent;//UA标识，如果为空，则在UA pool中循环追加一个

static uint8_t kth_ua = 0;


//brief:建立tcp连接
//return:描述符或则出错信息
//主要在于拼接sockaddr_in所需要的信息，然后进行connect
//becare:释放内存
int Http::CreateSocket(const char *host, int port) {
	int sockfd;
	sockaddr_in sa;
	in_addr inaddr;
	int ret;
	int optval = 1;

	Url url_parser;
	char *ip = url_parser.GetIpByHost(host);
	if (!ip) {
		log_msg("GetIpByHost failed():%s",host);
		return kHTTP_INVALIDHOST;
	}
	if (!url_parser.IsValidIp(ip)) {
		//释放内存
		log_msg("IsValidIp failed():%s", ip);
		ret = kHTTP_OUTOFRANGE;
		goto ERROR_MEMORY;
	}
	if (0 == inet_aton(ip, &inaddr)) {
		//转换失败，说明非法
		log_msg("Invalid IP:%s", ip);
		ret= kHTTP_ERROR;
		goto ERROR_MEMORY;
	}
	//运行到这里，说明是合法连接地址，初始化sockaddr_in结构信息
	memcpy((char*)&sa.sin_addr, (char*)&inaddr, sizeof inaddr);
	sa.sin_family = AF_INET;
	sa.sin_port = htons(port);
	//创建一个描述符
	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
		err_ret("create socket failed");
		return kHTTP_ERROR;
	}
	//开启这个描述符的重用功能
	//又不是服务器，这样子做意义？
	//可能是短时间内可能建立太多连接，提高复用吧
	if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, (char *)&optval, sizeof optval) != 0) {
		err_ret("Reuse failed");
		return kHTTP_ERROR;
	}
	
	//建立连接
	if ((ret = NonbConnect(sockfd, (sockaddr *)&sa, kDEFAULT_TIMEOUT)) < 0 ) {
		err_msg("nonb-connect failed");
		close(sockfd);
		return kHTTP_ERROR;
	}
	//return ret;
	//ret = 0;

	//出错处理
	ERROR_MEMORY:
#ifdef DEBUG_L
	std::unique_lock<std::mutex> tmp_lock(mutexMemory);
#endif
	delete[]ip;
#ifdef DEBUG_L
	tmp_lock.unlock();
#endif	
	ip = nullptr;
	return ret;
}


//brief:非阻塞的连接远程
//parameter：套接字描述符，sockaddr，超时信息
//return:错误信息或者连接成功的sockfd
//利用select进行非阻塞的I/O操作,同时把sockfd设置为NONBLOCK状态
//注意采用了select模型，适合连接数比较少的情况，否则应该采用epoll
int Http::NonbConnect(int sockfd, sockaddr *sa, int timewait) {
	fd_set multiplexing;
	int respose_status;
	timeval time_to_out;
	int flags;

	//设置file status
	if ((flags = fcntl(sockfd, F_GETFL, 0)) == -1) {
		err_ret("%d getfl failed", sockfd);
		return kHTTP_ERROR;
	}
	flags |= O_NONBLOCK;
	if (fcntl(sockfd, F_SETFL, flags) < 0) {
		err_ret("%d setfl failed", sockfd);
		return kHTTP_ERROR;
	}
	//注意这里并没有实现重试
	//todo:增加重连，考虑到指数补偿，以及可迁移性
	if (0 == connect(sockfd, sa, sizeof *sa)) {
		//在非阻塞模式下，不能成功会立刻返回
		flags &= O_NONBLOCK;
		if (fcntl(sockfd, F_SETFL, flags) < 0) {
			err_ret("%d reset to blocking failed", sockfd);
			return kHTTP_ERROR;
		}
		return sockfd;
	}
	//通过select等待连接结果
	FD_ZERO(&multiplexing);
	FD_SET(sockfd, &multiplexing);
	time_to_out.tv_sec = timewait;
	time_to_out.tv_usec = 0;
	//只等待到可以写即可

	//Todo:注意可迁移的连接，应该先关闭当前套接字，然后打开新的，重连
	//
RESTART:
	respose_status = select(sockfd + 1, nullptr, &multiplexing, nullptr, &time_to_out);

	switch (respose_status) {
	case -1:
		//连接失败,但可能是被其他信号中断，这里允许重启被中断的调用,注意部分系统可能更改timeval,变为剩余时间
		if (errno == EINTR) {
			goto RESTART;
		}
		goto ERROR_OUT;
	case 0:
		//超时
		goto ERROR_OUT;
	default:
		break;
	}
	//连接成功
	FD_CLR(sockfd, &multiplexing);
	flags &= ~O_NONBLOCK;
    if (fcntl(sockfd, F_SETFL, flags) < 0) {
		err_ret("%d reset to blocking failed", sockfd);
	}
	return sockfd;

    //错误出口
ERROR_OUT:
	flags &= ~O_NONBLOCK;
	if (fcntl(sockfd, F_SETFL, flags) < 0) {
		err_ret("%d reset to blocking failed", sockfd);
	}
#ifdef DEBUG_1_
	log_msg("%d connect failed", sockfd);
#endif
	return kHTTP_ERROR;
}


//brief:realloc的封装
//parameter:已申请资源,最大长度，还需要长度
//return:错误信息，0为成功
//Todo:如果像vector那样维护三个指针，效率就会比这个高，因为这里采用了strlen计算已使用空间
int Http::CheckBufSize(char **buf, int *len_max, int len_excepted_more) {
    int room_left;
	char *buf_new;

	room_left = *len_max - (strlen(*buf) + 1);
	if (room_left > len_excepted_more) {
		return 0;
	}

	//申请更大的资源，但是有上限
    if (*len_max + len_excepted_more + 1 -room_left> kMAX_MEMORY_LENGTH) {
		err_msg("Excess the Max memory(%d)", kMAX_MEMORY_LENGTH);
		return kHTTP_ERROR;
	}
	len_excepted_more = (len_excepted_more << 1) + *len_max + 1 - room_left;
	len_excepted_more = len_excepted_more > kMAX_MEMORY_LENGTH ? kMAX_MEMORY_LENGTH : len_excepted_more;
#ifdef DEBUG_L
	std::unique_lock<std::mutex> tmp_lock(mutexMemory);
#endif
	buf_new = (char*)realloc(*buf, len_excepted_more);
#ifdef DEBUG_L
	tmp_lock.unlock();
#endif
	if (nullptr == buf_new) {
		err_ret("Can't realloc more memory");
		return kHTTP_ERROR;
	}

	//更新信息
	*buf = buf_new;
	*len_max = len_excepted_more;
	return 0;
}

//brief:realloc的封装,改进版
//parameter:已申请资源的三个位置(参见vector实现)，还需要长度
//return:错误信息，0为成功
int Http::CheckBufSize(char **buf_start, char **buf_finish, char **buf_end_of_storage, int len_excepted_more) {
	char *buf_new;
	int len;
	int len_use;

	//int len = std::distance(*buf_end_of_storage, *buf_start);
	len_use = *buf_finish - *buf_start;
    len = *buf_end_of_storage - *buf_start;
	if (len-len_use > len_excepted_more) {
		return 0;
	}

	
    if (len_use + len_excepted_more + 1 > kMAX_MEMORY_LENGTH) {
		err_msg("Excess the Max memory(%d)", kMAX_MEMORY_LENGTH);
		return kHTTP_ERROR;
	}
	//申请更大的资源，但是有上限
	len_excepted_more = (len_excepted_more << 1) + len_use + 1;
	len_excepted_more = len_excepted_more > kMAX_MEMORY_LENGTH ? kMAX_MEMORY_LENGTH : len_excepted_more;
#ifdef DEBUG_L
	std::unique_lock<std::mutex> tmp_lock(mutexMemory);
#endif
	buf_new = (char*)realloc(*buf_start, len_excepted_more);
#ifdef DEBUG_L
	tmp_lock.unlock();
#endif
	if (nullptr == buf_new) {
		err_ret("Can't realloc more memory");
		return kHTTP_ERROR;
	}

	//更新信息
	*buf_start = buf_new;
	*buf_finish = buf_new + len_use;
	*buf_end_of_storage = buf_new + len_excepted_more;
	return 0;
}


//brief:连接并负责数据请求
//return:错误信息
//parameter:Url,存放数据的各类指针
//becare:对于重定向错误，只有location获得了资源
int Http::Fetch(const string &strUrl, char **fileBuf, char **fileHeaderBuf, char **location, int *nSock) {
	char *requestBuf;
	int sockfd;
	const char *host;
	int port;
	Url url_parser;
	int ret=0;
	int len_tmp;
	int len_write;
	string headerBuf;
	Page page_parser;
	char *pageBuf;
	int len_header;
	std::unique_lock<std::mutex> tmp_lock(mutexMemory,std::defer_lock);


	//解析Url
	if (false == url_parser.ParseUrl(strUrl)) {
#ifdef DEBUG_1_
		log_msg("ParserUrl faild:%s", url);
#endif
		return kHTTP_ERROR;
	}
	host = url_parser.m_sHost.c_str();
	port = url_parser.m_nPort;

	//封装头部信息
	if((len_write= _WrapHeader(strUrl, url_parser,(void**)&requestBuf))<0) {
#ifdef DEBUG_1_
		log_debug("%s:can't form a request header", strUrl);
#endif
		return kHTTP_ERROR;
	}
	
	//建立连接
	if (nSock && *nSock > 0) {
		//使用之前的一个连接
		sockfd = *nSock;
#ifdef DEBUG_1_
		log_debug("Using previous socked :%d", sockfd);
#endif
	}
	else {
		//需要建立新的连接
		sockfd = CreateSocket(host, port);
		if (sockfd < 0) {
			//ret = sockfd;
			ret=kHTTP_ERROR;
			goto ERROR_BUF;
		}
	}

	//发送请求
	if ((len_tmp = RioWrite(sockfd, requestBuf, len_write)) != len_write) {
		ret=kHTTP_ERROR;
		goto ERROR_SOCK;
	}

	//接收响应
	headerBuf.resize(kHEADER_BUF_SIZE);
	//memset((void*)&headerBuf[0], 0, kHEADER_BUF_SIZE);
	len_header = ReadHeader(sockfd, &headerBuf[0]);
	if (len_header < 0) {
		err_msg("receive data failed:%d",sockfd );
		ret=kHTTP_ERROR;
		goto ERROR_SOCK;
	}

	//解析相应
	if (headerBuf[0] == '\0') {
		err_msg("Nothing receieved");
		ret=kHTTP_ERROR;
		goto ERROR_SOCK;
	}
	page_parser.ParseHeaderInfo(headerBuf);
#ifdef DEBUG_1_
	log_debug("%s:status:%d", strUrl, page_parser.m_nStatusCode);
#endif
	len_tmp = page_parser.m_nStatusCode;
	log_msg("Status:%d",len_tmp);
	if (len_tmp == -1) {
		err_msg("ResponseStatusCode wrong");
		ret=kHTTP_ERROR;
		goto ERROR_SOCK;
	}

	//针对相应状态，分发操作
	if (301 == len_tmp || 302 == len_tmp) {
		//重定向
		//此时应该关闭这个连接，在新的URL上建立连接
		if (nullptr==location || page_parser.m_sLocation.empty() || page_parser.m_sLocation.size() > kURL_LEN) {
			err_msg("Location Url wrong");
			ret=kHTTP_ERROR;
			goto ERROR_SOCK;
		}
		else {
			//此时应该放弃当前连接，建立重定向后的连接
#ifdef DEBUG_L
			tmp_lock.lock();
#endif
			*location=(char*)calloc(page_parser.m_sLocation.size()+1,1);
			//*location=strdup(page_parser.m_sLocation.c_str());
#ifdef DEBUG_L
			tmp_lock.unlock();
#endif
			if(nullptr==*location){
				ret=kHTTP_ERROR;
				goto ERROR_SOCK;
			}
			memcpy(*location,page_parser.m_sLocation.c_str(),page_parser.m_sLocation.size());
			log_msg("NewLocation:%s",*location);
			ret=kHTTP_LOCATION;
			goto ERROR_SOCK;
		}
	}

	//处理其他的状态
	if(len_tmp<200 || len_tmp>299){
		ret=kHTTP_ERROR;
		err_msg("Bad status:%d",len_tmp);
		goto ERROR_SOCK;
	}

	//是否处理image类型的网页
	if(0==isHandleImage){
		//不处理的时候，进行过滤
		if(string::npos!=page_parser.m_sContentType.find("image")){
			ret=kHTTP_IMG;
			goto ERROR_SOCK;
		}
	}

	//处理响应体
	//注意存在两种情况
	//1.指定了content-length
	//2.指定了transferEncoding:chunked
	
	if(page_parser.m_nContentLength==-1 && page_parser.m_sTransferEncoding.empty()){
		//那一种都没有指定
		err_msg("read response failed:%d",sockfd);
		ret=kHTTP_ERROR;
		goto ERROR_SOCK;
	}

	//
	//if(page_parser.m_nContentLength==0 || page_parser.m_nContentLength<20){
	//	page_parser.m_nContentLength=kDEFAULT_PAGE_BUFSIZE;
	//}
	if(page_parser.m_nContentLength>kMAX_PAGE_BUFSIZE){
		err_msg("Too big page:%d",sockfd);
		goto ERROR_SOCK;
	}
	//这里的选择策略是如果存在chunked字段，contentlength将被忽略
	if(strcasecmp(page_parser.m_sTransferEncoding.c_str(),"chunked")==0){
		ret=ReadBody_chunked(sockfd,&pageBuf);
	}
	else{
		//注意\0
		ret=ReadBody(sockfd,page_parser.m_nContentLength+1,&pageBuf);
	}

	if(ret<0 || nullptr==pageBuf){
		err_msg("Read response faild:%d",sockfd);
		ret=kHTTP_ERROR;
		goto ERROR_SOCK;
	}

	//接收数据小于预期时
	//保留吗？？？
	if(page_parser.m_nContentLength>0 && ret<page_parser.m_nContentLength){
		err_msg("data length less than expected:%d-%d",sockfd,ret);
		//ret=kHTTP_ERROR;
		//goto ERROR_PAGE;
	}

	//是否丢弃数据
	//未指定接收者时
	if(!fileBuf || !fileHeaderBuf){
		*nSock=sockfd;
		//ret 记录的读取数据
		//ret=kHTTP_QUIT;
		goto ERROR_PAGE;
	}

#ifdef DEBUG_L
	tmp_lock.lock();
#endif
	(*fileHeaderBuf)=(char *)malloc(len_header+1);
#ifdef DEBUG_L
	tmp_lock.unlock();
#endif

	if(!*fileHeaderBuf){
		ret=kHTTP_ERROR;
		goto ERROR_PAGE;
	}

	memcpy(*fileHeaderBuf,(char *)&headerBuf[0],len_header+1);
	*fileBuf=pageBuf;
	*nSock=sockfd;
	return ret;

	//出错出口，释放pageBuf
ERROR_PAGE:
#ifdef DEBUG_L
	tmp_lock.lock();
#endif
	free(pageBuf);
#ifdef DEBUG_L
	tmp_lock.unlock();
#endif
	pageBuf=nullptr;

	//出错出口，释放socket
ERROR_SOCK:
	close(sockfd);
	*nSock = -1;
	err_msg("Data exchange failed");

	//出错出口，释放requestBuf以及url
ERROR_BUF:
#ifdef DEBUG_L
	tmp_lock.lock();
#endif
	free(requestBuf);
#ifdef DEBUG_L
	tmp_lock.unlock();
#endif
	requestBuf = nullptr;
	return ret;

}


//brief:辅助函数，封装头部
//return：错误信息或者头部长度，ret负责获得接受资源
//注意释放ret资源
int Http::_WrapHeader(const string &strUrl, Url &url_parser, void **ret) {
	char *url;
	char *requestBuf;
	char *buf_finish;
	char *buf_end_of_storage;
	const char *host;
	const char *path;
	int bufsize = kREQUEST_LEN;
	int len_tmp;
	int len_write;
	int kth_index;
	char tmp_cat[kMAX_URL_REFERENCES];
	//int port;

	if (strUrl.empty()) {
		return kHTTP_ERROR;
	}

#ifdef DEBUG_L
	std::unique_lock<std::mutex> tmp_lock(mutexMemory);
#endif
	url = (char*)calloc(strUrl.size()+1,1);
#ifdef DEBUG_L
	tmp_lock.unlock();
#endif
	if (!url) {
		err_ret("malloc failed,%d,", errno);
		return kHTTP_ERROR;
	}
	memcpy(url,strUrl.c_str(), strUrl.size());

	host = url_parser.m_sHost.c_str();
	path = url_parser.m_sPath.c_str();
	//port = url_parser.m_nPort > 0 ? url_parser.m_nPort : kDEFAULT_FTP_PORT;

	//malloc资源
#ifdef DEBUG_L
	tmp_lock.lock();
#endif
	requestBuf = (char*)calloc(bufsize,1);
#ifdef DEBUG_L
	tmp_lock.unlock();
#endif
	if (nullptr == requestBuf) {
		err_ret("malloc failed");
		goto ERROR_STRDUP;
	}
	buf_finish = requestBuf;
	buf_end_of_storage = requestBuf + bufsize;
	if (strlen(path) == 0) {
		//默认将空路径当作请求根目录
		len_write=snprintf(tmp_cat,kMAX_URL_REFERENCES,"GET / %s\r\n",kHTTP_VERSION.c_str());//得到拼接串长度
		if (len_write<0 || CheckBufSize(&requestBuf, &buf_finish, &buf_end_of_storage, len_write) != 0
			|| (len_tmp=sprintf(buf_finish,"%s",tmp_cat)) != len_write ) {
			goto ERROR_BUF;
		}
		buf_finish += len_write;
	}
	else {
		//处理非空路径
		len_write = snprintf(tmp_cat,kMAX_URL_REFERENCES,"GET %s %s\r\n", path, kHTTP_VERSION.c_str());
		if (len_write <0 || CheckBufSize(&requestBuf, &buf_finish, &buf_end_of_storage, len_write) != 0
			|| (len_tmp = sprintf(buf_finish, "%s", tmp_cat)) != len_write) {
			goto ERROR_BUF;
		}
		buf_finish += len_write;
	}
	//封装Host
	len_write = snprintf(tmp_cat, kMAX_URL_REFERENCES, "Host: %s\r\n", host);
	if (len_write <0 || CheckBufSize(&requestBuf, &buf_finish, &buf_end_of_storage, len_write) != 0
		|| (len_tmp = sprintf(buf_finish, "%s", tmp_cat)) != len_write) {
		goto ERROR_BUF;
	}
	buf_finish += len_write;

	//UA设置
	if (!kHIDE_USER_AGENT && userAgent == nullptr) {
		//策略：从默认UA中循环挑选一个
		//注意这里只允许最多16个,另外这里线程安全并不重要
		kth_ua = (kth_ua + 1) & 0xF;
		kth_index=kth_ua%kLEN_UA_POOL;
		len_write = snprintf(tmp_cat, kMAX_URL_REFERENCES, "User-Agent: %s\r\n",UA_POOL[kth_index].c_str());
		if (len_write <0 || CheckBufSize(&requestBuf, &buf_finish, &buf_end_of_storage, len_write) != 0
			|| (len_tmp = sprintf(buf_finish, "%s", tmp_cat)) != len_write) {
			goto ERROR_BUF;
		}
		buf_finish += len_write;
	}
	else if (!kHIDE_USER_AGENT) {
		//不匿名，且使用指定host
		len_write = snprintf(tmp_cat, kMAX_URL_REFERENCES, "User-Agent: %s\r\n",userAgent);
		if (len_write < 0 || CheckBufSize(&requestBuf, &buf_finish, &buf_end_of_storage, len_write) != 0
			|| (len_tmp = sprintf(buf_finish, "%s", tmp_cat)) != len_write) {
			goto ERROR_BUF;
		}
		buf_finish += len_write;
	}

	//Connection设置
	//这是头部的最后一部分
	//需要用\r\n分隔
	len_write = snprintf(tmp_cat, kMAX_URL_REFERENCES, "Connection: Keep-Alive\r\n\r\n");
	if (len_write < 0 || CheckBufSize(&requestBuf, &buf_finish, &buf_end_of_storage, len_write) != 0
		|| (len_tmp = sprintf(buf_finish, "%s", tmp_cat)) !=len_write) {
		goto ERROR_BUF;
	}
	buf_finish += len_write;

	//重新分配合适空间，去除冗余
	len_tmp = buf_finish - requestBuf;
#ifdef DEBUG_L
	tmp_lock.lock();
#endif
	*ret = realloc((char *)requestBuf, len_tmp + 1);//Becare:给\0预留空间
#ifdef DEBUG_L
	tmp_lock.unlock();
#endif
	if (!*ret) {
		goto ERROR_BUF;
	}
	
	//返回长度
	return len_tmp;

	//出错出口，释放requestBuf以及url
ERROR_BUF:
#ifdef DEBUG_L
	tmp_lock.lock();
#endif
	free(requestBuf);
#ifdef DEBUG_L
	tmp_lock.unlock();
#endif
	requestBuf = nullptr;

	//出错出口，统一释放url
ERROR_STRDUP:
#ifdef DEBUG_L
	tmp_lock.lock();
#endif
	free(url);
#ifdef DEBUG_L
	tmp_lock.unlock();
#endif
	url = nullptr;
	return kHTTP_ERROR;
}


//brief:发完请求之后，接收头部信息
//返回接收到的头部长度,注意是不包含\0的长度
//非阻塞I/O
//响应头与响应体之间有两个\r\n分割,以此为标记
//由于需要只读取头部，如果不采用rio_t提供缓存的话，必须逐个字符
int Http::ReadHeader(int sockfd, char *headerPtr){
	int lines=0;//标识\r\n\r\n
    int read_bytes=0;
	char *buf = headerPtr;
	int ret;
    fd_set fdset_read;
	timeval tv;
	int flags;

	//非阻塞模式
	if((flags=fcntl(sockfd,F_GETFL))<0){
		err_ret("get f_getfl failed");
		return kHTTP_ERROR;
	}
	flags|=O_NONBLOCK;
	if(fcntl(sockfd,F_SETFL,flags)<0){
		err_ret("set f_setfl failed");
		return kHTTP_ERROR;
	}

	while(2!=lines && read_bytes<kHEADER_BUF_SIZE-1 ){
		FD_ZERO(&fdset_read);
		FD_SET(sockfd,&fdset_read);

		//注意部分系统会修改这个值
		//保证他一定非阻塞
		tv.tv_sec=Timeout>0?Timeout:kDEFAULT_TIMEOUT;
		tv.tv_usec=0;

		//避免系统中断select操作
RESTART_SELECT:
		ret=select(sockfd+1,&fdset_read,nullptr,nullptr,&tv);
		if(ret<=0){
			//超时
			if(errno==EINTR){
				//允许重启
				goto RESTART_SELECT;
			}
			err_ret("Receive data failed or timeout:%d",sockfd);
			return kHTTP_ERROR;
		}
		ret=RioRead(sockfd,buf,1);

		if(ret<0){
			//注意，可能为0
			err_msg("Read header faild:%d,ret=%d",sockfd,ret);
			return kHTTP_ERROR;
		}
		
		//连续两次CRLF标识结束
		if(*buf=='\r'){
			//
			;
		}
		else if(*buf=='\n'){
			lines+=1;
		}
		else{
			lines=0;
		}
        read_bytes+=ret;
		buf += ret;
	}
	//注意这里，两次CRLF已经被记录
	//应当被终结
	//是否需要舍弃一次的CRLF??
	
	*buf='\0';

	//记得改回来。。。
	flags&=(~O_NONBLOCK);
	if(fcntl(sockfd,F_SETFL,flags)<0){
		return kHTTP_ERROR;
	}
	return read_bytes;
}


//brief:读取响应体，要求提供content-length
//return:错误信息
//parameter：ret记录读取的数据，注意通过malloc分配，需要释放
//Becare:所有多余数据全部舍弃，不容忍，并且存在超时机制
int Http::ReadBody(int sockfd, int len, char **ret){
	char *buf;
	int ret_number;
	int tv_sec;
	//char quit_data[10];

#ifdef DEBUG_L
	std::unique_lock<std::mutex> tmp_lock(mutexMemory);
#endif
	buf=(char*)malloc(len);
#ifdef DEBUG_L
	tmp_lock.unlock();
#endif
	if(buf==nullptr){
		err_ret("Memory malloc failed:%d",sockfd);
		return kHTTP_ERROR;
	}

	//读取数据的方式是相似的
	//先设置描述符为非阻塞的
	//
	//这里通过Rio实现
	tv_sec=Timeout>0?Timeout:kDEFAULT_TIMEOUT;
	ret_number=RioRead_nonb(sockfd,buf,len-1,tv_sec);//注意预留了\0位置

	if(ret_number<0){
		goto ERROR_BUF;
	}

	//丢弃多余的数据，
	//注意下面的函数封装了超时机制，10s,因为基于字节流的数据如果keepalive了是没有边界的，就可能永久阻塞
	//while(RioRead_nonb(sockfd,quit_data,10,10)>0){
	//	;
	//}

	*(buf+ret_number)='\0';
	*ret=buf;
	return ret_number;
	

	//错误出口
	ERROR_BUF:
#ifdef DEBUG_L
	tmp_lock.lock();
#endif
    free(buf);
#ifdef DEBUG_L
	tmp_lock.unlock();
#endif
	buf=nullptr;
	*ret = nullptr;
	return kHTTP_ERROR;
}

//brief:ReadBody_chunked的辅助函数，检查char[]并转换为数字
//return:-1:出错，否则返回大于0的值
static int _Check_Transform(char *start, char *last) {
	string str(start, last);
	int out;
	try {
		out = stoi(str);
	}
	catch (std::invalid_argument) {
		return -1;
	}
	return out > 0 ? out : -1;
}
//brief:读取响应体，当声明为chunked时
//return:错误信息
//parameter:ret记录读取的数据，注意通过malloc分配，需要释放
//BeCare: chunked的传输方式是http1.1之后提出的，传输结束通过一个空白块表达
int Http::ReadBody_chunked(int sockfd, char **ret){
	char *buf;
	int tv_sec;
	//char quit_data[10];
	int len = 10240;//初始空间，指数增长，有上限
	int pos = 0;//写入元素的位置
    timeval tv;
	fd_set fdset_read;
	int flags;
	int nread;
	int pos_start;//块的起始位置
	int len_chunk;


#ifdef DEBUG_L
	std::unique_lock<std::mutex> tmp_lock(mutexMemory);
#endif
	buf=(char*)malloc(len);
#ifdef DEBUG_L
	tmp_lock.unlock();
#endif
	if(buf==nullptr){
		err_ret("Memory malloc failed:%d",sockfd);
		return kHTTP_ERROR;
	}
	*ret = buf;
	//读取数据的方式是相似的
	//先设置描述符为非阻塞的
	//
	//这里通过Rio实现
	tv_sec=Timeout>0?Timeout:kDEFAULT_TIMEOUT;

	if((flags=fcntl(sockfd,F_GETFL,0))<0){
		goto ERROR_BUF;
	}

	flags |= O_NONBLOCK;//保证非阻塞模式

	if(fcntl(sockfd,F_SETFL,flags)<0){
		goto ERROR_BUF;
	}

	//读取数据，分为两个步骤
	while (1) {
		//每次重置
		//因为只有一个描述符轮询，因此返回值如果成功只能为1
		FD_ZERO(&fdset_read);
		FD_SET(sockfd, &fdset_read);
		tv.tv_sec = tv_sec;
		tv.tv_usec = 0;
		nread = select(sockfd + 1, &fdset_read, nullptr, nullptr, &tv);

		if (nread != 1) {
			if (errno == EINTR) {
				continue;
			}
			goto ERROR_BUF;
		}


		pos_start = pos;
		//先读取这个块的大小，直到\n
		while (1) {
			if (pos >= len) {
				//扩充空间
				if (0 != CheckBufSize(&buf, &len, len)) {
					goto ERROR_BUF;
				}
			}
			nread = read(sockfd, (void *)(buf + pos), 1);
			if (nread == 0) {
				goto ERROR_BUF;
			}
			else if (nread < 0) {
				if (errno == EINTR) {
					continue;
				}
				else {
					goto ERROR_BUF;
				}
			}

			if (buf[pos] == '\n') {
				//可以确定大小了
				len_chunk = _Check_Transform(buf + pos_start, buf + pos);
				if (len_chunk < 0) {
					goto ERROR_BUF;
				}
				//丢弃块大小的说明数据
				pos = pos_start;
				break;
			}

			pos += nread;
		}

		//是否结束
		if (len_chunk == 0) {
			//最后一个空块用于说明数据传输完毕
			break;
		}

		//然后读取指定大小的块
		if (pos + len_chunk >= len) {
			//扩展空间
			if (0 != CheckBufSize(&buf, &len, len)) {
				goto ERROR_BUF;
			}
		}
		nread = RioRead_nonb(sockfd, buf + pos, len_chunk, tv_sec);
		if (nread != len_chunk) {
			//读取低于预期
			goto ERROR_BUF;
		}
		pos += len_chunk;
	}

	if(fcntl(sockfd,F_SETFL,flags)<0){
		goto ERROR_BUF;
	}

	//还可以考虑缩减不必要的空间数据
	buf[pos] = '\0';
	return pos;//不包含\0

	//错误出口
ERROR_BUF:
#ifdef DEBUG_L
	tmp_lock.lock();
#endif
    free(buf);
#ifdef DEBUG_L
	tmp_lock.unlock();
#endif
	buf=nullptr;
	*ret = nullptr;
	return kHTTP_ERROR;
}


Http::Http() {}

Http::~Http() {}
