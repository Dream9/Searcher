/*************************************************************************
	> File Name: response.cpp
	> Author: Dream9
	> Brief:
	> Created Time: 2019年06月15日 星期六 15时39分05秒
 ************************************************************************/

#include"response.h"

extern const size_t kNUMBER_OF_ANSWER_EACH_PAGE;//每页显示结果数
extern const char *kDEPLOYMENT_ADDRESS;//部署的位置
extern const char *kIMG_ADDRESS;//图片位置
extern const char *kCGI_QUERY_PATH;//代理查询cgi路径
extern const char *kCGI_LEFT_PAGE_PATH;//响应剩余查询结果cgi


//中文汉字第一个位置的首字节
static const uint8_t kGBK_FIRST_CHARACTER_BYTE = 0xB0;
//ASCII的界限
static const uint8_t kASCII_MAX = 128;
//动态摘要长度
static const int kABSTRACT_LENGTH = 256;
static const char kSEPARATE_ABSTRACT_PARAGRAPHY='/';

static int _find_end_position_of_abstract(const char *strRefine, const char *strRefineLast, const char *cur);
static int _find_start_position_of_abstract(const char *strRefine, const char *cur);
static bool _is_trival(char c);

#define ENSURE_EXIST(x) if(!x){continue;}
//brief:展示搜索结果部分
//parameter:str_query_word:查询内容
//          query_and:代理查询的结果
//          page_number:显示第几个网页（确定范围）
//          file_path：raw文档数据
//          doc_idx:正向索引数据
//          words:查询词的分隔数组
//          idf:每个查询词的idf值
int Response::ResponseResult(const char *str_query_word, score_container &query_ans,
	size_t page_number, const char *file_path, forward_index_type &doc_idx,
	vector<string> &words, vector<float> &idf) {
	//打开原始数据文件
	//todo:mmap实现
	std::ifstream ifsData(file_path, ios::in | ios::binary);
	if (!ifsData.is_open()) {
		return -1;
	}
	//定位显示范围
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
		//读取数据
		char *ptrDoc = (char*)calloc(data_len + 1, sizeof(char));
		ifsData.seekg(doc_idx[doc_id]);
		ifsData.read(ptrDoc, data_len);
		//提取title和url
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
		//对标题进行筛选
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

		//形成摘要
		//*pos_title_end = '<';
#ifdef DEBUG_1
		cout<<"\nceshi:\n"<<pos_url_start;
#endif
		_abstract_and_highlight(pos_title_end, words, idf);

		free(ptrDoc);
		//网页快照
		//todo snapshot
		cout << "</div>\n"
			"<a href=/cgi-bin/todo-snapshot?word=" << str_query_word << "&docid=" << doc_id
			<<" target=_blank>网页快照</a><br><br>\n";
	}

	cout << "<br><br>";
	return 0;
}

//brief:生成网页顶端内容，并设置CSS样式
int Response::ResponseTop(const char *query_string) {
	//const char *local_address = getenv("HTTP_HOST");
	//头部
	cout<<"Content-type:text/html\r\n\r\n"
		"<html><head>\n"
		"<meta charset=\"gbk\">\n"
		"<title>Dream9 Search</title>\n";

	//CSS样式
	cout << "<style>\n";
	cout << "body{color:#333;background:#fff;padding:6px 0 0;margin:0;position:relative;min-width:900px}\n"
		"table{border:0;width:100%;cellspacing:0;height:29;align=center;}\n"
		"img{border:0;width:308;height:65;display:block;margin:auto}\n"
		"p{text-align:left;}\n"
		".idx{font-size:16px;}\n"
		".f{align:center;width=100%;height=25;}\n"
		"</style>\n"
		"</head>\n";

	//顶部链接设置
	cout << "<body>\n<table>\n<tr>\n"
		"<td style=width:36%;rowspan:2;height:1><a href=http://" << kDEPLOYMENT_ADDRESS
		<< "><img src=" << kIMG_ADDRESS << "></a></td></tr>\n"
		"<tr><td style=\"width:64%;height:33;font-size:12;text-align:center;vertical-align:middle;\"><a href=http://" << kDEPLOYMENT_ADDRESS
		<< ">搜索主页</a> | <a href=\"\">使用帮助:</a><br></td>\n"
		"</tr>" << std::endl;
	//搜索框
	string tmp_str(query_string);
	replace(tmp_str.begin(),tmp_str.end(),kSEPARATE_CHAR,' ');
	cout << "<tr style=\"text_align:center;vertical-align:middle;\">\n<td><span><form method=get action=" << kCGI_QUERY_PATH
		<< " name=got class=f>"
		"<input type=text name=word size=55 value=" << tmp_str << ">\n"
		"<input type=submit value=搜索>\n"
		"<input type=hidden name=start value=1>\n"//隐藏标签
		"</form></span></td></tr>"
		"</table><br><hr><br>" << std::endl;
	return 0;
}

//brief:下方的跳转链接
//todo:需要完善
int Response::ResponseSelectPage(int number_result, int time_used, int page_no, const char *word_md5) {
	cout << "<span>Dream9 为您找到约<b>" << number_result << "</b>篇文档,耗时<b>"
		<< time_used << "</b>毫秒</span>\n"
		"<table class=idx}><tr>";
	if (page_no > 1) {
		cout << "<td><a href=" << kCGI_LEFT_PAGE_PATH << "?sessionid=" << word_md5 << "&page=" << page_no - 1 << ">上一页</a></td>";
	}
	else {
		cout << "<td>上一页</td>";
	}
	cout << "<td>&nbsp;&nbsp;&nbsp;&nbsp;</td>"
		"<td><strong>" << page_no << "</strong></td>" 
		"<td>&nbsp;&nbsp;&nbsp;&nbsp;</td>"
		"<td><a href=" << kCGI_LEFT_PAGE_PATH << "?sessionid=" << word_md5 << "&page=" << page_no + 1 << ">下一页</a></td>"
		"</table>\n" ;

	//网页尾端内容
	cout << "<br><hr><br><p style=text-align:center;>&copy Copyleft Dream9</p><br><br>\n";
	cout << "</body>\n</html>";
	return 0;
}

//brief:网页摘要生成，并高亮显示
//     改良的基于滑动窗口的最优摘要选取算法：维护map<段落起始实pos位置,pair<段落长度，段落权值>>
//     其中段落最长有限制(假设：在正常情况下一般是先找到了段落起始而不是长度超限)
//     最后根据权值选择最优摘要
int Response::_abstract_and_highlight(const char *str, vector<string> &words, vector<float> &idf) {
	cout << "<span style=\"color:blue; font - weight:bold\"> 摘要 </span>\n";
	//
	Document doc_parser;
	doc_parser.RefineWithSeparate(str);
	//
	const char *strRefine = doc_parser.m_sContent_Refined.c_str();
#ifdef DEBUG_1
	cout<<'\n'<<strRefine<<'\n';
#endif
	const char *strRefineLast = strRefine + doc_parser.m_sContent_Refined.size();

	//2019.6.21修改策略，采用类似quickUnion的实现方式
	typedef std::pair<int, float> weight_type;//段长，权值
	unordered_map<int, weight_type>mapScore;//段落起始位置，段落信息
	vector<std::pair<int,int>>vecQuickUnion;//词所处的小段落位置，该词所处的段落起始位置

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
		//找到一个词
		while ((pos = strstr(pos, cur_word.c_str()))) {
			//定位到段落开始位置
			//即定位到向前第一个 (非汉字字符 && 非英文字母 && 非数字) 的位置
			int idx_start = _find_start_position_of_abstract(strRefine, pos);
			//定位到段落结束位置
			pos += cur_word.size();
			int idx_last = _find_end_position_of_abstract(strRefine, strRefineLast, pos);

			int length=idx_last-idx_start;
			if(length>=kABSTRACT_LENGTH || vecQuickUnion.empty()){
				//足够长
				vecQuickUnion.emplace_back(std::make_pair(idx_start,idx_start));
				mapScore.insert({idx_start,{length,idf[iter]}});
				if(idf[iter]>max_weight){
					max_weight=idf[iter];
					max_weight_position=idx_start;
				}
			}
			else{
				//需要和前面的合并
				int _start=vecQuickUnion.back().second;
				auto _iter=mapScore.find(_start);
				if((*_iter).second.first<kABSTRACT_LENGTH){
					//合并
					vecQuickUnion.emplace_back(std::make_pair(idx_start,_start));
					(*_iter).second.first=idx_last - _start;
					(*_iter).second.second+=idf[iter];
					if((*_iter).second.second>max_weight){
						max_weight=idf[iter];
						max_weight_position=_start;
					}
				}
				else{
					//前方过长,不能合并
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
	//及其特殊的情况，也就是标题党。。。仅在标题中出现了这个关键字，之后再也没有出现。。。
	if (max_weight_position == -1) {
		cout << "<span>This page is <strong>just a title</strong> ! Kill him!</span>";
		return -1;
	}
	//动态摘要生成
	//根据长度限制做必要的扩展收缩，并高亮显示
	len = mapScore[max_weight_position].first;
	if (len < kABSTRACT_LENGTH) {
		int pos = max_weight_position;//为扩展前段落begin位置
		int pos_end = max_weight_position + len;
		//向前扩展
		--pos;
		while (pos >= 0 && strRefine[pos] != '/') {
			--pos;
		}

		max_weight_position = pos == 0 ? 0 : pos + 1;//更新begin位置,防止意外
		//向后扩展
		int lastone = doc_parser.m_sContent_Refined.size();
		while (pos_end < lastone && strRefine[pos_end] != '/') {
			++pos;
		}
		len = pos_end - max_weight_position;		
	}
	//生成摘要
	string out(strRefine + max_weight_position, len);
	//去除多余
    out.erase(remove_if(out.begin(),out.end(),_is_trival),out.end());
	//高亮显示
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

//brief:辅助函数，找到候选段落的起始位置
//becare:cur:向前回溯的end()位置，，--cur为第一个要检验的值；；；返回的是begin位置
int _find_start_position_of_abstract(const char *strRefine, const char *cur) {
	//const char *cur = pos;
	if (cur > strRefine) {
		--cur;
		//策略调整
		while(cur!=strRefine && *cur!=kSEPARATE_ABSTRACT_PARAGRAPHY){
			--cur;
		}
		//uint8_t value = (uint8_t)*cur;
		////英文
		//while (value < kASCII_MAX
		//	&& ((value >= 'A'&&value <= 'Z')
		//		|| (value >= 'a' && value <= 'z')
		//		|| (value >= '0' && value <= '9')
		//		|| (value == ' '))) {
		//	--cur;
		//	value = (uint8_t)*cur;
		//	continue;
		//}
		////英文标点也算结束符号
		//if (value > kASCII_MAX) {
		//	//中文
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

//brief:辅助函数，找到候选段落的起始位置
//brcare:cur:向后查询的第一个元素，begin位置;;;返回的是end位置
int _find_end_position_of_abstract(const char *strRefine, const char *strRefineLast, const char *cur) {
	//const char *cur = pos;
	if (cur < strRefineLast) {
		while(cur!=strRefineLast && *cur!=kSEPARATE_ABSTRACT_PARAGRAPHY){
			++cur;
		}
		//uint8_t value = (uint8_t)*cur;
		////英文
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
		//	//中文
		//	while (cur < strRefineLast && (uint8_t)*cur >= kGBK_FIRST_CHARACTER_BYTE) {
		//		cur += 2;
		//	}
		//	//防止乱码导致越界
		//	if (cur > strRefineLast) {
		//		cur = strRefineLast;
		//	}
		//}
	}
	return cur - strRefine;
	
}

//brief:精简abstract
bool _is_trival(char c){
	if(c=='\r' || c=='\n' || c=='/'){
		return true;
	}
	return false;
}
