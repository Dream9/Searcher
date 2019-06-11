/*************************************************************************
	> File Name: charset_transfer.h
	> Author: Dream9
	> Brief: �����ַ���
	> Created Time: 2019��06��04�� ������ 10ʱ09��13��
 ************************************************************************/


#include"charset_transfer.h"

static const uint8_t kTOKEN[] = { 0x80,0x20,0x10,0x08,0x04 };
static const int kLEN_TOKEN = sizeof(kTOKEN) / sizeof(uint8_t);

//brief:����iconv��from_charsetת����to_charset
//becare:����ʱ�������inlen outlen����Ϣ�����·�ʽ����
/*
       The iconv() function converts one multibyte character at a time, and
	   for each character conversion it increments *inbuf and decrements
	   *inbytesleft by the number of converted input bytes, it increments
	   *outbuf and decrements *outbytesleft by the number of converted
	   output bytes, and it updates the conversion state contained in cd.
	   If the character encoding of the input is stateful, the iconv()
	   function can also convert a sequence of input bytes to an update to
	   the conversion state without producing any output bytes; such input
	   is called a shift sequence.
       ������Ϣ�μ�Linux Programmer's Manual
*/
int CharsetTransfer::Convert(const char *from_charset, const char *to_charset, char *inbuf, size_t &inlen,
	char *outbuf, size_t &outlen) {
	iconv_t cd;
	char **pin = &inbuf;
	char **pout = &outbuf;

	//����
	cd = iconv_open(to_charset, from_charset);
	if (cd < 0) {
		err_msg("iconv_open() faied");
		return -1;
	}
	memset(outbuf, 0, outlen);
	//ת��
	if (iconv(cd, pin, &inlen, pout, &outlen) == (size_t)-1) {
		err_msg("iconv() failed");
		return -2;
	}
	
	//�ر���
	iconv_close(cd);
	**pout = '\0';
	return 0;
}

//brief:ת���ӿ�
int CharsetTransfer::Utf2Gbk(char *inbuf, size_t inlen, char *outbuf, size_t outlen) {
	return Convert("utf-8", "gbk", inbuf, inlen, outbuf, outlen);
}

//brief:ת���ӿ�
int CharsetTransfer::Gbk2Utf(char *inbuf, size_t inlen, char *outbuf, size_t outlen) {
	return Convert("gbk", "utf-8", inbuf, inlen, outbuf, outlen);
}

//
//brief:ת���ӿ�
int CharsetTransfer::Unicode2Utf(char *inbuf, size_t inlen, char *outbuf, size_t outlen) {
	return Convert("unicode", "utf-8", inbuf, inlen, outbuf, outlen);
}

//brief:ת���ӿ�
int CharsetTransfer::Utf2Unicode(char *inbuf, size_t inlen, char *outbuf, size_t outlen) {
	return Convert("utf-8", "unicode", inbuf, inlen, outbuf, outlen);
}

//brief:ת���ӿ�
int CharsetTransfer::Unicode2Gbk(char *inbuf, size_t inlen, char *outbuf, size_t outlen) {
	return Convert("unicode", "gbk", inbuf, inlen, outbuf, outlen);
}

//brief:ת���ӿ�
int CharsetTransfer::Gbk2Unicode(char *inbuf, size_t inlen, char *outbuf, size_t outlen) {
	return Convert("gbk", "unicode", inbuf, inlen, outbuf, outlen);
}


//brief:�Ƿ�Ϊutf8����
//becare:�����ж��Ǳ�Ҫ���������
//      ����RFC2639�Լ�ISO10646��׼
//return:0:true  -1:false   -2:error
int CharsetTransfer::Is_valid_utf8(char *source) {
	if (source) {
		return -2;
	}
	while (*source) {
		//����ֽڳ���
		int len = 0;
		for (; len < kLEN_TOKEN; ++len) {
			if ((kTOKEN[len] & *source) == 0) {
				break;
			}
			++len;
		}
		//ISO10646Ҫ�����ʹ��4�ֽڱ���
		if (len == 4) {
			return -1;
		}
		//�жϺ����ֽ��Ƿ�Ϊ0x80
		while (len > 0) {
			++source;
			if (*source || (*source & 0xC0) != 0x80) {
				return -1;
			}
			--len;
		}
		++source;
	}

	return 0;
}

//brief:
//return:0:true   -1:false   -2:error
int CharsetTransfer::Is_vaild_gbk(char *source) {
	return 0;
	//char *buf = strdup(source);
	//if (buf) {
	//	err_ret("strdup() failed");
	//	return -2;
	//}
	int len = strlen(source);
	char *buf = (char *)calloc(sizeof(char), len << 1);
	if (0 != Gbk2Unicode(source, len, buf, len)) {
		err_msg("Gbk2Unicode() failed in Is_valid_gdk");
		return -2;
	}
}


//brief:todo
int CharsetTransfer::Is_vaild_big5(char *source) {
	return 0;
}
