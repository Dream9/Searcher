/*************************************************************************
	> File Name: query.cpp
	> Author: Dream9
	> Brief:
	> Created Time: 2019年06月12日 星期三 14时57分39秒
 ************************************************************************/
#include"query.h"

#include<string.h>


//brief:获取参数信息
//return: 错误信息
//      其中kERROR_QUERY_EXCEED_SIZE这项错误需要特别对待
//becare：只是初步获得查询字符串，并没有处理查询串
int Query::GetInput() {
	const char *method = getenv("REQUEST_METHOD");
	if (!method) {
		return kERROR_QUERY_PARAMETER;
	}
	int length;
	int ret=0;
	char parameter[kMAX_QUERY_LENGTH + 1];
	memset(parameter, 0, sizeof parameter);
	if (memcmp(method, "POST", sizeof "POST" -1) == 0) {
		//从stdin中获取
		length = atoi(getenv("CONTENT_LENGTH"));
		if (0 == length) {
			return kERROR_QUERY_PARAMETER;
		}
		else if (length > kMAX_QUERY_LENGTH) {
			//过长截断
			ret = kERROR_QUERY_EXCEED_SIZE;
			length = kMAX_QUERY_LENGTH;
		}
		if (length != RioRead(STDIN_FILENO, parameter, length)) {
			return kERROR_QUERY;
		}
	}
	else if (memcmp(method, "GET", sizeof "GET" - 1) == 0) {
		//GET需要从环境变量中提取
		const char *ptr = getenv("QUERY_STRING");
		length = strlen(ptr);
		if (0 == length) {
			return kERROR_QUERY_PARAMETER;
		}
		else if (length > kMAX_QUERY_LENGTH) {
			ret = kERROR_QUERY_EXCEED_SIZE;
			length = kMAX_QUERY_LENGTH;
		}
		memcpy(parameter, ptr, length);
	}
	else{
		return kERROR_QUERY_PARAMETER;
	}
	//解析参数串，格式形如 word=六六六&sdf=sss
	//但考虑到扩展性可以假设有不确定个参数需要提取
	/*假设有多个参数：
	vector<string> vec_query;
	ParameterSplit(parameter, kHTML_PARAMETER_DELIM_CHAR, vec_query);
	for (string &str : vec_query) {
		vector<string> tmp;
		ParameterSplit(str.c_str(), kHTML_PARAMETER_EQUAL_CHAR, tmp);
		if (tmp.size() != 2) {
			return kERROR_QUERY_PARAMETER;
		}
		m_query_string.emplace_back((tmp[0], tmp[1]));
	}
	*/

	//
	char *pos_equal_char = strchr(parameter, kHTML_PARAMETER_EQUAL_CHAR);
	if (!pos_equal_char) {
		return kERROR_QUERY_PARAMETER;
	}
	StrFun::TranslateUrl(pos_equal_char,&length);//length返回未使用的长度

	//要求第一个参数为word=XXX&...
	//...会被省略，xxx为查询词
	char *pos_end=strchr(pos_equal_char, kHTML_PARAMETER_DELIM_CHAR);
	if(pos_end){
		m_query_string.assign(pos_equal_char+1,pos_end);
	}
	else{
		m_query_string.assign(pos_equal_char+1);
	}
	if (m_query_string.empty()) {
		return kERROR_QUERY_PARAMETER;
	}
	return ret;
	
}

//brief:外部直接传入word部分(是指从?往后的部分,例如： word=99&... )
int Query::SetQueryParameter(const string& word){
	char parameter[kMAX_QUERY_LENGTH + 1];
	size_t len = static_cast<size_t>(kMAX_QUERY_LENGTH) < word.size() ? static_cast<size_t>(kMAX_QUERY_LENGTH) : word.size();
	size_t equal_pos = word.find(kHTML_PARAMETER_DELIM_CHAR);
	if(string::npos == equal_pos)
		return kERROR_QUERY_PARAMETER;

	memcpy(parameter, word.c_str() + equal_pos + 1, len);

	StrFun::TranslateUrl(parameter,nullptr);//length返回未使用的长度

	//要求第一个参数为word=XXX&...
	//...会被省略，xxx为查询词
	char *pos_end=strchr(parameter, kHTML_PARAMETER_DELIM_CHAR);
	if(pos_end){
		m_query_string.assign(parameter,pos_end);
	}
	else{
		m_query_string.assign(parameter);
	}
	if (m_query_string.empty()) {
		return kERROR_QUERY_PARAMETER;
	}

	return 0;
}

//brief：按照指定分隔符delimiter切割字符串target，结果保存在vec中
//return:出错信息
int Query::ParameterSplit(const char *target, const char delimeter, vector<string> &vec) {
	vec.clear();
	const char *pos_start = target;
	const char *pos_end = nullptr;
	while ((*pos_start) && (pos_end = strchr(pos_start, delimeter))) {
		vec.emplace_back(pos_start, pos_end);
		pos_start = pos_end + 1;
	}
	if (!*pos_start) {
		return kERROR_QUERY_PARAMETER;
	}
	vec.emplace_back(pos_start);

	return 0;
}

//brief:加载词典,简单的封装一下
int Query::GetDictinary(Dictinary &dict, const char *dict_path) {
	if (0 != dict.OpenDictinary(dict_path)) {
		return kERROR_QUERY_LOAD_DATA;
	}
	return 0;
}

//brief:处理查询字符串
//todo:联想与扩展
int Query::HandleQueryString(Dictinary &dict) {
	WordSegment word_parser;
	m_query_string = std::move(word_parser.Segment_need_preprose(dict, m_query_string));
	return 0;
}

#define ENSURE(x,y) if(y<0){return y;} else if(y>0){x=y;}
//brief:初始化查询环境
//becare:forward_index为doc id -> doc offset结构，
//       inverse_index为word -> doc list str结构
int Query::EnvironmentInit(unordered_map<int,int> &forward_index, const char *forward_index_file,
	unordered_map<string,string> &inverse_index, const char *inverse_index_file) {
	int ret = 0;
	int tmp;
	tmp = _load_forward_data(forward_index, forward_index_file);
	ENSURE(ret,tmp);
	tmp = _load_inverse_data(inverse_index, inverse_index_file);
	ENSURE(ret, tmp);
	return ret; 
}
#undef ENSURE

//brief:外部接口
//int Query::QueryStringInit(Dictinary &dict) {
//	int ret = 0;
//	int tmp = GetInput();
//	ENSURE(ret,tmp);
//	tmp = HandleQueryString(dict);
//	ENSURE(ret, tmp);
//	return ret;
//}


//brief:加载正向索引数据
//todo:mmap
int Query::_load_forward_data(unordered_map<int,int> &forward_index, const char *file_path) {
	if (!file_path) {
		return kERROR_QUERY_LOAD_DATA;
	}
	auto flag = ios::in | ios::binary;
	std::fstream ifsDocIndex(file_path, flag);
	if (!ifsDocIndex) {
		return kERROR_QUERY_LOAD_DATA;
	}

	//获取doc位置信息
	string strLine;
	while (getline(ifsDocIndex, strLine)) {
		if(strLine.empty() || strLine[0]=='\0' || strLine[0]=='\n' || strLine[0]=='\r' || strLine[0]=='#'){
			continue;
		}
		int docid, position;
		if (2 != sscanf(strLine.c_str(), "%d %d %*s", &docid, &position)) {
			return kERROR_QUERY_LOAD_DATA;
		}
		forward_index.insert(std::make_pair(docid, position));
	}
	ifsDocIndex.close();
	return 0;
}

//brief:加载反向索引数据
int Query::_load_inverse_data(unordered_map<string, string> &inverse_index, const char *file_path) {
	if (!file_path) {
		return kERROR_QUERY_LOAD_DATA;
	}
	auto flag = ios::in | ios::binary;
	std::fstream ifsIndex(file_path, flag);
	if (!ifsIndex) {
		return kERROR_QUERY_LOAD_DATA;
	}

	//获取doc位置信息
	string strLine;
	while (getline(ifsIndex, strLine)) {
		if(strLine.empty() || strLine[0]=='\0' || strLine[0]=='\n' || strLine[0]=='\r' || strLine[0]=='#'){
			continue;
		}
		size_t pos = strLine.find_first_of(kSEPARATE_CHAR_INVERSE_FILE);
		if (string::npos == pos) {
			return kERROR_QUERY_LOAD_DATA;
		}
		strLine[pos] = '\0';
		inverse_index.insert(std::make_pair(&strLine[0], &strLine[pos + 1]));
	}
	ifsIndex.close();
	return 0;
}



//brief:利用TF-IDF计算网页得分
int Query::GetAnswer(int total_doc_number, unordered_map<string,string> &mapInverse, score_container &score_ret,
                     vector<string> &vec_words, vector<float> &vec_idf) {
	string word;
	size_t pos_start = 0;
	size_t pos_sep;
	string sscanf_format("%d");
	sscanf_format += kSEPARATE_CHAR_INVERSE_FILE;
	string sscanf_format4doc = sscanf_format + sscanf_format;
	unordered_map<int, float>tmp_score;

	while (string::npos != (pos_sep = m_query_string.find_first_of(kSEPARATE_CHAR, pos_start))) {
		string word(&m_query_string[pos_start], &m_query_string[pos_sep]);
		pos_start = pos_sep + 1;
		if (mapInverse.find(word) == mapInverse.end()) {
			//查不到这个词
			continue;
		}
		//计算idf
		int  word_doc_number;
		const char *ptr_start = mapInverse[word].c_str();
		if (1 != sscanf(ptr_start, sscanf_format.c_str(), &word_doc_number)) {
			//数据文件有问题
			continue;
		}
		const char *ptr_end = strchr(ptr_start, kSEPARATE_CHAR_INVERSE_FILE);
		//int word_doc_number = std::stoi(string(ptr_start, ptr_end));
		float idf = log((float)total_doc_number / word_doc_number);
		//记录这些idf
		vec_words.emplace_back(word);
		vec_idf.emplace_back(idf);
		//更新每一个网页的权重
		ptr_start = ptr_end + 1;
		while (word_doc_number > 0) {
			int docid;
			int doc_count;
			if (2 != sscanf(ptr_start, sscanf_format4doc.c_str(), &docid, &doc_count)) {
				return kERROR_QUERY_DATA;
			}
			//更新分数
			tmp_score[docid] = idf * doc_count;
			//score_ret.update({ docid,idf*doc_count });
			//定位到下一个位置
			word_doc_number--;
			//处理异常格式
			try {
				ptr_start = strchr(ptr_start, kSEPARATE_CHAR_INVERSE_FILE);
				ptr_start = strchr(ptr_start + 1, kSEPARATE_CHAR_INVERSE_FILE);
				ptr_start++;
			}
			catch (...) {
				return kERROR_QUERY_DATA;
			}
		}
	}
	//结果排序,,,注意降序排列
	std::multimap<float, int, std::greater<float>> tmp4sort;
	for (auto cur : tmp_score) {
		tmp4sort.insert({ cur.second,cur.first });
	}
	score_ret.reserve(tmp_score.size());
	for (auto cur : tmp4sort) {
		score_ret.emplace_back(cur.second);
	}
	return 0;
}

//brief:日志记录
int Query::_log_query() {
	time_t tv= time(nullptr);
	if (0 != _logger(m_query_string.c_str(), ctime(&tv))) {
		return kERROR_LOG;
	}
	return 0;
}


