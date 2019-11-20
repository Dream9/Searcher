/*************************************************************************
	> File Name: query.cpp
	> Author: Dream9
	> Brief:
	> Created Time: 2019��06��12�� ������ 14ʱ57��39��
 ************************************************************************/
#include"query.h"

#include<string.h>


//brief:��ȡ������Ϣ
//return: ������Ϣ
//      ����kERROR_QUERY_EXCEED_SIZE���������Ҫ�ر�Դ�
//becare��ֻ�ǳ�����ò�ѯ�ַ�������û�д����ѯ��
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
		//��stdin�л�ȡ
		length = atoi(getenv("CONTENT_LENGTH"));
		if (0 == length) {
			return kERROR_QUERY_PARAMETER;
		}
		else if (length > kMAX_QUERY_LENGTH) {
			//�����ض�
			ret = kERROR_QUERY_EXCEED_SIZE;
			length = kMAX_QUERY_LENGTH;
		}
		if (length != RioRead(STDIN_FILENO, parameter, length)) {
			return kERROR_QUERY;
		}
	}
	else if (memcmp(method, "GET", sizeof "GET" - 1) == 0) {
		//GET��Ҫ�ӻ�����������ȡ
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
	//��������������ʽ���� word=������&sdf=sss
	//�����ǵ���չ�Կ��Լ����в�ȷ����������Ҫ��ȡ
	/*�����ж��������
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
	StrFun::TranslateUrl(pos_equal_char,&length);//length����δʹ�õĳ���

	//Ҫ���һ������Ϊword=XXX&...
	//...�ᱻʡ�ԣ�xxxΪ��ѯ��
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

//brief:�ⲿֱ�Ӵ���word����(��ָ��?����Ĳ���,���磺 word=99&... )
int Query::SetQueryParameter(const string& word){
	char parameter[kMAX_QUERY_LENGTH + 1];
	size_t len = static_cast<size_t>(kMAX_QUERY_LENGTH) < word.size() ? static_cast<size_t>(kMAX_QUERY_LENGTH) : word.size();
	size_t equal_pos = word.find(kHTML_PARAMETER_DELIM_CHAR);
	if(string::npos == equal_pos)
		return kERROR_QUERY_PARAMETER;

	memcpy(parameter, word.c_str() + equal_pos + 1, len);

	StrFun::TranslateUrl(parameter,nullptr);//length����δʹ�õĳ���

	//Ҫ���һ������Ϊword=XXX&...
	//...�ᱻʡ�ԣ�xxxΪ��ѯ��
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

//brief������ָ���ָ���delimiter�и��ַ���target�����������vec��
//return:������Ϣ
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

//brief:���شʵ�,�򵥵ķ�װһ��
int Query::GetDictinary(Dictinary &dict, const char *dict_path) {
	if (0 != dict.OpenDictinary(dict_path)) {
		return kERROR_QUERY_LOAD_DATA;
	}
	return 0;
}

//brief:�����ѯ�ַ���
//todo:��������չ
int Query::HandleQueryString(Dictinary &dict) {
	WordSegment word_parser;
	m_query_string = std::move(word_parser.Segment_need_preprose(dict, m_query_string));
	return 0;
}

#define ENSURE(x,y) if(y<0){return y;} else if(y>0){x=y;}
//brief:��ʼ����ѯ����
//becare:forward_indexΪdoc id -> doc offset�ṹ��
//       inverse_indexΪword -> doc list str�ṹ
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

//brief:�ⲿ�ӿ�
//int Query::QueryStringInit(Dictinary &dict) {
//	int ret = 0;
//	int tmp = GetInput();
//	ENSURE(ret,tmp);
//	tmp = HandleQueryString(dict);
//	ENSURE(ret, tmp);
//	return ret;
//}


//brief:����������������
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

	//��ȡdocλ����Ϣ
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

//brief:���ط�����������
int Query::_load_inverse_data(unordered_map<string, string> &inverse_index, const char *file_path) {
	if (!file_path) {
		return kERROR_QUERY_LOAD_DATA;
	}
	auto flag = ios::in | ios::binary;
	std::fstream ifsIndex(file_path, flag);
	if (!ifsIndex) {
		return kERROR_QUERY_LOAD_DATA;
	}

	//��ȡdocλ����Ϣ
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



//brief:����TF-IDF������ҳ�÷�
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
			//�鲻�������
			continue;
		}
		//����idf
		int  word_doc_number;
		const char *ptr_start = mapInverse[word].c_str();
		if (1 != sscanf(ptr_start, sscanf_format.c_str(), &word_doc_number)) {
			//�����ļ�������
			continue;
		}
		const char *ptr_end = strchr(ptr_start, kSEPARATE_CHAR_INVERSE_FILE);
		//int word_doc_number = std::stoi(string(ptr_start, ptr_end));
		float idf = log((float)total_doc_number / word_doc_number);
		//��¼��Щidf
		vec_words.emplace_back(word);
		vec_idf.emplace_back(idf);
		//����ÿһ����ҳ��Ȩ��
		ptr_start = ptr_end + 1;
		while (word_doc_number > 0) {
			int docid;
			int doc_count;
			if (2 != sscanf(ptr_start, sscanf_format4doc.c_str(), &docid, &doc_count)) {
				return kERROR_QUERY_DATA;
			}
			//���·���
			tmp_score[docid] = idf * doc_count;
			//score_ret.update({ docid,idf*doc_count });
			//��λ����һ��λ��
			word_doc_number--;
			//�����쳣��ʽ
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
	//�������,,,ע�⽵������
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

//brief:��־��¼
int Query::_log_query() {
	time_t tv= time(nullptr);
	if (0 != _logger(m_query_string.c_str(), ctime(&tv))) {
		return kERROR_LOG;
	}
	return 0;
}


