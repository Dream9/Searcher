/*************************************************************************
	> File Name: crawl_config.h
	> Author: Dream9
	> Brief: 
	> Created Time: 2019��05��13�� ����һ 19ʱ20��25��
 ************************************************************************/

#ifndef _CRAWL_CONFIG_H_
#define _CRAWL_CONFIG_H_

#include<string>
#include<vector>

using std::string;
using std::vector;

//����Ŀ¼
const string kWORK_DIRECTORY("../doc");

//��ʼ��
const string kUNVISITEDURL_FILE_NAME("unvisited.url");
const string kREPLICAS_FILE_NAME("replicas");

//ԭʼ��ҳ��Ϣǰ׺
const string kDATA_FORMAT_FILE_PREFIX("formatfile.raw");
const string kDATA_LINK4SE_FILE_PREFIX("link4sefile.raw");

//�ѷ�����Ϣ
const string kURLMD5_FILE_NAME("visitedurl.md5");
const string kPAGEMD5_FILE_NAME("visitedpage.md5");

const string kLINK4SE_FILE_NAME("link4se.url");
const string kLINK4HISTORY_FILE_NAME("link4history.url");


//����ip
const string kIPBLOCK_FILE_NAME("ipblock");
const string kUNREACHHOST_FILE_NAME("unreachhost.str");

//isam����+����
const string kISAM_DATA_FILE_NAME("isam.data");
const string kISAM_INDEX_FILE_NAME("isam.index");

//gzip�ռ�����
const int kGZIP_LEN=1024000;

//֧�ֽ���link����ҳ����
const vector<string> kPLAIN_TEXT_TYPE = { "text/html" ,"text/plain","text/xml","application/msword",
			 "application/pdf" ,"text/rtf" ,"application/postscript","application/vnd.ms-execl" ,
             "application/vnd.ms-powerpoint" };

//Э��
const vector<string> kHTTP_PROTOCOLS = { "http","https" };

//�汾��
//const string kVERSION_LINK4SE("1.0");
//const string kVERSION_ORIGINAL_PAGE("1.0");

#endif
