/*************************************************************************
	> File Name: page.cpp
	> Author: Dream9
	> Brief: 
	> Created Time: 2019年04月26日 星期五 17时09分39秒
 ************************************************************************/
#include"page.h"
#include"strfun.h"

#include<iostream>
#include<cstring>
#include<string>
#include<iterator>


//int IsFilterLink(const string &);

//brief:默认初始化
Page::Page(){
	//初始化需要分析装填的信息
    m_nStatusCode=0;
	m_nContentLength=0;
	m_sLocation="";
	m_bConnectionState=false;
	m_sContentEncoding="";
	m_sContentType="";
	m_sCharset="";
	m_sTransferEncoding="";

	m_sContentLinkInfo="";
	m_sLinkInfo4SE="";
	m_sLinkInfo4History="";

	m_sContentNoTags="";
	m_nRefLink4SENum=0;
	m_nRefLink4HistoryNum=0;

	m_eType=kPlain_Text;

	int len_m_RefLink4History = kMAX_URL_REFERENCES >> 1;
	for (int i = 0; i < kMAX_URL_REFERENCES; ++i) {
		m_RefLink4SE[i].link = nullptr;
		m_RefLink4SE[i].anchor_text = nullptr;
		m_RefLink4SE[i].strCharset = "";

		if (i < len_m_RefLink4History) {
			m_RefLink4History[i].link = nullptr;
		}
	}
}

//brief:额外传入部分数据进行初始化
//包括最初地址，重定向地址，响应头，响应体，体长度
Page::Page(const string &strUrl, const string &strLocation, const char *header, const char *body, int nLenBody) {
	//初始化需要提取装填的信息
    m_nStatusCode=0;
	m_nContentLength=0;
	//m_sLocation="";
	m_bConnectionState=false;
	m_sContentEncoding="";
	m_sContentType="";
	m_sCharset="";
	m_sTransferEncoding="";

	m_sContentLinkInfo="";
	m_sLinkInfo4SE="";
	m_sLinkInfo4History="";

	m_sContentNoTags="";
	m_nRefLink4SENum=0;
	m_nRefLink4HistoryNum=0;

	m_eType=kPlain_Text;

	int len_m_RefLink4History = kMAX_URL_REFERENCES >> 1;
	for (int i = 0; i < kMAX_URL_REFERENCES; ++i) {
		m_RefLink4SE[i].link = nullptr;
		m_RefLink4SE[i].anchor_text = nullptr;
		m_RefLink4SE[i].strCharset = "";

		if (i < len_m_RefLink4History) {
			m_RefLink4History[i].link = nullptr;
		}
	}

	//记录其他信息
	//也就是被分析数据
	m_sUrl = strUrl;
	m_sLocation = strLocation;
	m_sHeader = header;
	m_nLenHeader = strlen(header);
	m_sContent.assign(body, nLenBody);
	m_nLenContent = nLenBody;
}

//析构
//Page::~Page() {
//	//ToDo;;
//}

//brief:设置特殊信息
//becare:m_sUrl采用http协议
void Page::SetPage(const string &strUrl, const string &strLocation, const char *header, const char *body, int nLenBody) {
	m_sUrl = strUrl;
	m_sLocation = strLocation;
	m_sHeader = header;
	m_nLenHeader = strlen(header);
	m_sContent.assign(body, nLenBody);
	m_nLenContent = nLenBody;

}



//一下为响应头各个部分的解析函数实现

//brief:解析StatusCode
void Page::GetStatusCode(string &header) {
	//形如：HTTP/1.1 200 OK .........\r\n
	auto pos_http = StrFun::FindCase(header, "http/");
	if (pos_http == string::npos) {
		//未找到
		m_nStatusCode = -1;
		return ;
	}
	//
	//pos_http += strlen("http/");
	//while (header[pos_http] == ' ') {
	//	++pos_http;
	//}
	//效果是一样的
	//while (header[pos_http] != ' ') {
	//	++pos_http;
	//}
	//定位到数字前面的空格，排除意外数据
	pos_http += sizeof "http/" - 1;     
	while (header[pos_http] != ' ') pos_http++;

	int ret=sscanf(header.c_str() + pos_http, "%i", &m_nStatusCode);
	if (ret != 1) {
		//失败了
		//err_msg("StatusCode : failed");
		m_nStatusCode = -1;
	}
}

//brief:解析ContentLength
void Page::GetContentLength(string &header) {
	//形如 ... Content-Length: 54\r\n
	auto pos_content_length = StrFun::FindCase(header, "content-length");
	if (pos_content_length == string::npos) {
		//
		m_nContentLength = -1;
		return;
	}

	pos_content_length += sizeof "content-length" -1;// strlen("content-length");
	while (header[pos_content_length] != ' ') pos_content_length++;

	int ret = sscanf(header.c_str() + pos_content_length, "%i", &m_nContentLength);
	if (ret != 1) {
		//失败
		//err_msg("Content-Length : failed");
		m_nContentLength = -1;
	}
}

//brief:解析重定向信息
void Page::GetLocation(string &header) {
	//形如： location:	Location/DEVOPS/index.php/Main/index\r\n
	//注意这里提取的是一个string,一直到\r\n分隔
	auto pos_location_start = StrFun::FindCase(header, "location:");
	if (pos_location_start != string::npos) {
		pos_location_start += sizeof "location:" - 1;
	    while (header[pos_location_start] == ' ') pos_location_start++;
		auto pos_location_end = header.find_first_of("\r\n", pos_location_start);
		if (pos_location_end != string::npos) {
			//find it
			m_sLocation = header.substr(pos_location_start, pos_location_end - pos_location_start);
		}
	}
}

//brief:解析字符集
void Page::GetCharset(string &header) {
	//形如： Content-Type: application/json; charset=utf-8\r\n
	//注意这里的分隔符，多一点，包括 ",;> 
	const string delims("\",;>\r\n");
	auto pos_charset_start = StrFun::FindCase(header, "charset=");
	if (pos_charset_start != string::npos) {
		pos_charset_start += sizeof "charset=" - 1;
	    while (header[pos_charset_start] == ' ') pos_charset_start++;
		auto pos_charset_end = header.find_first_of(delims, pos_charset_start);
		if (pos_charset_end != string::npos) {
			m_sCharset = header.substr(pos_charset_start, pos_charset_end - pos_charset_start);
		}
	}
}

//brief:解析编码方式
void Page::GetContentEncoding(string &header) {
	//形如： Content-Encoding：gzip\r\n
	auto pos_contentencoding_start = StrFun::FindCase(header, "content-encoding:");
	if (pos_contentencoding_start != string::npos) {
		pos_contentencoding_start += sizeof "content_encding:" - 1;
	    while (header[pos_contentencoding_start] == ' ') pos_contentencoding_start++;
		auto pos_contentencoding_end = header.find_first_of("\r\n", pos_contentencoding_start);
		if (pos_contentencoding_end != string::npos) {
			m_sContentEncoding = header.substr(pos_contentencoding_start, pos_contentencoding_end - pos_contentencoding_start);
		}
	}
}

//brief:解析是否断开连接
void Page::GetConnectionState(string &header) {
	//形如： Connection: keep-alive\r\n
	auto pos_connectionstate_start = StrFun::FindCase(header, "connection:");
	if (pos_connectionstate_start != string::npos) {
		pos_connectionstate_start += sizeof "connection:" - 1;
	    while (header[pos_connectionstate_start] == ' ') pos_connectionstate_start++;
		auto pos_connectionstate_end = header.find_first_of("\r\n",pos_connectionstate_start);
		if (pos_connectionstate_end != string::npos) {
			string state_token = header.substr(pos_connectionstate_start, pos_connectionstate_end - pos_connectionstate_start);
			//if (strcasecmp(state_token.c_str(), "keep-alive") == 0) {
			//	m_bConnectionState = true;//error
			//}
			if(string::npos!=StrFun::FindCase(state_token,"keep-alive")){
				m_bConnectionState = true;
			}
		}
	}
}

//brief:解析类型
void Page::GetContentType(string &header) {
	//形如： Content-Type: application / json; charset = utf - 8\r\n
	auto pos_contenttype_start = StrFun::FindCase(header, "content-type:");
	if (pos_contenttype_start != string::npos) {
		pos_contenttype_start += sizeof "content-type:" - 1;
	    while (header[pos_contenttype_start] == ' ') pos_contenttype_start++;
		auto pos_contenttype_end = header.find_first_of(";\r\n", pos_contenttype_start);
		if (pos_contenttype_end != string::npos) {
			m_sContentType = header.substr(pos_contenttype_start, pos_contenttype_end - pos_contenttype_start);
		}
	}
}

//brief:解析传输编码方式
void Page::GetTransferEncoding(string &header) {
	//形如：Transfer-Encoing:chunked\r\n
	auto pos_transfreencoding_start = StrFun::FindCase(header, "transfer-encoding:");
	if (pos_transfreencoding_start != string::npos) {
		pos_transfreencoding_start += sizeof "transfer-encoding:" - 1;
	    while (header[pos_transfreencoding_start] == ' ')  pos_transfreencoding_start++;
		auto pos_transferencoding_end = header.find_first_of(";\r\n", pos_transfreencoding_start);
		if (pos_transferencoding_end != string::npos) {
			m_sTransferEncoding = header.substr(pos_transfreencoding_start, pos_transferencoding_end - pos_transfreencoding_start);
		}
	}
}

//利用上述函数完成头部解析：：：：
void Page::ParseHeaderInfo(string &strHeader) {
	//外部接口
	GetStatusCode(strHeader);
	GetContentLength(strHeader);
	GetLocation(strHeader);
	GetConnectionState(strHeader);
	GetCharset(strHeader);
	GetContentEncoding(strHeader);
	GetContentType(strHeader);
	GetTransferEncoding(strHeader);
}



//以下为对response body的解析

//brief:IsFilterLink的辅助函数，找寻两个word
bool _find_two_word(const string &target, const string &first, const string &second) {
	auto pos_first = StrFun::FindCase(target,first);
	if (pos_first != string::npos) {
		pos_first += first.size();
		auto pos_second = StrFun::FindCaseFrom(target,second, pos_first);
		return pos_second != string::npos;
	}
	return false;
}
//brief:判断是否需要过滤这个连接
//去除一些无意义的连接，，spam link
//这个规则需要根据需求进行扩充和更改
//当下规则：
//1.重复出现出现?,&,"//","http","misc","ipb","https"以及同时出现-+
//2.出现一些脚本，或者需要登录等额外信息的标志
int IsFilterLink(const string &strlink) {
	if (strlink.empty() || strlink.size() > kURL_LEN) {
		return 1;
	}
	
	const string filter_word_list[] = { "?","&","//","http","misc","ipb","https" };
	for (string filter_word : filter_word_list) {
		if (_find_two_word(strlink, filter_word, filter_word)) {
			return 1;
		}
	}
	//特殊的一个
	if (_find_two_word(strlink, "-", "+")) {
		return 1;
	}

	const string filter_str[] = {
	"cgi-bin",	"htbin",	"linder",	"srs5",		"uin-cgi", 
	"uhtbin",	"snapshot",	"=+",		"=-",		"script",
	"gate",		"search",	"clickfile",	"data/scop",	"names",
	"staff/",	"enter",	"user",		"mail",	"pst?",
	"find?",	"ccc?",		"fwd?",		"tcon?",	"&amp",
	"counter?",	"forum",	"cgisirsi",	"{",		"}",
	"proxy",	"login",	"00.pl?",	"sciserv.pl",	"sign.asp",
	"<",		">",		"review.asp?",	"result.asp?",	"keyword",
	"\"",		"'",		"php?s=",	"error",	"showdate",
	"niceprot.pl?",	"volue.asp?id",	".css",		".asp?month",	"prot.pl?",
	"msg.asp",	"register.asp", "database",	"reg.asp",	"qry?u",
	"p?msg",	"tj_all.asp?page", ".plot.",	"comment.php",	"nicezyme.pl?",
	"entr",		"compute-map?", "view-pdb?",	"list.cgi?",	"lists.cgi?",
	"details.pl?",	"aligner?",	"raw.pl?",	"interface.pl?","memcp.php?",
	"member.php?",	"post.php?",	"thread.php",	"bbs/",		"/bbs"
	};
	for (string word : filter_str) {
		if (string::npos != StrFun::FindCase(strlink, word)) {
			return 1;
		}
	}

	return 0;
}

//brief:转调
int Page::IsFilter(const string &strlink) const{
	return IsFilterLink(strlink);
}

//brief:GetContentLinkInfo等的辅助函数，
void _find_and_record_tag(string &strtarget, const string &tag, string &strrecord) {
	auto pos_word_start = StrFun::FindCase(strtarget, tag);
	while (string::npos != pos_word_start) {
		//注意是从下一个位置
		//auto pos_word_end = strtarget.find_first_of('<', pos_word_start);
		auto pos_word_end = strtarget.find_first_of('<', pos_word_start+1);
		if (string::npos != pos_word_end) {
			//strrecord += strtarget.substr(pos_word_start, pos_word_end - pos_word_start + 1);
			string tmp = strtarget.substr(pos_word_start, pos_word_end - pos_word_start);
			strrecord += tmp;
		}
		else {
			//err_msg("Unexcepted end of response body!!");
			//注意最后一个链接的后面没有<了
			strrecord += strtarget.substr(pos_word_start);
			return;
		}
		pos_word_start = StrFun::FindCaseFrom(strtarget, tag, pos_word_end);
	}
}

//brief:作用是提炼m_sContent,去除不需要的tag,结果保存到m_sContentLinkInfo中
//此处的策略为：
//1.去除\r\n\t <br>    ??<\br>
//2.仅保留可能取得link的tag,包括 <a href= >,<area >,<img >
bool Page::GetContentLinkInfo() {

	if (m_sContent.empty()) {
		return false;
	}

	m_sContentLinkInfo = m_sContent;
	string &alias = m_sContentLinkInfo;

	//将\r\n\t替换为单个空格，注意相邻的算作一个
	const string delims("\r\n\t");
	auto pos_delims = alias.find_first_of(delims);
	while (string::npos != pos_delims) {
		alias[pos_delims] = ' ';
		auto pos_next_delims = alias.find_first_of(delims, pos_delims+1);
		while (string::npos != pos_next_delims && pos_next_delims == pos_delims + 1) {
			//相邻的算作一个
			alias.erase(pos_next_delims, 1);
			pos_next_delims = alias.find_first_of(delims, pos_delims + 1);
		}
		
		//pos_delims = alias.find_first_of(delims, pos_delims);//注意这个位置已经替换为空格了
		pos_delims = alias.find_first_of(delims, pos_delims+1);
	}

	//替换<br>
	StrFun::ReplaceStr(alias, "<br>", " ");
	if (alias.size() < kMIN_CONTENTLINKINFO) {
		alias = "";
		return false;
	}

	string strfinal_link;
	//auto pos_word = StrFun::FindCaseFrom(alias, "href",0);
	//while (string::npos!=pos_href) {
	//	auto pos_word_end=
	//}
	const string tags[] = { "<a href","<area","<img" };//注意保持完整的<a href 形式，之后可以通过<进行区分
	for (string tag : tags) {
		_find_and_record_tag(alias, tag, strfinal_link);
	}
	alias.swap(strfinal_link);
	StrFun::EraseStr(alias, "\\");
	if (alias.size() < kMIN_CONTENTLINKINFO) {
		alias = "";
		return false;
	}
	return true;
}

//brief:从m_sContentLinkInfo中提取信息，结果存储在m_sLinkInfo4SE,用于FindLinkInfo4SE
//在初步过滤基础之上，进行二次过滤
//这里的过滤规则为，提取 <a href
bool Page::GetLinkInfo4SE() {
	if (m_sContentLinkInfo.empty()) {
		return false;
	}

	m_sLinkInfo4SE = m_sContentLinkInfo;
	string &alias = m_sLinkInfo4SE;

	string strfinal_link;
	const string tags[] = { "<a href","<area" };
	for (string tag : tags) {
		_find_and_record_tag(alias, tag, strfinal_link);
	}

	//需要进一步删除的字符集，&nbsp标识空格（html中连续空格当作1个处理）
	const string str_to_del[] = { "\"","'","&nbsp" };
	for (string str_del_word : str_to_del) {
        StrFun::EraseStr(strfinal_link, str_del_word);
	}
	if (strfinal_link.size() < kMIN_CONTENTLINKINFO) {
		alias = "";
		return false;
	}

	//<a href = "http://www.w3school.com.cn">W3School< / a>
	//<a href="http://www.w3school.com.cn">W3School
	//进一步简化link信息
	//const string delims(" #>\"");//上一部已经去除了",,现在形如：<a href=http://www.w3school.com.cn>W3School
	const string delims(" #>");
	alias.clear();
	auto len_strfinal_link = strfinal_link.size();
	auto pos_href = StrFun::FindCase(strfinal_link, "href");
	while (pos_href < len_strfinal_link) {
		//pos_href = StrFun::FindCaseFrom(strfinal_link, "href", pos_href);
		if (string::npos == pos_href) {
			break;
		}
		pos_href += sizeof "href" - 1;
		while ((pos_href < len_strfinal_link) && (strfinal_link[pos_href] == ' ' || strfinal_link[pos_href] == '=')) {
			++pos_href;
		}
		if (pos_href == len_strfinal_link) {
			break;
		}

		//此时应该指向http://www.w3school.com.cc>W3School</a>
		auto pos_href_end = strfinal_link.find_first_of(delims, pos_href);
		if (string::npos == pos_href_end) {
			break;
		}
		alias += '"' + strfinal_link.substr(pos_href, pos_href_end - pos_href);
		//这里也把href的text信息提取出来
		auto idx = strfinal_link.find('>', pos_href);
		if (string::npos == idx) {
			break;
		}
		alias += '>';
		auto idx2 = strfinal_link.find('<', idx + 1);
		pos_href = StrFun::FindCaseFrom(strfinal_link, "href", idx2);//跳到下一个上面
		idx2 = idx2 < pos_href ? idx2 : pos_href;
		alias += strfinal_link.substr(idx + 1, idx2 - idx - 1);
	}
	alias += '"';
	StrFun::EraseStr(alias, "\"\"");//去除空url
	if (alias.size() < kMIN_CONTENTLINKINFO) {
		alias = "";
		return false;
	}
	return true;
}


//brief:解析用于后期存档的链接
//主要针对<img ...>标签
bool Page::GetLinkInfo4History() {
	if (m_sContentLinkInfo.empty()) {
		return false;
	}

	m_sLinkInfo4History= m_sContentLinkInfo;
	string &alias = m_sLinkInfo4History;

	string strfinal_link;
	const string tags[] = { "<img" };
	for (string tag : tags) {
		_find_and_record_tag(alias, tag, strfinal_link);
	}

	//需要进一步删除的字符集，&nbsp标识空格（html中连续空格当作1个处理）
	const string str_to_del[] = { "\"","'","&nbsp" };
	for (string str_del_word : str_to_del) {
        StrFun::EraseStr(strfinal_link, str_del_word);
	}
	if (strfinal_link.size() < kMIN_CONTENTLINKINFO) {
		alias = "";
		return false;
	}

	//<a href = "http://www.w3school.com.cn">W3School< / a>
	//<a href="http://www.w3school.com.cn">W3School
	//进一步简化link信息
	//const string delims(" #>\"");//上一部已经去除了",,现在形如：<a href=http://www.w3school.com.cn>W3School
	const string delims(" #>");
	alias.clear();
	size_t  len_strfinal_link = strfinal_link.size();
	auto pos_href = StrFun::FindCase(strfinal_link, "src");
	while (pos_href < len_strfinal_link) {
		//pos_href = StrFun::FindCaseFrom(strfinal_link, "href", pos_href);
		if (string::npos == pos_href) {
			break;
		}
		pos_href += sizeof "src" - 1;
		while ((pos_href < len_strfinal_link) && (strfinal_link[pos_href] == ' ' || strfinal_link[pos_href] == '=')) {
			++pos_href;
		}
		if (pos_href == len_strfinal_link) {
			break;
		}

		//此时应该指向http://www.w3school.com.cc>W3School</a>
		auto pos_href_end = strfinal_link.find_first_of(delims, pos_href);
		if (string::npos == pos_href_end) {
			break;
		}
		alias += '"' + strfinal_link.substr(pos_href, pos_href_end - pos_href);
		//这里也把href的anchor_text信息提取出来
		auto idx = strfinal_link.find('>', pos_href);
		if (string::npos == idx) {
			break;
		}
		alias += '>';
		auto idx2 = strfinal_link.find('<', idx + 1);
		pos_href = StrFun::FindCaseFrom(strfinal_link, "src", idx2);//跳到下一个上面
		idx2 = idx2 < pos_href ? idx2 : pos_href;
		alias += strfinal_link.substr(idx + 1, idx2 - idx - 1);
	}
	alias += '"';//方便分隔
	StrFun::EraseStr(alias, "\"\"");//去除空url
	if (alias.size() < kMIN_CONTENTLINKINFO) {
		alias = "";
		return false;
	}
	return true;
}

//brief:辅助函数，网页补全功能
//针对相对路径的link,结合url已经分析出来的域名，端口等信息补全网页link
//becare:协议都是用http,只要保证m_sUrl是http的，，需要外部保证！
static int WebComplete(string &str_web,const Url &url_parser,const string &m_sUrl) {
	//url_parser
	if (str_web[0] == '/') {
		//形如  /index.html
		//相对路径,使用特殊端口的时候需要带上端口号
		//if (url_parser.m_eScheme == kSchemeHttps) {
		//	;
		//}
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

//brief:辅助函数，完成网页简化
//becare:把所有的https协议替换为http,否则涉及到ssl握手
static int _NormalizeUrl(string &str_url){
	StrFun::ReplaceStr(str_url, "https", "http");
	if (StrFun::FindCase(str_url, "http://") == string::npos) {
		return -1;
	}
	auto idx = str_url.rfind('/');
	if (idx < 8) {
		str_url = str_url + "/";	//以标准目录结尾
		return 0;
	}

	while ((idx = str_url.find("/./")) != string::npos) {
		if (idx != string::npos) str_url.erase(idx, 2);
	}

	while ((idx = str_url.find("/../")) != string::npos) {
		string strPre, strSuf;

		strPre = str_url.substr(0, idx);

		if (str_url.length() > idx + 4)
			strSuf = str_url.substr(idx + 4);

		idx = strPre.rfind("/");
		if (idx != string::npos)
			strPre = strPre.substr(0, idx + 1);
		if (strPre.length() < 10) return false;

		str_url = strPre + strSuf;
	}

	if (StrFun::FindCase(str_url, "http://") != 0) return false;

	return 0;
}
//brief:对URL简化
int Page::NormalizeUrl(string &str_url) {
	//WebComplete(str_url);
	return _NormalizeUrl(str_url);
	
}

//brief:FindRefLink4SE,FindRefLink4History的辅助函数,用于实际提取信息
//当typeflag=True时，为RefLink4SE；反之，反之
#define __PLACE_HOLD_1__(x,y) if((x) >= (y)) {break;}
#define __PLACE_HOLD_2__(x,y,z) if((x) >= (y)) {(z++);break;}
template<typename T>
static int _extract_info(char *buf, T ref[], int &ref_num, bool type_flag=true) {
	if (!buf) {
		return -1;
	}

	int len = strlen(buf);
	if (len < kMIN_CONTENTLINKINFO) {
		return -2;
	}
	int num = 0;
	auto buf_end=buf+len;
	while (buf < buf_end && num<kMAX_URL_REFERENCES) {
		// 形式   "link>anchor_text"
		//记录link
		while (buf < buf_end && *buf == '"') {
			++buf;
		}
		__PLACE_HOLD_1__(buf, buf_end);
		//此时buf指向link的起始位置
		ref[num].link = buf;
		while (buf < buf_end && *buf != '>') {
			*buf = *buf == ' ' ? '\0' : *buf;
			++buf;
		}
		__PLACE_HOLD_2__(buf, buf_end, num);
		//记录anchor_test
		*buf++ = '\0';
		__PLACE_HOLD_2__(buf, buf_end, num);
		if (*buf == '"') {
			++buf;//掠过下一个开头的"
			if (type_flag) {
				((RefLink4SE*)ref)[num].anchor_text = nullptr;
			}
		}
		else {
			if (type_flag) {
				((RefLink4SE*)ref)[num].anchor_text = buf;
			}
			//定位到下一个位置，并将其设置为/0，依次分隔
			while (buf < buf_end && *buf != '"') {
				++buf;
			}
			__PLACE_HOLD_2__(buf, buf_end, num);
			*buf++ = '\0';
		}
		num++;
	}
	ref_num = num;
	return 0;
}
//解除define
#undef __PLACE_HOLD_1__
#undef __PLACE_HOLD_2__


//brief:辅助函数，用于建立map结构
//becare:
static int _build_map(const Url &url_parser,RefLink4SE ref[],const int &ref_num, const string &m_sUrl,map<string,string> &map_ref) {
	if (!ref) {
		return -1;
	}
	const string delims(" #");
	for (int i=0; i<ref_num; ++i){
		string strlink = ref[i].link;
		auto pos_delims = strlink.find_first_of(delims);
		strlink = string::npos == pos_delims ? strlink : strlink.substr(0, pos_delims);
		int len_strlink = strlink.size();
		if (len_strlink == 0 || len_strlink > kURL_LEN - 1 || len_strlink < kMIN_URL_LEN) {
			continue;
		}

		//规范化处理
		auto pos_http = StrFun::FindCase(strlink, "http");
		if (pos_http != 0) {
			//此时网页是相对路径
			if (WebComplete(strlink, url_parser,m_sUrl) != 0) {
				err_msg("BuildMap failed when dealing with WebComplete:(%s)", strlink.c_str());
				continue;
			}
		}
		if (_NormalizeUrl(strlink) != 0) {
			log_msg("Failed to _NormalizeUrl:(%s)", strlink.c_str());
			continue;
		}
		if (IsFilterLink(strlink)) {
			//log_msg("Bad Link shouled be dopped:(%s)", strlink.c_str());
			continue;
		}
#ifdef DEBUG_1_
		log_msg("Good link: %s", strlink);
#endif
		if (strlink == m_sUrl) {
			continue;
		}
		if (ref[i].anchor_text) {
			//注意这里并不在乎插入之后的状态
			map_ref.insert(std::make_pair(strlink, ref[i].anchor_text));
		}
		else {
			map_ref.insert(std::make_pair(strlink, "\0"));
		}
	}
	return 0;
}

//brief：完成提炼信息
//从m_sLinkInfo4SE中提取信息，以RefLink4SE的形式进行存放，然后建立map结构
//Todo:直接在m_sLinkInfo4SE上面修改
bool Page::FindRefLink4SE() {
	if (m_sLinkInfo4SE.empty()) {
		return false;
	}

	const char *buffer = m_sLinkInfo4SE.c_str();
	//弃用
	//static char buf[kURL_REFERENCE_LEN];//常驻内存//注意这块空间是被这个类共享的//因此不适合并发计算
	//memset(buf, 0, kURL_REFERENCE_LEN);
	char buf[kURL_REFERENCE_LEN];

	//int len_buffer = strlen(buffer);//线性运算
	int len_buffer = m_sLinkInfo4SE.size();
	len_buffer = len_buffer < kURL_REFERENCE_LEN - 1 ? len_buffer : kURL_REFERENCE_LEN - 1;
	memcpy(buf, buffer, len_buffer);
	buf[len_buffer] = '\0';//哨兵

	//内部link的存储方式为 "link>anchor_text 的形式
	//提取出来存放到struct RefLink4SE中去
	//
	//ToDo:可以直接在string上面更改，
	if (0 != _extract_info(buf, m_RefLink4SE, m_nRefLink4SENum)) {
		return false;
	}

	//接下来将配合Url分析，对link进行格式规范化，然后存入到map结构中，用于继续使用
	m_mapLink4SE.clear();

	Url url_parser;
	//解析URL，使用原始地址？
	if (url_parser.ParseUrl(m_sUrl) == false) {
		err_msg("[%s]url_parser failed,which url is(%s)", __FUNCTION__, m_sUrl.c_str());
		return false;
	}
	//构建map记录link
	if (0 != _build_map(url_parser, m_RefLink4SE, m_nRefLink4SENum, m_sUrl, m_mapLink4SE)) {
		err_msg("build_map failed");
		return false;
	}
	m_nRefLink4SENum = m_mapLink4SE.size();

	return true;
}

//brief:GetRefLink4Histoty辅助函数，用于建立vec结构
//与_build_map很接近，也可以考虑template实现
static int _build_vec(const Url &url_parser,RefLink4History ref[],const int &ref_num, const string &m_sUrl, vector<string> &vec_ref) {
	if (!ref) {
		return -1;
	}
	for (int i=0; i<ref_num; ++i){
		string strlink = ref[i].link;
		int len_strlink = strlink.size();
		if (len_strlink == 0 || len_strlink > kURL_LEN - 1 || len_strlink < kMIN_URL_LEN) {
			continue;
		}

		//规范化处理
		auto pos_http = StrFun::FindCase(strlink, "http");
		if (pos_http != 0) {
			//此时网页是相对路径
			if (WebComplete(strlink, url_parser,m_sUrl) != 0) {
				err_msg("BuildMap failed when dealing with WebComplete:(%s)", strlink.c_str());
				continue;
			}
		}
		if (_NormalizeUrl(strlink) != 0) {
			log_msg("Failed to _NormalizeUrl:(%s)", strlink.c_str());
			continue;
		}
		if (IsFilterLink(strlink)) {
			log_msg("Bad Link shouled be dopped:(%s)", strlink.c_str());
			continue;
		}
#ifdef DEBUG_2_
		log_msg("Good link: %s", strlink);
#endif
		if (strlink == m_sUrl) {
			continue;
		}
		auto iter = std::find(vec_ref.begin(), vec_ref.end(), strlink);
		if (vec_ref.end() == iter) {
			vec_ref.emplace_back(strlink);
		}
	}
	return 0;
}
//brief:基于相似的处理，提取用于History的链接
bool Page::FindRefLink4History() {
	if (m_sLinkInfo4History.empty()) {
		return false;
	}
	m_vecLink4History.clear();
	m_nRefLink4HistoryNum = m_vecLink4History.size();
	return true;

	const char *buffer = m_sLinkInfo4SE.c_str();
	//static char buf[kURL_REFERENCE_LEN>>1];//常驻内存//注意这块空间是被这个类共享的//因此不适合并发计算
	char buf[kURL_REFERENCE_LEN>>1];//

	int len_buffer = m_sLinkInfo4SE.size();
	len_buffer = len_buffer < (kURL_REFERENCE_LEN>>1) - 1 ? len_buffer : (kURL_REFERENCE_LEN>>1) - 1;
	memcpy(buf, buffer, len_buffer);
	buf[len_buffer] = '\0';//哨兵

	//内部link的存储方式为 "link> 的形式
	//
	//ToDo:可以直接在string上面更改，
	if (0 != _extract_info(buf, m_RefLink4History, m_nRefLink4HistoryNum, false)) {
		return false;
	}

	//接下来将配合Url分析，对link进行格式规范化，然后存入到map结构中，用于继续使用
	m_vecLink4History.clear();

	Url url_parser;
	//解析URL，使用原始地址？
	if (url_parser.ParseUrl(m_sUrl) == false) {
		err_msg("[%s]url_parser failed,which url is(%s)", __FUNCTION__, m_sUrl.c_str());
		return false;
	}
	//构建map记录link
	if (0 != _build_vec(url_parser, m_RefLink4History, m_nRefLink4HistoryNum, m_sUrl, m_vecLink4History)) {
		err_msg("build_vec failed");
		return false;
	}
	m_nRefLink4HistoryNum = m_vecLink4History.size();

	return true;
}

//brief:综合解析功能，进行解析网页（response body）的link
//只要有一个提取成功，就应该true
bool Page::ParseHyperLinks() {
	//ToDo:
	if (false == GetContentLinkInfo()) {
		err_msg("Failed at GetContentLinkInfo");
		return false;
	}
	if (m_sContentLinkInfo.empty()) {
		return false;
	}
	bool bSE = false;
	bool bHistory = false;
	if (GetLinkInfo4SE()) {
		if (FindRefLink4SE()) {
			bSE = true;
		}
	}
	if (GetLinkInfo4History()) {
		if (FindRefLink4History()) {
			bHistory = true;
		}
	}
	return bSE || bHistory;
}
