/*************************************************************************
	> File Name: document.h
	> Author: Dream9
	> Brief:
	> Created Time: 2019年05月31日 星期五 15时23分08秒
 ************************************************************************/
#include"document.h"


//static int _find_meta_charset(const char *source, string &charset);
static void _find_next_tag(const char**, const char*); 

const vector<string> kTAG = {"<script","<style","<!--"};
const vector<string> kTAG_END = {"/script>","/style>","-->"};

//brief:提炼网页信息
//todo:标签信息记录下来，用于评分
//int Document::Refine(const char *doc, string &charset) {
int Document::Refine(const char *doc) {
	m_sContent_Refined.clear();
	int len=kTAG.size();
	
	//信息简化
	const char *iter = doc;
	while (iter && *iter) {
		//跳过标签,  todo:一些特殊标签之间的内容也应该完全过滤，比如<script>222</script>
		//
		if (*iter == '<') {
			//6.21存在问题，注释标签为<!--   ...   -->,注意中间会出现<>, 已经修改
			//const char *next_iter = strchr(iter,'>');
			//处理特殊标签
			for(int i=0; i<len; ++i){
				if(0 == memcmp(iter, &kTAG[i][0], kTAG[i].size())){
					_find_next_tag(&iter, kTAG_END[i].c_str());
					break;
				}
			}

			if (!iter) {
				return 0;
			}
			if(*iter == '>'){
				iter += 1;
			}
			else{
				iter = strchr(iter, '>') + 1;
			}
		}
		else {
			//ret += strDoc[iter];
			const char *pos_end = strchr(iter, '<');
			m_sContent_Refined += (!pos_end) ? iter : string(iter, pos_end);
			iter = (!pos_end) ? nullptr : pos_end;
			continue;
		}
	}
//弃用
//	//编码转化
//	if (charset.empty()) {
//		//先从网页的meta中寻找
//		_find_meta_charset(doc, charset);
//		//
//		if (charset.empty()) {
//			if (0 == CharsetTransfer::Is_valid_utf8(&m_sContent_Refined[0])) {
//				charset = "utf-8";
//			}
//			else if (0 == CharsetTransfer::Is_vaild_gbk(&m_sContent_Refined[0])) {
//				charset = "gbk";
//			}
//			else if(0 == CharsetTransfer::Is_vaild_big5(&m_sContent_Refined[0])){
//				charset="big5";
//				//todo:需要转化到简体中文
//			}
//			else {
//				err_msg("%s:unsolved charset", __FUNCTION__);
//				return -1;
//			}
//		}
//	}
//	if (string::npos != StrFun::FindCase(charset, "gb")) {
//		//不须转换
//		return 0;
//	}
//	size_t max_len = m_sContent_Refined.size() << 1;
//	size_t len_source_info = m_sContent_Refined.size();
//	char *buf = (char *)malloc(max_len);
//	if (0 != CharsetTransfer::Convert(charset.c_str(), "gbk", &m_sContent_Refined[0], len_source_info, buf, max_len)) {
//		err_msg("%s:Convert() error",__FUNCTION__);
//		free(buf);
//		buf = nullptr;
//		return -1;
//	}
//	if (len_source_info > 0) {
//		err_msg("Infomation may lost");
//	}
//	m_sContent_Refined = buf;
//	free(buf);
//	buf = nullptr;

	return 0;
}

//brief:提炼网页信息，用于动态摘要生成
int Document::RefineWithSeparate(const char * doc) {
    if(!doc){
		return -1;
	}
	m_sContent_Refined.clear();
	
	//信息简化
	const char *iter = doc;
	while (iter && *iter) {
		//跳过标签,  todo:一些特殊标签之间的内容也应该完全过滤，比如<script>222</script>
		//
		if (*iter == '<') {
			//6.21存在问题，注释标签为<!--   ...   -->,注意中间会出现<>, 已经修改
			//const char *next_iter = strchr(iter,'>');
			//处理特殊标签
			int len=kTAG.size();
			for(int i=0; i<len; ++i){
				if(0 == memcmp(iter, &kTAG[i][0], kTAG[i].size())){
					_find_next_tag(&iter, kTAG_END[i].c_str());
					break;
				}
			}
			
			if (!iter) {
				return 0;
			}

			if(*iter == '>'){
				iter += 1;
			}
			else{
				iter = strchr(iter, '>') + 1;
			}

		}
		else {
			//ret += strDoc[iter];
			const char *pos_end = strchr(iter, '<');
			m_sContent_Refined += "/" + ((!pos_end) ? iter : string(iter, pos_end));
			iter = (!pos_end) ? nullptr : pos_end;
			continue;
		}
	}
	return 0;
}

//brief:辅助函数，从网页中提取charset信息
//int _find_meta_charset(const char *source, string &charset) {
//	if(!source){
//		log_msg("warning:nullptr");
//		return 0;
//	}
//	const char *ptr_pos_end = strstr(source,"</head>");
//	if(!ptr_pos_end){
//		err_msg("%s:invalid format without </headt> tag",__FUNCTION__);
//		return -1;
//	}
//	size_t pos_end = ptr_pos_end - source;
//	size_t pos_charset = 0;
//	//更严谨一点的话应该判断保证位于<meta>标签内
//	pos_charset = StrFun::FindCase(source + pos_charset, pos_end - pos_charset, "charset=", sizeof "charset=" - 1);
//	if (pos_charset == string::npos) {
//		return 0;
//	}
//	pos_charset += sizeof"charset=" - 1;
//	//提过空格
//	while (*(source + pos_charset) == ' ') {
//		++pos_charset;
//	}
//	ptr_pos_end = source + pos_charset;
//	while(*ptr_pos_end){
//		char cur=*ptr_pos_end;
//		if(cur=='\"' || cur=='\r' || cur=='\n' || cur=='>' || cur==';'){
//			break;
//		}
//		++ptr_pos_end;
//	}
//	//pos_end = source.find_first_of("\"> ;\r\n");
//	if (!*ptr_pos_end) {
//		charset.clear();
//		return 0;
//	}
//	charset = string(source + pos_charset, ptr_pos_end);
//	return 0;
//}
//

//brief:辅助函数
//parameter:start_tag_tail:开始标签的头指针,tag:需要定位的标签
//becare:结果修改在start_tag_tail,使其指向'>'
void _find_next_tag(const char **start_tag_tail, const char *tag){
	*start_tag_tail = strstr(*start_tag_tail, tag);
	if(*start_tag_tail){
		*start_tag_tail += strlen(tag) - 1;
	}
}
