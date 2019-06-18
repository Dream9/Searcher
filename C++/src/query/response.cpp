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

#define ENSURE_EXIST(x) if(!x){continue;}
//brief:展示搜索结果部分
//parameter:str_query_word:查询内容
//          query_and:代理查询的结果
//          page_number:显示第几个网页（确定范围）
//          file_path：raw文档数据
//          doc_idx:正向索引数据
int Response::ResponseResult(const char *str_query_word, score_container &query_ans,
	size_t page_number, const char *file_path, forward_index_type &doc_idx) {
	//打开原始数据文件
	//todo:mmap实现
	std::ifstream ifsData(file_path, ios::in | ios::binary);
	if (!ifsData.is_open()) {
		return -1;
	}
	//定位显示范围
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
		*pos_title_end = '\0';

		cout << "<a href=" << pos_url_start << "><strong>" << pos_title_start << "</strong></a>\n"
			<< "<br><div>\n";

		//形成摘要
		*pos_title_end = '<';
		_abstract_and_highlight(pos_url_end + 1);

		free(ptrDoc);
		//网页快照
		//todo snapshot
		cout << "</div>"
			<< "<a href=/cgi路径/index/snapshot?word=" << str_query_word << "&docid=" << doc_id
			<< " target=_blank>网页快照</a><br>\n";
	}

	cout << "</ol><br><br>";
	return 0;
}

//brief:生成网页顶端内容，并设置CSS样式
int Response::ResponseTop(const char *query_string) {
	const char *local_address = getenv("HTTP_HOST");
	cout << "<title>Dream9 Search</title>\n";

	//CSS样式
	cout << "<style>\n";
	cout << "body{color:#333;background:#fff;padding:6px 0 0;margin:0;position:relative;min-width:900px}\n"
		"table{border:0;width:100%;cellspacing:0;height:29;}\n"
		"img{border:0;width:308;height:65;}\n"
		"p{text-align:left;}\n"
		".idx{border:2;}\n"
		"</style>\n";

	//顶部链接设置
	cout << "<body>\n<table>\n<tr>\n"
		"<td style=width:36%;rowspan:2;height:1><a href=http://" << local_address << kDEPLOYMENT_ADDRESS
		<< "><img src=" << kIMG_ADDRESS << "></a></td>\n"
		"<td style=width:64%;height:33;font-size:2><a href=http://" << local_address << kDEPLOYMENT_ADDRESS
		<< ">搜索主页</a> | <a href=\"\">使用帮助:todo</a><br></td>\n"
		"</tr>" << std::endl;
	//搜索框
	cout << "<tr>\n<td><span><form method=get action=" << kCGI_QUERY_PATH
		<< " name=got>"
		"<input type=text name=word size=55 value=" << query_string << ">\n"
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
	cout << "<br><hr><br>&copy Copyleft Dream9<br><br>\n";
	cout << "</body>\n</html>";
	return 0;
}

//brief:网页摘要生成，并高亮显示
//todo:尚未完成
int Response::_abstract_and_highlight(const char *str) {
	cout << "todo:<span style=\"color:blue; font - weight:bold\"> 摘要 </span>\n";
	return 0;
}
