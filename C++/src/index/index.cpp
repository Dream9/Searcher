/*************************************************************************
	> File Name: index.cpp
	> Author: Dream9
	> Brief:
	> Created Time: 2019年06月06日 星期五 15时34分02秒
 ************************************************************************/

#include"index.h"

extern const char kSEPARATE_CHAR;
extern const char kSEPARATE_CHAR_INVERSE_FILE;

static const string kHTML_SPACE = "&nbsp;";
static int _deal_with_system_status(int);
//static int _find_charset_from_header(char *, int, string &);

#define _ENSURE_SUCCESS(x,y) if(!x){err_msg("%s error:open %s failed",y);}
//brief:FormatFile文件的分词
//parameter:formatfile路径,doc索引路径，结果文件路径,词典
int Index::DocumentSegment(const char *file_path,const char *doc_index_file,
	const char *segment_file,Dictinary &dict) {
	if (!file_path || !segment_file|| !doc_index_file) {
		err_msg("%s error: null pointer", __FUNCTION__);
		return -1;
	}
	auto flag = ios::in | ios::out | ios::binary;
	fstream ifsFile(file_path, flag);
	_ENSURE_SUCCESS(ifsFile, file_path);
	fstream ofsSegment(segment_file, flag | ios::trunc);
	_ENSURE_SUCCESS(ofsSegment, segment_file);
	fstream ifsDocIndex(doc_index_file, flag);
	_ENSURE_SUCCESS(ifsDocIndex, doc_index_file);

	//获取doc位置信息
	string strLine;
	vector<pair<int, int>>vecDocIndex;
	while (getline(ifsDocIndex, strLine)) {
		if(strLine.empty() || strLine[0]=='\0' || strLine[0]=='\n' || strLine[0]=='\r'){
			continue;
		}
		int docid, position;
		if (2 != sscanf(strLine.c_str(), "%d %d %*s", &docid, &position)) {
			err_msg("%s error:bad file format", __FUNCTION__);
			return -2;
		}
		vecDocIndex.emplace_back(make_pair(docid, position));
	}
	ifsDocIndex.close();

	//定位文档并分词
	int len_vecDocIndex = vecDocIndex.size() - 1;
	Document doc_parser;
	for (int iter = 0; iter < len_vecDocIndex; ++iter) {
		int len_doc = vecDocIndex[iter + 1].second - vecDocIndex[iter].second - 1;
		//becare:资源释放和指针置空
		char *buf = (char *)calloc(sizeof(char), len_doc + 1);
		ifsFile.seekg(vecDocIndex[iter].second);
		ifsFile.read(buf, len_doc);
		/*文档示例
		version: 1.0
		url: http://www-sp.jl.cninfo.net/
		date: Sun, 18 Apr 2004 01:38:22 GMT
		ip: 202.98.7.130
		length: 64748

		HTTP/1.0 200 OK
		Server: Netscape-Enterprise/2.01
		Date: Sat, 17 Apr 2004 15:59:45 GMT
		Accept-ranges: bytes
		Last-modified: Fri, 16 Apr 2004 08:07:01 GMT
		Content-length: 64519
		Content-type: text/html
		Connection: keep-alive

		<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.01 Transitional//EN">
		...
		*/
		//跳过文档的两个附加头部信息
		//becare:\n\n分隔
		char *ptrBuf = strstr(buf, "\n\n");
		if (!ptrBuf) {
			err_msg("%s:bad file format:%s", __FUNCTION__, file_path);
			free(buf);
			buf = nullptr;
			return -3;
		}
		ptrBuf += 2;
		//从头部中提取charset信息
		//char *header = ptrBuf;
		if (!(ptrBuf = strstr(ptrBuf, "\n\n"))) {
			err_msg("%s:bad file format:%s", __FUNCTION__, file_path);
			free(buf);
			buf = nullptr;
			return -3;
		}
		//string charset;
		//if (0 != _find_charset_from_header(header, ptrBuf - header, charset)) {
		//	err_msg("%s:bad file format:%s", __FUNCTION__, file_path);
		//	free(buf);
		//	buf = nullptr;
		//	return -3;
		//}
		//文档简化以及格式调整
		ptrBuf += 2;
		//if (0 != doc_parser.Refine(ptrBuf,charset)) {
		if (0 != doc_parser.Refine(ptrBuf)) {
			err_msg("Refine() failed");
			free(buf);
			buf = nullptr;
			continue;
		}
		free(buf);
		buf = nullptr;
		//log_msg("charset is %s.",charset.c_str());

		StrFun::ReplaceStr(doc_parser.m_sContent_Refined, kHTML_SPACE, " ");
		WordSegment word_parser;
		//通过‘#’区分doc id
		ofsSegment << '#' << vecDocIndex[iter].first << "\n"
			<< word_parser.Segment_need_preprose(dict, doc_parser.m_sContent_Refined)<<"\n";
		ofsSegment.flush();
	}
	ifsDocIndex.close();
	ofsSegment.close();
	return 0;
}
#undef _ENSURE_SUCCESS


//brief:产生正向索引文件，包括url md5->doc id和doc id->data
int Index::CreateForwardIndex(const char *raw_data_file, const char *url_index_file, const char *forward_index_file) {
	if (0 != CreateDocIndex(raw_data_file,url_index_file,forward_index_file)) {
		err_msg("%s:CreateDocIndex()failed", __FUNCTION__);
		return -1;
	}
	return 0;
}

//brief:产生反向索引文件
//
//2016/6/14：格式调整为 word+分隔符+Count文档数+分隔符+docid+分隔符+该docid中该word重复次数+.....
int Index::CreateInverseIndex(const char *segment_file, const char *inverse_index_file) {
	//对切词文件进行数据预处理
	//期待格式为  word+分隔符+docid+\n........并且已经完成排序
	string tmpfile = segment_file;
	tmpfile += ".temp_sort";
	if (0 != _group_and_sort_segment_file(segment_file, tmpfile.c_str())) {
		err_msg("%s:_group_and_sort_segment_file() failed",__FUNCTION__);
		return -1;
	}
	
	//
	ifstream ifsTmpSegmentFile(tmpfile.c_str(), ios::binary | ios::in);
	if (!ifsTmpSegmentFile.is_open()) {
		err_msg("%s:%s cann't access", __FUNCTION__, tmpfile.c_str());
		return -2;
	}
	ofstream ofsInverseFile(inverse_index_file, ios::binary | ios::out | ios::trunc);
	if (!ofsInverseFile.is_open()) {
		err_msg("%s:%s cann't access", __FUNCTION__, inverse_index_file);
		return -3;
	}
	string strLine;
	string strDocIds;
	string strLastWord;
	string strLastDocId;
	int count_id = 0;
	int count_doc_total = 0;
	while (getline(ifsTmpSegmentFile, strLine)) {
		if(strLine.empty() || strLine[0]=='\n' || strLine[0]=='\0'){
			continue;
		}
		size_t pos_sep = strLine.find_first_of(kSEPARATE_CHAR);
		if (string::npos == pos_sep) {
			continue;
		}
		strLine[pos_sep] = '\0';
		string strCurrentWord(strLine.c_str());
		if (strCurrentWord == strLastWord) {
			//统计docid重复次数
			if (strLastDocId == &strLine[pos_sep + 1]) {
				++count_id;
				//++count_doc_total;相同的文档算一个
			}
			else{
				strDocIds += kSEPARATE_CHAR_INVERSE_FILE + strLastDocId + kSEPARATE_CHAR_INVERSE_FILE + to_string(count_id);
				strLastDocId = &strLine[pos_sep + 1];
				count_id = 1;
				count_doc_total++;
			}
			//else {
			//	strLastDocId = &strLine[pos_sep + 1];
			//	count_id = 1;
			//	count_doc_total++;
			//}
		}
		else {
			//更新LastWord等信息
			if(!strLastWord.empty()){
				strDocIds += kSEPARATE_CHAR_INVERSE_FILE + strLastDocId + kSEPARATE_CHAR_INVERSE_FILE + to_string(count_id);
				ofsInverseFile << strLastWord << kSEPARATE_CHAR_INVERSE_FILE  << to_string(count_doc_total) << strDocIds <<'\n';
			}
			string tmp;
			strDocIds.swap(tmp);
			strLastWord.swap(strCurrentWord);
			strLastDocId = &strLine[pos_sep + 1];
			//strDocIds = &strLine[pos_sep + 1];
			count_doc_total = 1;
			count_id = 1;
		}
	}
	//处理最后一个词
	strDocIds += kSEPARATE_CHAR_INVERSE_FILE + strLastDocId + kSEPARATE_CHAR_INVERSE_FILE + to_string(count_id);
	ofsInverseFile << strLastWord << kSEPARATE_CHAR_INVERSE_FILE  << to_string(count_doc_total)
				   << strDocIds << endl;
	return 0;
}

//brief:对切词文件进行处理，每一行为word+docid组成，并按照词排序
//     辅助函数
int Index::_group_and_sort_segment_file(const char *segment_file, const char *tmp_sort_file) {
	ifstream ifsSegment(segment_file, ios::in | ios::binary);
	if (!ifsSegment) {
		err_ret("%s:can't access %s", __FUNCTION__, segment_file);
		return -1;
	}
	FILE *ofsSort= fopen(tmp_sort_file, "wb");
	//ofstream ofsSort(tmp_sort_file, ios::out | ios::binary | ios::trunc);
	if (!ofsSort) {
        err_ret("%s:can't access %s", __FUNCTION__, tmp_sort_file);
		return -1;
	}

	//一行一组
	string strLine;
	string strDocId;
	while (getline(ifsSegment, strLine)) {
		if (strLine.empty() || strLine[0] == '\0' || strLine[0] == '\n') {
			continue;
		}
		if (strLine[0] == '#') {
			strDocId = strLine.substr(1, strLine.size() - 1);
			continue;
		}
		//处理正文
		size_t pos_sep_start = 0;
		size_t pos_sep_end = 0;
		while (string::npos != (pos_sep_end = strLine.find_first_of(kSEPARATE_CHAR,pos_sep_start))) {
			strLine[pos_sep_end] = '\0';//减少不必要的拷贝操作
			fwrite(&strLine[pos_sep_start], sizeof(char), pos_sep_end - pos_sep_start, ofsSort);
			fputc(kSEPARATE_CHAR, ofsSort);
			fwrite(strDocId.c_str(), sizeof(char), strDocId.size(), ofsSort);
			fputc('\n', ofsSort);
			pos_sep_start=pos_sep_end + 1;
		}
	}
	//确保刷新到磁盘
	fsync(fileno(ofsSort));
	fclose(ofsSort);
	//排序
	//becare:这里偷懒直接调用了sort命令
	const char *name = "LANG";
	const char *value;
	value = getenv(name);
	string cmd;
	int status = 0;
	cmd = "export LANG=en && sort ";
	cmd += tmp_sort_file;
	cmd += " -o ";
	cmd += tmp_sort_file;
	status = system(cmd.c_str());
	if ((_deal_with_system_status(status)) < 0) {
		return -1;
	}
	//还原环境变量
	//其实作用不大
	cmd = "export LANG=";
	cmd += value;
	status = system(cmd.c_str());
	if ((_deal_with_system_status(status)) < 0) {
		err_msg("export LANG failed");
		return -1;
	}

	return 0;
}

//brief:多个反向索引文件合并
int Index::MergeInverseFile(const char **file_list, int len_file_list) {
	;
	return 0;
}

//brief:
//return:-1：system调用失败或者shell进程异常终止
int _deal_with_system_status(int status) {
	if (-1 == status) {
		err_msg("System() error");
		return -1;
	}
	//查询进程返回状态
	if (WIFEXITED(status)) {
		//非零表示正常返回
		return WEXITSTATUS(status);
	}
	
	//进程非正常结束
	err_msg("cmd failed");
	return -1;
}

//brief:获取响应头中的charset信息
//     结果保存在ret之中
//int _find_charset_from_header(char *header, int len_header, string &ret) {
//	size_t pos_charset_start = StrFun::FindCase(header, len_header, "charset=", sizeof "charser=" -1 );
//	if (string::npos == pos_charset_start) {
//		ret.clear();
//		return 0;
//	}
//    pos_charset_start += sizeof "charset=" - 1;
//	while (header[pos_charset_start] == ' ') pos_charset_start++;
//	const char *ptr_charset_end = strchr(header + pos_charset_start, '\r');
//	if (!ptr_charset_end) {
//		err_msg("%s:unexpeted end", __FUNCTION__);
//		ret.clear();
//		return -1;
//	}
//	int tmp_pos = ptr_charset_end - header;
//	*(header + tmp_pos) = '\0';
//	ret = header + pos_charset_start;
//	*(header + tmp_pos) = '\r';
//	return 0;
//	
//}


