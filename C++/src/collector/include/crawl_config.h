/*************************************************************************
	> File Name: crawl_config.h
	> Author: Dream9
	> Brief: 
	> Created Time: 2019年05月13日 星期一 19时20分25秒
 ************************************************************************/

#ifndef _CRAWL_CONFIG_H_
#define _CRAWL_CONFIG_H_

#include<string>
#include<vector>

using std::string;
using std::vector;

//工作目录
const string kWORK_DIRECTORY("../doc");

//初始化
const string kUNVISITEDURL_FILE_NAME("unvisited.url");
const string kREPLICAS_FILE_NAME("replicas");

//原始网页信息前缀
const string kDATA_FORMAT_FILE_PREFIX("formatfile.raw");
const string kDATA_LINK4SE_FILE_PREFIX("link4sefile.raw");

//已访问信息
const string kURLMD5_FILE_NAME("visitedurl.md5");
const string kPAGEMD5_FILE_NAME("visitedpage.md5");

const string kLINK4SE_FILE_NAME("link4se.url");
const string kLINK4HISTORY_FILE_NAME("link4history.url");


//限制ip
const string kIPBLOCK_FILE_NAME("ipblock");
const string kUNREACHHOST_FILE_NAME("unreachhost.str");

//isam数据+索引
const string kISAM_DATA_FILE_NAME("isam.data");
const string kISAM_INDEX_FILE_NAME("isam.index");

//gzip空间限制
const int kGZIP_LEN=1024000;

//支持解析link的网页类型
const vector<string> kPLAIN_TEXT_TYPE = { "text/html" ,"text/plain","text/xml","application/msword",
			 "application/pdf" ,"text/rtf" ,"application/postscript","application/vnd.ms-execl" ,
             "application/vnd.ms-powerpoint" };

//协议
const vector<string> kHTTP_PROTOCOLS = { "http","https" };

//版本号
//const string kVERSION_LINK4SE("1.0");
//const string kVERSION_ORIGINAL_PAGE("1.0");

#endif
