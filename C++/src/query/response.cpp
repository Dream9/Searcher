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


//���ĺ��ֵ�һ��λ�õ����ֽ�
static const uint8_t kGBK_FIRST_CHARACTER_BYTE = 0xB0;
//ASCII�Ľ���
static const uint8_t kASCII_MAX = 128;
//��̬ժҪ����
static const int kABSTRACT_LENGTH = 256;
static const char kSEPARATE_ABSTRACT_PARAGRAPHY='/';

static int _find_end_position_of_abstract(const char *strRefine, const char *strRefineLast, const char *cur);
static int _find_start_position_of_abstract(const char *strRefine, const char *cur);
static bool _is_trival(char c);

#define ENSURE_EXIST(x) if(!x){continue;}
//brief:չʾ�����������
//parameter:str_query_word:��ѯ����
//          query_and:�����ѯ�Ľ��
//          page_number:��ʾ�ڼ�����ҳ��ȷ����Χ��
//          file_path��raw�ĵ�����
//          doc_idx:������������
//          words:��ѯ�ʵķָ�����
//          idf:ÿ����ѯ�ʵ�idfֵ
int Response::ResponseResult(const char *str_query_word, score_container &query_ans,
	size_t page_number, const char *file_path, forward_index_type &doc_idx,
	vector<string> &words, vector<float> &idf) {
	//��ԭʼ�����ļ�
	//todo:mmapʵ��
	std::ifstream ifsData(file_path, ios::in | ios::binary);
	if (!ifsData.is_open()) {
		return -1;
	}
	//��λ��ʾ��Χ
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
		//*pos_title_end = '\0';
		//�Ա������ɸѡ
		string title_tmp(pos_title_start,pos_title_end);
		title_tmp.erase(remove_if(title_tmp.begin(),title_tmp.end(),_is_trival),title_tmp.end());
		if(title_tmp.size()<3){
			pos_title_start=strstr(pos_title_end,"title=");
			if(pos_title_start){
				pos_title_start+=sizeof "title=" -1;
				
				while(*pos_title_start=='\"' || *pos_title_start==' '){
					++pos_title_start;
				}
				pos_title_end=pos_title_start;
				while(*pos_title_end && *pos_title_end!=' ' && *pos_title_end!='\"' && *pos_title_end!='>'){
					++pos_title_end;
				}
				string(pos_title_start,pos_title_end).swap(title_tmp);
			}
			
			if(title_tmp.size()<3){
				title_tmp=str_query_word;
			}
		}

		cout << "<a href=" << pos_url_start << "><strong>" << title_tmp<< "</strong></a>\n"
			<< "<br><div>\n";

		//�γ�ժҪ
		//*pos_title_end = '<';
#ifdef DEBUG_1
		cout<<"\nceshi:\n"<<pos_url_start;
#endif
		_abstract_and_highlight(pos_title_end, words, idf);

		free(ptrDoc);
		//��ҳ����
		//todo snapshot
		cout << "</div>\n"
			"<a href=/cgi-bin/todo-snapshot?word=" << str_query_word << "&docid=" << doc_id
			<<" target=_blank>��ҳ����</a><br><br>\n";
	}

	cout << "<br><br>";
	return 0;
}

//brief:������ҳ�������ݣ�������CSS��ʽ
int Response::ResponseTop(const char *query_string) {
	//const char *local_address = getenv("HTTP_HOST");
	//ͷ��
	cout<<"Content-type:text/html\r\n\r\n"
		"<html><head>\n"
		"<meta charset=\"gbk\">\n"
		"<title>Dream9 Search</title>\n";

	//CSS��ʽ
	cout << "<style>\n";
	cout << "body{color:#333;background:#fff;padding:6px 0 0;margin:0;position:relative;min-width:900px}\n"
		"table{border:0;width:100%;cellspacing:0;height:29;align=center;}\n"
		"img{border:0;width:308;height:65;display:block;margin:auto}\n"
		"p{text-align:left;}\n"
		".idx{font-size:16px;}\n"
		".f{align:center;width=100%;height=25;}\n"
		"</style>\n"
		"</head>\n";

	//������������
	cout << "<body>\n<table>\n<tr>\n"
		"<td style=width:36%;rowspan:2;height:1><a href=http://" << kDEPLOYMENT_ADDRESS
		<< "><img src=" << kIMG_ADDRESS << "></a></td></tr>\n"
		"<tr><td style=\"width:64%;height:33;font-size:12;text-align:center;vertical-align:middle;\"><a href=http://" << kDEPLOYMENT_ADDRESS
		<< ">������ҳ</a> | <a href=\"\">ʹ�ð���:</a><br></td>\n"
		"</tr>" << std::endl;
	//������
	string tmp_str(query_string);
	replace(tmp_str.begin(),tmp_str.end(),kSEPARATE_CHAR,' ');
	cout << "<tr style=\"text_align:center;vertical-align:middle;\">\n<td><span><form method=get action=" << kCGI_QUERY_PATH
		<< " name=got class=f>"
		"<input type=text name=word size=55 value=" << tmp_str << ">\n"
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
	cout << "<br><hr><br><p style=text-align:center;>&copy Copyleft Dream9</p><br><br>\n";
	cout << "</body>\n</html>";
	return 0;
}

//brief:��ҳժҪ���ɣ���������ʾ
//     �����Ļ��ڻ������ڵ�����ժҪѡȡ�㷨��ά��map<������ʼʵposλ��,pair<���䳤�ȣ�����Ȩֵ>>
//     ���ж����������(���裺�����������һ�������ҵ��˶�����ʼ�����ǳ��ȳ���)
//     ������Ȩֵѡ������ժҪ
int Response::_abstract_and_highlight(const char *str, vector<string> &words, vector<float> &idf) {
	cout << "<span style=\"color:blue; font - weight:bold\"> ժҪ </span>\n";
	//
	Document doc_parser;
	doc_parser.RefineWithSeparate(str);
	//
	const char *strRefine = doc_parser.m_sContent_Refined.c_str();
#ifdef DEBUG_1
	cout<<'\n'<<strRefine<<'\n';
#endif
	const char *strRefineLast = strRefine + doc_parser.m_sContent_Refined.size();

	//2019.6.21�޸Ĳ��ԣ���������quickUnion��ʵ�ַ�ʽ
	typedef std::pair<int, float> weight_type;//�γ���Ȩֵ
	unordered_map<int, weight_type>mapScore;//������ʼλ�ã�������Ϣ
	vector<std::pair<int,int>>vecQuickUnion;//��������С����λ�ã��ô������Ķ�����ʼλ��

	if(words.size() != idf.size()){
		cout<<"<span>error,data lost</span>";
		return -1;
	}
	size_t iter = 0;
	size_t len = words.size();
	float max_weight = -9999;
	int max_weight_position = -1;
	for (; iter < len; ++iter) {
		string &cur_word = words[iter];
		const char *pos=strRefine;
		//�ҵ�һ����
		while ((pos = strstr(pos, cur_word.c_str()))) {
			//��λ�����俪ʼλ��
			//����λ����ǰ��һ�� (�Ǻ����ַ� && ��Ӣ����ĸ && ������) ��λ��
			int idx_start = _find_start_position_of_abstract(strRefine, pos);
			//��λ���������λ��
			pos += cur_word.size();
			int idx_last = _find_end_position_of_abstract(strRefine, strRefineLast, pos);

			int length=idx_last-idx_start;
			if(length>=kABSTRACT_LENGTH || vecQuickUnion.empty()){
				//�㹻��
				vecQuickUnion.emplace_back(std::make_pair(idx_start,idx_start));
				mapScore.insert({idx_start,{length,idf[iter]}});
				if(idf[iter]>max_weight){
					max_weight=idf[iter];
					max_weight_position=idx_start;
				}
			}
			else{
				//��Ҫ��ǰ��ĺϲ�
				int _start=vecQuickUnion.back().second;
				auto _iter=mapScore.find(_start);
				if((*_iter).second.first<kABSTRACT_LENGTH){
					//�ϲ�
					vecQuickUnion.emplace_back(std::make_pair(idx_start,_start));
					(*_iter).second.first=idx_last - _start;
					(*_iter).second.second+=idf[iter];
					if((*_iter).second.second>max_weight){
						max_weight=idf[iter];
						max_weight_position=_start;
					}
				}
				else{
					//ǰ������,���ܺϲ�
					vecQuickUnion.emplace_back(std::make_pair(idx_start,idx_start));
					mapScore.insert({idx_start,{length,idf[iter]}});
					if(idf[iter]>max_weight){
						max_weight=idf[iter];
						max_weight_position=idx_start;
					}
				}
			}

			//auto i = mapScore.find(idx_start);
			//if (i == mapScore.end()) {
			//	mapScore.insert({ idx_start,weight_type(idx_last - idx_start,idf[iter]) });
			//	if (idf[iter] > max_weight) {
			//		max_weight_position = idx_start;
			//	}
			//}
			//else {
			//	mapScore[idx_start].second+= idf[iter]; 
			//	if (mapScore[idx_start].second > max_weight) {
			//		max_weight_position = idx_start;
			//	}
			//	
			//}
		}
	}
	//��������������Ҳ���Ǳ��⵳���������ڱ����г���������ؼ��֣�֮����Ҳû�г��֡�����
	if (max_weight_position == -1) {
		cout << "<span>This page is <strong>just a title</strong> ! Kill him!</span>";
		return -1;
	}
	//��̬ժҪ����
	//���ݳ�����������Ҫ����չ��������������ʾ
	len = mapScore[max_weight_position].first;
	if (len < kABSTRACT_LENGTH) {
		int pos = max_weight_position;//Ϊ��չǰ����beginλ��
		int pos_end = max_weight_position + len;
		//��ǰ��չ
		--pos;
		while (pos >= 0 && strRefine[pos] != '/') {
			--pos;
		}

		max_weight_position = pos == 0 ? 0 : pos + 1;//����beginλ��,��ֹ����
		//�����չ
		int lastone = doc_parser.m_sContent_Refined.size();
		while (pos_end < lastone && strRefine[pos_end] != '/') {
			++pos;
		}
		len = pos_end - max_weight_position;		
	}
	//����ժҪ
	string out(strRefine + max_weight_position, len);
	//ȥ������
    out.erase(remove_if(out.begin(),out.end(),_is_trival),out.end());
	//������ʾ
	for (string &cur : words) {
		size_t pos = out.find(cur);
		while(pos!=string::npos){
			out.insert(pos, "<strong>");
			out.insert(pos + sizeof "<strong>" - 1 + cur.size(), "</strong>");
			pos =out.find(cur,pos+cur.size() + sizeof "<strong>" -1 +sizeof "</strong>" -1);
		}
	}
	
	cout << out << "\n";
	return 0;
}

//brief:�����������ҵ���ѡ�������ʼλ��
//becare:cur:��ǰ���ݵ�end()λ�ã���--curΪ��һ��Ҫ�����ֵ���������ص���beginλ��
int _find_start_position_of_abstract(const char *strRefine, const char *cur) {
	//const char *cur = pos;
	if (cur > strRefine) {
		--cur;
		//���Ե���
		while(cur!=strRefine && *cur!=kSEPARATE_ABSTRACT_PARAGRAPHY){
			--cur;
		}
		//uint8_t value = (uint8_t)*cur;
		////Ӣ��
		//while (value < kASCII_MAX
		//	&& ((value >= 'A'&&value <= 'Z')
		//		|| (value >= 'a' && value <= 'z')
		//		|| (value >= '0' && value <= '9')
		//		|| (value == ' '))) {
		//	--cur;
		//	value = (uint8_t)*cur;
		//	continue;
		//}
		////Ӣ�ı��Ҳ���������
		//if (value > kASCII_MAX) {
		//	//����
		//	--cur;
		//	//value = (uint8_t)*cur;
		//	while (cur >= strRefine && (uint8_t)*cur >= kGBK_FIRST_CHARACTER_BYTE) {
		//		cur -= 2;
		//	}
		//	++cur;
		//}
		//++cur;
	}
	return cur - strRefine;
	
}

//brief:�����������ҵ���ѡ�������ʼλ��
//brcare:cur:����ѯ�ĵ�һ��Ԫ�أ�beginλ��;;;���ص���endλ��
int _find_end_position_of_abstract(const char *strRefine, const char *strRefineLast, const char *cur) {
	//const char *cur = pos;
	if (cur < strRefineLast) {
		while(cur!=strRefineLast && *cur!=kSEPARATE_ABSTRACT_PARAGRAPHY){
			++cur;
		}
		//uint8_t value = (uint8_t)*cur;
		////Ӣ��
		//while (value < kASCII
		//	&& ((value >= 'A'&&value <= 'Z')
		//		|| (value >= 'a' && value <= 'z')
		//		|| (value >= '0' && value <= '9')
		//		|| (value == ' '))) {
		//	++cur;
		//	value = (uint8_t)*cur;
		//	continue;
		//}
		//if (value > kASCII_MAX) {
		//	//����
		//	while (cur < strRefineLast && (uint8_t)*cur >= kGBK_FIRST_CHARACTER_BYTE) {
		//		cur += 2;
		//	}
		//	//��ֹ���뵼��Խ��
		//	if (cur > strRefineLast) {
		//		cur = strRefineLast;
		//	}
		//}
	}
	return cur - strRefine;
	
}

//brief:����abstract
bool _is_trival(char c){
	if(c=='\r' || c=='\n' || c=='/'){
		return true;
	}
	return false;
}
