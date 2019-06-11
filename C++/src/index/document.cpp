/*************************************************************************
	> File Name: document.h
	> Author: Dream9
	> Brief:
	> Created Time: 2019��05��31�� ������ 15ʱ23��08��
 ************************************************************************/
#include"document.h"


static int _find_meta_charset(const char *source, string &charset);
static void _find_next_tag(const char**, const char*); 

//brief:������ҳ��Ϣ
//todo:��ǩ��Ϣ��¼��������������
int Document::Refine(const char *doc, string &charset) {
	m_sContent_Refined.clear();

	const string kTAG[] = {"</script","</style"};
	//��Ϣ��
	const char *iter = doc;
	while (iter && *iter) {
		//������ǩ,  todo:һЩ�����ǩ֮�������ҲӦ����ȫ���ˣ�����<script>222</script>
		//
		if (*iter == '<') {
			const char *next_iter = strchr(iter,'>');
			//���������ǩ
			for(const string &tag:kTAG){
				if(0 == memcmp(iter + 1, &tag[2], tag.size() - 2)){
					_find_next_tag(&next_iter, tag.c_str());
					break;
				}
			}

			if (!next_iter) {
				return 0;
			}
			iter = next_iter + 1;
		}
		else {
			//ret += strDoc[iter];
			const char *pos_end = strchr(iter, '<');
			m_sContent_Refined += (!pos_end) ? iter : string(iter, pos_end);
			iter = (!pos_end) ? nullptr : pos_end;
			continue;
		}
	}

	//����ת��
	if (charset.empty()) {
		//�ȴ���ҳ��meta��Ѱ��
		_find_meta_charset(doc, charset);
		//
		if (charset.empty()) {
			if (0 == CharsetTransfer::Is_valid_utf8(&m_sContent_Refined[0])) {
				charset = "utf-8";
			}
			else if (0 == CharsetTransfer::Is_vaild_gbk(&m_sContent_Refined[0])) {
				charset = "gbk";
			}
			else if(0 == CharsetTransfer::Is_vaild_big5(&m_sContent_Refined[0])){
				charset="big5";
				//todo:��Ҫת������������
			}
			else {
				err_msg("%s:unsolved charset", __FUNCTION__);
				return -1;
			}
		}
	}
	if (string::npos != StrFun::FindCase(charset, "gb")) {
		//����ת��
		return 0;
	}
	size_t max_len = m_sContent_Refined.size() << 1;
	size_t len_source_info = m_sContent_Refined.size();
	char *buf = (char *)malloc(max_len);
	if (0 != CharsetTransfer::Convert(charset.c_str(), "gbk", &m_sContent_Refined[0], len_source_info, buf, max_len)) {
		err_msg("%s:Convert() error",__FUNCTION__);
		free(buf);
		buf = nullptr;
		return -1;
	}
	if (len_source_info > 0) {
		err_msg("Infomation may lost");
	}
	m_sContent_Refined = buf;
	free(buf);
	buf = nullptr;

	return 0;
}

//brief:��������������ҳ����ȡcharset��Ϣ
int _find_meta_charset(const char *source, string &charset) {
	if(!source){
		log_msg("warning:nullptr");
		return 0;
	}
	const char *ptr_pos_end = strstr(source,"</head>");
	if(!ptr_pos_end){
		err_msg("%s:invalid format without </headt> tag",__FUNCTION__);
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
	while(*ptr_pos_end){
		char cur=*ptr_pos_end;
		if(cur=='\"' || cur=='\r' || cur=='\n' || cur=='>' || cur==';'){
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


//brief:��������
//parameter:start_tag_tail:��ʼ��ǩ��β��ָ��,tag:"</"+��ǩ����ֵ
void _find_next_tag(const char **start_tag_tail, const char *tag){
	*start_tag_tail = strstr(*start_tag_tail, tag);
	if(*start_tag_tail){
		*start_tag_tail = strchr(*start_tag_tail, '>');
	}
}
