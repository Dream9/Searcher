/*************************************************************************
	> File Name: charset_transfer.h
	> Author: Dream9
	> Brief: �����ַ���,�ṩ�жϺ�ת������
	> Created Time: 2019��06��04�� ������ 9ʱ48��45��
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

//����charset,,̫��������
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

	//todo:�����
	//

private:
	static int _find_meta_charset(const char *, string &);

};

#endif
