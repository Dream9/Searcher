/*************************************************************************
	> File Name: word_segment.cpp
	> Author: Dream9
	> Brief: 
	> Created Time: 2019��06��03�� ������ 15ʱ11��10��
 ************************************************************************/

#include"word_segement.h"

//�ָ���
extern const char kSEPARATE_CHAR;
//��󲽳�
extern const int kMAX_WORD_LENGTH;

//�ر�ļ���ACSII��
static const uint8_t SPACE = 32;
static const uint8_t CR = 13;
static const uint8_t LF = 10;
static const uint8_t HYPHER = 45;

//���ĺ��ֵ�һ��λ�õ����ֽ�
static const uint8_t kGBK_FIRST_CHARACTER_BYTE = 0xB0;
//���ķ��ŵ�һ��λ�õ����ֽ�
static const uint8_t kGBK_FIRST_SPECIAL_BYTE = 0xA1;
//ASCII�Ľ���
static const uint8_t kASCII_MAX = 128;

static inline int _MIN(int a, int b) { return a < b ? a : b; }
//brief:��url�е������ַ�ת������
//becare:URL������ðٷֺű��뷽ʽ��ʮ�����Ƹ�ʽ
//return:ת����Ľ����¼��ptrUrlԭ��ָ��ռ�
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
		//����ٷֺű��룬�ַ���Χ��[0-9,A-F]
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


//brief:��ҪԤ����ķִʣ�ת����Segment
//becare:1.��������ı������gb2312(gbk����)
//       2.��Ҫ��ȡ��ҳ������Ϣ֮������д�
//return:�дʽ��
string WordSegment::Segment_need_preprose(Dictinary &dict, const string & strDoc) const {
	//todo:
	//����ʶ����ת��
	string str_out;
	int iter = 0;
	int len = strDoc.size();
	while (iter < len) {
		uint8_t cur=(uint8_t)strDoc[iter];
		//����Ӣ�ķִ�
		while (cur < kASCII_MAX) {
			if ((cur >= 'A' && cur <= 'Z')
				|| (cur >= 'a' && cur <= 'z')
				|| (cur >='0' && cur <='9')) {
				str_out += strDoc[iter];
			}
			else{
				//�����ַ�
				switch (cur) {
					//case SPACE:
					////	str_out += kSEPARATE_CHAR; 
					////	break;
					////	//���\r����
					case CR:
						//��ֹ�ظ�
						if(str_out.empty() || str_out.back() == kSEPARATE_CHAR){
							break;
						}
						str_out += kSEPARATE_CHAR; 
						if ((uint8_t)strDoc[iter + 1] == LF) {
							++iter;
						}
						break;
						
						//���\n����
						////case LF:
						////	str_out += kSEPARATE_CHAR;
						////	break;
						//
						//������ַ�
					case HYPHER:
						if ((uint8_t)strDoc[iter + 1] == CR) {
							++iter;
						}
						if ((uint8_t)strDoc[iter + 1] == LF) {
							++iter;
						}
						break;
						
					default:
						//��ֹ�ظ�
						if(str_out.empty() || str_out.back() == kSEPARATE_CHAR){
							break;

						}
						str_out += kSEPARATE_CHAR;
						break;
				}
			}
			//��������
			++iter;
			if(iter >= len){
				break;
			}
			cur = (uint8_t)strDoc[iter];
		}
		if(!str_out.empty() && str_out.back() != kSEPARATE_CHAR){
			str_out += kSEPARATE_CHAR;//����һ���������
		}

		//��ʱ���ַ�Ӧ��Ϊgb2312�е������ַ�
		//������û���κ����壬ȫ������
		while(iter < len && (uint8_t)strDoc[iter] < kGBK_FIRST_CHARACTER_BYTE){
			iter += 2;
		}
		//���ķִ�
		int tmpi = iter;
		while (tmpi < len && tmpi + 1 <len && (uint8_t)strDoc[tmpi] >= kGBK_FIRST_CHARACTER_BYTE) {
			tmpi += 2;
		}
		str_out += Segment(dict, strDoc.substr(iter, tmpi - iter));
		iter = tmpi;
	}
	return str_out;
}


//brief:���ķִ�
//becare:1.������������ţ��������ı���kSEPARATE_CHAR��
//       2.���ؽ��ͨ��kSEPARATE_CHAR�ָ�
string WordSegment::Segment(Dictinary &dict, const string &strDoc) const {
	int len = strDoc.size();
	int iter = 0;
	string str_out;
	
	while (iter < len) {
		//����Ӣ�ķִ�
		while (iter < len && (uint8_t)strDoc[iter] < kASCII_MAX) {
			while (iter < len && strDoc[iter] == kSEPARATE_CHAR) {//��λ����ʼλ��
				++iter;
			}
			int pos_start = iter;
			while (iter < len && strDoc[iter] != kSEPARATE_CHAR && (uint8_t)strDoc[iter] < kASCII_MAX) {//��λ���ָ�λ��
				++iter;
			}
			str_out += strDoc.substr(pos_start, iter - pos_start) + kSEPARATE_CHAR;
			iter = strDoc[iter] == kSEPARATE_CHAR ? iter + 1 : iter;
		}

		//��ֹ��Ȼ�������ַ�
		while (iter < len && (uint8_t)strDoc[iter] < kGBK_FIRST_SPECIAL_BYTE) {
			iter += 2;
		}

		//�������ķִ�
		while (iter < len && (uint8_t)strDoc[iter] >=kGBK_FIRST_SPECIAL_BYTE) {
			int pos_start = iter;
			while (iter < len && strDoc[iter] != kSEPARATE_CHAR && (uint8_t)strDoc[iter] >= kGBK_FIRST_SPECIAL_BYTE) {//��λ���ָ�λ��
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

//brief:�������������ַ��ִ�
//parameter:�ֵ䣬���ִʴ�a��a�ĳ��ȣ������
//becare:��ȫ�����ķִʣ��������κ������ַ�
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

//brief:�������������ַ��ִ�
//parameter:�ֵ䣬���ִʴ�a��a�ĳ���, �����
//becare:��ȫ������
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
		//����Ѱ��
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

//brief:��ҳԪ������
//todo:��ǩ��Ϣ��¼��������������
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

//brief:todo �з�url
int WordSegment::SegmentUrl(Dictinary &dict, string &url) const {
	char *translate_url = (char*)url.c_str();
	TranslateURLChar(translate_url);
	url = translate_url;
	return 0;
}

