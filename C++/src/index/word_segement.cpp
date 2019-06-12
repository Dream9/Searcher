/*************************************************************************
	> File Name: word_segment.cpp
	> Author: Dream9
	> Brief: 
	> Created Time: 2019年06月03日 星期五 15时11分10秒
 ************************************************************************/

#include"word_segement.h"

//分隔符
extern const char kSEPARATE_CHAR;
//最大步长
extern const int kMAX_WORD_LENGTH;

//特别的几个ACSII码
static const uint8_t SPACE = 32;
static const uint8_t CR = 13;
static const uint8_t LF = 10;
static const uint8_t HYPHER = 45;

//中文汉字第一个位置的首字节
static const uint8_t kGBK_FIRST_CHARACTER_BYTE = 0xB0;
//中文符号第一个位置的首字节
static const uint8_t kGBK_FIRST_SPECIAL_BYTE = 0xA1;
//ASCII的界限
static const uint8_t kASCII_MAX = 128;

static inline int _MIN(int a, int b) { return a < b ? a : b; }
//brief:将url中的特殊字符转换出来
//becare:URL编码采用百分号编码方式，十六进制格式
//return:转换后的结果记录在ptrUrl原本指向空间
int WordSegment::TranslateURLChar(char *ptrUrl) const {
	char *ptrStr;
	char *ptrCur;
	char *leader = ptrUrl;
	int len = strlen(ptrUrl);
	int ret=0;
	
	ptrStr = (char *)calloc(sizeof(char), len + 1);
	if (ptrStr) {
		err_ret("calloc failed when %s", __FUNCTION__);
		return -1;
	}

	ptrCur = ptrStr;
	while (*ptrUrl) {
		if (*ptrUrl != '%') {
			*ptrCur++ = *ptrUrl++;
			continue;
		}
		//处理百分号编码，字符范围：[0-9,A-F]
		if (*(ptrUrl+1) && *(ptrUrl+2)) {
            int tmp_num;
	        char tmp_buf[3];
			tmp_buf[0] = *(++ptrUrl);
			tmp_buf[1] = *(++ptrUrl);
			tmp_buf[2] = '\0';
			if (1 != sscanf(tmp_buf, "%3x", &tmp_num)) {
				err_ret("Sscanf failed:%s", tmp_buf);
				ret = -1;
				goto FREE_BUF;
			}
			*ptrCur++ = tmp_num;
		}
		else {
			err_msg("Encode Url failed");
			ret = -1;
			goto FREE_BUF;
		}
	}
	*ptrCur = '\0';
	memmove(leader, ptrStr, len + 1);
	
FREE_BUF:
	if (ptrStr) {
		free(ptrStr);
		ptrStr = nullptr;
	}
	return ret;
}


//brief:需要预处理的分词，转调用Segment
//becare:1.这里的中文编码采用gb2312(gbk兼容)
//       2.需要提取网页正文信息之后进行切词
//return:切词结果
string WordSegment::Segment_need_preprose(Dictinary &dict, const string & strDoc) const {
	//todo:
	//编码识别与转换
	string str_out;
	int iter = 0;
	int len = strDoc.size();
	while (iter < len) {
		uint8_t cur=(uint8_t)strDoc[iter];
		//处理英文分词
		while (cur < kASCII_MAX) {
			if ((cur >= 'A' && cur <= 'Z')
				|| (cur >= 'a' && cur <= 'z')
				|| (cur >='0' && cur <='9')) {
				str_out += strDoc[iter];
			}
			else{
				//特殊字符
				switch (cur) {
					//case SPACE:
					////	str_out += kSEPARATE_CHAR; 
					////	break;
					////	//针对\r换行
					case CR:
						//防止重复
						if(str_out.empty() || str_out.back() == kSEPARATE_CHAR){
							break;
						}
						str_out += kSEPARATE_CHAR; 
						if ((uint8_t)strDoc[iter + 1] == LF) {
							++iter;
						}
						break;
						
						//针对\n换行
						////case LF:
						////	str_out += kSEPARATE_CHAR;
						////	break;
						//
						//针对连字符
					case HYPHER:
						if ((uint8_t)strDoc[iter + 1] == CR) {
							++iter;
						}
						if ((uint8_t)strDoc[iter + 1] == LF) {
							++iter;
						}
						break;
						
					default:
						//防止重复
						if(str_out.empty() || str_out.back() == kSEPARATE_CHAR){
							break;

						}
						str_out += kSEPARATE_CHAR;
						break;
				}
			}
			//继续计算
			++iter;
			if(iter >= len){
				break;
			}
			cur = (uint8_t)strDoc[iter];
		}
		if(!str_out.empty() && str_out.back() != kSEPARATE_CHAR){
			str_out += kSEPARATE_CHAR;//处理一下特殊情况
		}

		//此时的字符应该为gb2312中的特有字符
		//标点符号没有任何意义，全部跳过
		while(iter < len && (uint8_t)strDoc[iter] < kGBK_FIRST_CHARACTER_BYTE){
			iter += 2;
		}
		//中文分词
		int tmpi = iter;
		while (tmpi < len && tmpi + 1 <len && (uint8_t)strDoc[tmpi] >= kGBK_FIRST_CHARACTER_BYTE) {
			tmpi += 2;
		}
		str_out += Segment(dict, strDoc.substr(iter, tmpi - iter));
		iter = tmpi;
	}
	return str_out;
}


//brief:中文分词
//becare:1.不包含特殊符号（除了中文标点和kSEPARATE_CHAR）
//       2.返回结果通过kSEPARATE_CHAR分隔
string WordSegment::Segment(Dictinary &dict, const string &strDoc) const {
	int len = strDoc.size();
	int iter = 0;
	string str_out;
	
	while (iter < len) {
		//处理英文分词
		while (iter < len && (uint8_t)strDoc[iter] < kASCII_MAX) {
			while (iter < len && strDoc[iter] == kSEPARATE_CHAR) {//定位到开始位置
				++iter;
			}
			int pos_start = iter;
			while (iter < len && strDoc[iter] != kSEPARATE_CHAR && (uint8_t)strDoc[iter] < kASCII_MAX) {//定位到分隔位置
				++iter;
			}
			str_out += strDoc.substr(pos_start, iter - pos_start) + kSEPARATE_CHAR;
			iter = strDoc[iter] == kSEPARATE_CHAR ? iter + 1 : iter;
		}

		//防止仍然有特殊字符
		while (iter < len && (uint8_t)strDoc[iter] < kGBK_FIRST_SPECIAL_BYTE) {
			iter += 2;
		}

		//处理中文分词
		while (iter < len && (uint8_t)strDoc[iter] >=kGBK_FIRST_SPECIAL_BYTE) {
			int pos_start = iter;
			while (iter < len && strDoc[iter] != kSEPARATE_CHAR && (uint8_t)strDoc[iter] >= kGBK_FIRST_SPECIAL_BYTE) {//定位到分隔位置
				iter += 2;
			}
			string str_tmp;
			_segment_rmm(dict, strDoc.substr(pos_start, iter - pos_start), iter - pos_start, str_tmp);
			str_out += str_tmp;

			iter = strDoc[iter] == kSEPARATE_CHAR ? iter + 1 : iter;
		}
	}
	return str_out;
}

//brief:采用正向最大减字法分词
//parameter:字典，待分词串a，a的长度，结果串
//becare:完全的中文分词，不包括任何其他字符
int WordSegment::_segment_mm(Dictinary &dict, const string &target, int len, string &ret) const {
	int iter = 0;
	while (iter < len) {
		int word_step = _MIN(kMAX_WORD_LENGTH, len - iter);
		if (word_step & 0x1) {
			err_msg("_segment_mm() error:Unexcepted end");
			iter = len;
			break;
		}
		string strWord = target.substr(iter, word_step);
		while (word_step > 2) {
			if (true == dict.IsWord(strWord)) {
				break;
			}
			word_step -= 2;
			strWord.pop_back();
			strWord.pop_back();
		}
		ret += strWord + kSEPARATE_CHAR;
		iter += word_step;
	}
	return 0;
}

//brief:采用逆向最大减字法分词
//parameter:字典，待分词串a，a的长度, 结果串
//becare:完全的中文
int WordSegment::_segment_rmm(Dictinary &dict, const string &target, int len, string &ret) const {
    int iter = len;
	while (iter > 0) {
		int word_step = _MIN(kMAX_WORD_LENGTH, iter);
		if (word_step & 0x1) {
			err_msg("_segment_rmm() error:Unexcepted end");
			iter = len;
			break;
		}
		string strWord = target.substr(iter-word_step, word_step);
		//逆向寻找
		while (word_step > 2) {
			if (true == dict.IsWord(strWord)) {
				break;
			}
			word_step -= 2;
			strWord = target.substr(iter - word_step, word_step);
		}
		//ret += strWord + kSEPARATE_CHAR;
		ret = strWord + kSEPARATE_CHAR + ret;
		iter -= word_step;
	}
	return 0;
}

//brief:网页元素提炼
//todo:标签信息记录下来，用于评分
int  WordSegment::_refine_document(const string &strDoc, string &ret) const {
	size_t iter = 0;
	size_t len = strDoc.size();
	while (iter<len) {
		if (strDoc[iter] == '<') {
			//while (iter < len && strDoc[iter] != '>') {
			//	++iter;
			//}
			//if (iter == len) {
			//	return 0;
			//}
		    iter = strDoc.find_first_of('>', iter);
			if (string::npos == iter) {
				return 0;
			}
		}
		else {
			//ret += strDoc[iter];
			size_t pos_end = strDoc.find_first_of('<', iter);
			ret += (string::npos == pos_end) ? strDoc.substr(iter) : strDoc.substr(iter, pos_end - iter);
			iter = (string::npos == pos_end) ? len : pos_end;
		}
		++iter;
	}
	return 0;
}

//brief:todo 切分url
int WordSegment::SegmentUrl(Dictinary &dict, string &url) const {
	char *translate_url = (char*)url.c_str();
	TranslateURLChar(translate_url);
	url = translate_url;
	return 0;
}

