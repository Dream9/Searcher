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

	//FILE*f=fopen("ceshi.yuanshi","wb");
	//fwrite(inbuf,sizeof(char),strlen(inbuf),f);
	//fclose(f);

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

//brief:��ͷ������ȡcharset��Ϣ
//becare:Page���Ѿ������˱����ܣ�����
int CharsetTransfer::FindCharsetFromHeader(const char *header, int len_header, string &ret) {
	size_t pos_charset_start = StrFun::FindCase(header, len_header, "charset=", sizeof "charser=" - 1);
	if (string::npos == pos_charset_start) {
		ret.clear();
		return 0;
	}
	pos_charset_start += sizeof "charset=" - 1;
	while (header[pos_charset_start] == ' ') pos_charset_start++;
	const char *ptr_charset_end = strchr(header + pos_charset_start, '\r');
	if (!ptr_charset_end) {
		err_msg("%s:unexpeted end", __FUNCTION__);
		ret.clear();
		return -1;
	}
	ret = string((const char *)header + pos_charset_start, ptr_charset_end);

	//int tmp_pos = ptr_charset_end - header;
	//*(header + tmp_pos) = '\0';
	//ret = header + pos_charset_start;
	//*(header + tmp_pos) = '\r';
	return 0;
}



//brief:ת����ҳ����
int CharsetTransfer::TransferPageCharset(Page *page_parser, const char *target_charset) {
	//����ת��
	if (page_parser->m_sCharset.empty()) {
		//�ȴ���ҳ��meta��Ѱ��
		_find_meta_charset(page_parser->m_sContent.c_str(), page_parser->m_sCharset);
		//
		if (page_parser->m_sCharset.empty()) {
			if (0 == Is_valid_utf8(&(page_parser->m_sContent)[0])){
				page_parser->m_sCharset = "utf-8";
			}
			else if (0 == CharsetTransfer::Is_vaild_gbk(&(page_parser->m_sContent)[0])) {
				page_parser->m_sCharset = "gbk";
			}
			else if (0 == CharsetTransfer::Is_vaild_big5(&(page_parser->m_sContent)[0])) {
				page_parser->m_sCharset = "big5";
				//todo:��Ҫת������������
			}
			else {
				log_msg("%s:unsolved charset", __FUNCTION__);
				return -1;
			}
		}
	}
	//log_msg("charset:%s--to %s",page_parser->m_sCharset.c_str(),target_charset);
	if (string::npos != StrFun::FindCase(page_parser->m_sCharset, "gb")) {
		//����ת��
		return 0;
	}
	//ת����ʽ
	size_t max_len = page_parser->m_sContent.size() << 1;
	size_t len_source_info = page_parser->m_sContent.size();
	char *buf = (char *)malloc(max_len);
	if (0 != Convert(page_parser->m_sCharset.c_str(), target_charset, &(page_parser->m_sContent)[0], len_source_info, buf, max_len)) {
		err_msg("%s:Convert() error", __FUNCTION__);
		free(buf);
		buf = nullptr;
		return -1;
	}
	if (len_source_info > 0) {
		err_msg("Infomation may lost");
	}
	page_parser->m_sContent = buf;
	free(buf);
	buf = nullptr;

	return 0;
}



//brief:��������������ҳ��<head>...</head>��ǩ����ȡcharset��Ϣ
int CharsetTransfer::_find_meta_charset(const char *source, string &charset) {
	if (!source) {
		log_msg("warning:nullptr");
		return 0;
	}
	const char *ptr_pos_end = strstr(source, "</head>");
	if (!ptr_pos_end) {
		err_msg("%s:invalid format without </headt> tag", __FUNCTION__);
		return -1;
	}
	size_t pos_end = ptr_pos_end - source;
	size_t pos_charset = 0;
	//���Ͻ�һ��Ļ�Ӧ���жϱ�֤λ��<meta>��ǩ��
	pos_charset = StrFun::FindCase(source + pos_charset, pos_end - pos_charset, "charset=", sizeof "charset=" - 1);
	if (pos_charset == string::npos) {
		return 0;
	}
	pos_charset += sizeof"charset=" - 1;
	//����ո�
	while (*(source + pos_charset) == ' ') {
		++pos_charset;
	}
	ptr_pos_end = source + pos_charset;
	while (*ptr_pos_end) {
		char cur = *ptr_pos_end;
		if (cur == '\"' || cur == '\r' || cur == '\n' || cur == '>' || cur == ';') {
			break;
		}
		++ptr_pos_end;
	}
	//pos_end = source.find_first_of("\"> ;\r\n");
	if (!*ptr_pos_end) {
		charset.clear();
		return 0;
	}
	charset = string(source + pos_charset, ptr_pos_end);
	return 0;
}
