/*************************************************************************
	> File Name: response.cpp
	> Author: Dream9
	> Brief:
	> Created Time: 2019��06��15�� ������ 15ʱ39��05��
 ************************************************************************/

#include"response.h"

extern const size_t kNUMBER_OF_ANSWER_EACH_PAGE;//ÿҳ��ʾ�����
extern const char *kDEPLOYMENT_ADDRESS;//�����λ��
extern const char *kIMG_ADDRESS;//ͼƬλ��
extern const char *kCGI_QUERY_PATH;//�����ѯcgi·��
extern const char *kCGI_LEFT_PAGE_PATH;//��Ӧʣ���ѯ���cgi

#define ENSURE_EXIST(x) if(!x){continue;}
//brief:չʾ�����������
//parameter:str_query_word:��ѯ����
//          query_and:�����ѯ�Ľ��
//          page_number:��ʾ�ڼ�����ҳ��ȷ����Χ��
//          file_path��raw�ĵ�����
//          doc_idx:������������
int Response::ResponseResult(const char *str_query_word, score_container &query_ans,
	size_t page_number, const char *file_path, forward_index_type &doc_idx) {
	//��ԭʼ�����ļ�
	//todo:mmapʵ��
	std::ifstream ifsData(file_path, ios::in | ios::binary);
	if (!ifsData.is_open()) {
		return -1;
	}
	//��λ��ʾ��Χ
	cout << "<ol>\n";
	size_t pos_start = (page_number - 1)*kNUMBER_OF_ANSWER_EACH_PAGE;
	size_t len = query_ans.size();
	//score_container::iterator iter = query_ans.begin();
	if (pos_start >= len) {
		return -2;
	}
	//std::advance(iter, pos_start);
	len = pos_start + kNUMBER_OF_ANSWER_EACH_PAGE >= len ? len : pos_start + kNUMBER_OF_ANSWER_EACH_PAGE;
	while (pos_start < len) {
		int doc_id = query_ans[pos_start++];
		int data_len = doc_idx[doc_id + 1] - doc_idx[doc_id];
		//��ȡ����
		char *ptrDoc = (char*)calloc(data_len + 1, sizeof(char));
		ifsData.seekg(doc_idx[doc_id]);
		ifsData.read(ptrDoc, data_len);
		//��ȡtitle��url
		char *pos_url_start = strstr(ptrDoc, "url:");
		ENSURE_EXIST(pos_url_start);
		pos_url_start += sizeof "url:" - 1;
		char *pos_url_end = strchr(pos_url_start, '\n');
		ENSURE_EXIST(pos_url_end);
		*pos_url_end = '\0';

		char *pos_title_start = strstr(pos_url_end + 1, "<title>");
		ENSURE_EXIST(pos_title_start);
		pos_title_start += sizeof "<title>" - 1;
		char *pos_title_end = strstr(pos_title_start, "</title>");
		*pos_title_end = '\0';

		cout << "<a href=" << pos_url_start << "><strong>" << pos_title_start << "</strong></a>\n"
			<< "<br><div>\n";

		//�γ�ժҪ
		*pos_title_end = '<';
		_abstract_and_highlight(pos_url_end + 1);

		free(ptrDoc);
		//��ҳ����
		//todo snapshot
		cout << "</div>"
			<< "<a href=/cgi·��/index/snapshot?word=" << str_query_word << "&docid=" << doc_id
			<< " target=_blank>��ҳ����</a><br>\n";
	}

	cout << "</ol><br><br>";
	return 0;
}

//brief:������ҳ�������ݣ�������CSS��ʽ
int Response::ResponseTop(const char *query_string) {
	const char *local_address = getenv("HTTP_HOST");
	cout << "<title>Dream9 Search</title>\n";

	//CSS��ʽ
	cout << "<style>\n";
	cout << "body{color:#333;background:#fff;padding:6px 0 0;margin:0;position:relative;min-width:900px}\n"
		"table{border:0;width:100%;cellspacing:0;height:29;}\n"
		"img{border:0;width:308;height:65;}\n"
		"p{text-align:left;}\n"
		".idx{border:2;}\n"
		"</style>\n";

	//������������
	cout << "<body>\n<table>\n<tr>\n"
		"<td style=width:36%;rowspan:2;height:1><a href=http://" << local_address << kDEPLOYMENT_ADDRESS
		<< "><img src=" << kIMG_ADDRESS << "></a></td>\n"
		"<td style=width:64%;height:33;font-size:2><a href=http://" << local_address << kDEPLOYMENT_ADDRESS
		<< ">������ҳ</a> | <a href=\"\">ʹ�ð���:todo</a><br></td>\n"
		"</tr>" << std::endl;
	//������
	cout << "<tr>\n<td><span><form method=get action=" << kCGI_QUERY_PATH
		<< " name=got>"
		"<input type=text name=word size=55 value=" << query_string << ">\n"
		"<input type=submit value=����>\n"
		"<input type=hidden name=start value=1>\n"//���ر�ǩ
		"</form></span></td></tr>"
		"</table><br><hr><br>" << std::endl;
	return 0;
}

//brief:�·�����ת����
//todo:��Ҫ����
int Response::ResponseSelectPage(int number_result, int time_used, int page_no, const char *word_md5) {
	cout << "<span>Dream9 Ϊ���ҵ�Լ<b>" << number_result << "</b>ƪ�ĵ�,��ʱ<b>"
		<< time_used << "</b>����</span>\n"
		"<table class=idx}><tr>";
	if (page_no > 1) {
		cout << "<td><a href=" << kCGI_LEFT_PAGE_PATH << "?sessionid=" << word_md5 << "&page=" << page_no - 1 << ">��һҳ</a></td>";
	}
	else {
		cout << "<td>��һҳ</td>";
	}
	cout << "<td>&nbsp;&nbsp;&nbsp;&nbsp;</td>"
		"<td><strong>" << page_no << "</strong></td>" 
		"<td>&nbsp;&nbsp;&nbsp;&nbsp;</td>"
		"<td><a href=" << kCGI_LEFT_PAGE_PATH << "?sessionid=" << word_md5 << "&page=" << page_no + 1 << ">��һҳ</a></td>"
		"</table>\n" ;
	//��ҳβ������
	cout << "<br><hr><br>&copy Copyleft Dream9<br><br>\n";
	cout << "</body>\n</html>";
	return 0;
}

//brief:��ҳժҪ���ɣ���������ʾ
//todo:��δ���
int Response::_abstract_and_highlight(const char *str) {
	cout << "todo:<span style=\"color:blue; font - weight:bold\"> ժҪ </span>\n";
	return 0;
}
