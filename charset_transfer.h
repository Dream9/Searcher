/*************************************************************************
	> File Name: charset_transfer.h
	> Author: Dream9
	> Brief: 管理字符集,提供判断和转换功能
	> Created Time: 2019年06月04日 星期五 9时48分45秒
 ************************************************************************/

#ifndef _INDEX_CHARSET_TRANSFER_H_
#define _INDEX_CHARSET_TRANSFER_H_

#include"errlog.h"
#include"strfun.h"
#include"page.h"

#include<cstdlib>
#include<cstdint>

#include<iconv.h>



enum {
	kASCII,
	kUtf8,
	kBig5,
	kUnicode
};

//管理charset,,太恶心人了
class CharsetTransfer {
public:
	static int Is_valid_utf8(char*);
	static int Is_vaild_gbk(char*);
	static int Is_vaild_big5(char*);

	static int FindCharsetFromHeader(const char *, int, string &);
	static int TransferPageCharset(Page *, const char *);

	static int Utf2Gbk(char *, size_t, char *, size_t);
	static int Gbk2Utf(char *, size_t, char *, size_t);
	static int Unicode2Utf(char *, size_t, char *, size_t);
	static int Utf2Unicode(char *, size_t, char *, size_t);
	static int Unicode2Gbk(char *, size_t, char *, size_t);
	static int Gbk2Unicode(char *, size_t, char *, size_t);
//private:
	static int Convert(const char *, const char *, char *, size_t &, char *, size_t &);

	//todo:繁体简化
	//

private:
	static int _find_meta_charset(const char *, string &);

};

#endif
