/*************************************************************************
	> File Name: page_sort.cpp
	> Author: Dream9
	> Brief:
	> Created Time: 2019年07月04日 星期四 15时02分37秒
 ************************************************************************/
#include"page_sort.h"

static int _load_forward_data(vector<pair<int, int>> &forward_index, const char *file_path);
const int kMAX_BUFFER_SIZE = 1024 * 1024;

#define ENSURE_EXIST(x) if (!x) {\
		err_msg("%s:can not access file", __FUNCTION__);\
		return -1;}
#define ENSURE_ZERO(x) if(0!=x){\
		err_msg("%s:unexcepted error",__FUNCTION__);\
		return -1;}

//brief:外部接口
//parameter:url-docid的索引文件，原始数据文件，docid-原始数据文件的索引文件，PR结果输出文件
int Pagesort::SortPage(const char *url_index, const char *raw_data, const char *doc_idx, const char *out_put) {
	FILE *tempfile = fopen("tempfile", "wb+");
	ENSURE_EXIST(tempfile);
	ENSURE_ZERO(_load_url_indx(url_index));
	ENSURE_ZERO(_get_outdegree(raw_data, doc_idx,tempfile));
	ENSURE_ZERO(_sort_page(tempfile,out_put));
	fclose(tempfile);
	//ENSURE_ZERO(unlink("tempfile"));
	return 0;
}

//brief:载入数据，关联id
int Pagesort::_load_url_indx(const char *url_index) {
	FILE *ifs = fopen(url_index, "rb");
	ENSURE_EXIST(ifs);
	char buf[128];
	memset(buf, 0, sizeof buf);
	int docid;

	while (2 == fscanf(ifs, "%128s %d", buf, &docid)) {
		_m_mapUrlId.insert({ buf,docid });
	}
	if (ferror(ifs)) {
		err_msg("%s:unexpected error", __FUNCTION__);
		fclose(ifs);
		return -1;
	}
	fclose(ifs);
	return 0;
}

#define ENSURE_EXIST_OR_FREE_BUF(x,y) if(!x){err_msg("%s:error",__FUNCTION__);\
										free(y);return -1;}
//brief:统计出度，并保存到临时文件中（已经unlink）
int Pagesort::_get_outdegree(const char *raw_data, const char *doc_idx, FILE *temp) {
	FILE *ifs_data = fopen(raw_data, "rb");
	ENSURE_EXIST(ifs_data);
	vector<pair<int, int>> forward_idx;
	ENSURE_ZERO(_load_forward_data(forward_idx, doc_idx));
	Page page_parser;
	Md5 md5_parser;

	int len = forward_idx.size() - 1;
	for (int i = 0; i < len;++i) {
		int data_len = forward_idx[i + 1].second - forward_idx[i].second;
		char *buf = (char *)calloc(sizeof(char), data_len + 1);
		ENSURE_EXIST(buf);
		try {
			fseek(ifs_data, forward_idx[i].second, SEEK_SET);
			fread(buf, data_len, 1, ifs_data);
			//定位到url
			char *ptrUrl = strstr(buf, "url:");
			ENSURE_EXIST_OR_FREE_BUF(ptrUrl, buf);
			ptrUrl += sizeof"url:" - 1;
			char *ptrHead = strchr(ptrUrl, '\n');
			ENSURE_EXIST_OR_FREE_BUF(ptrHead, buf);
			*ptrHead = '\0';
			md5_parser.GenerateMd5((uint8_t *)ptrUrl,ptrHead-ptrUrl);
			string cur_url_md5 = md5_parser.ToString();
			auto iter = _m_mapUrlId.find(cur_url_md5);
			if (iter == _m_mapUrlId.end()) {
				err_msg("%s:current url:%s is not found", __FUNCTION__, ptrUrl);
				continue;
			}
			int cur_doc_id = iter->second;
			//定位到响应头
			ptrHead = strstr(ptrHead + 1, "\n\n");
			ENSURE_EXIST_OR_FREE_BUF(ptrHead, buf);
			ptrHead += 2;
			//定位到响应体
			char *ptrContent = strstr(ptrHead, "\n\n");
			ENSURE_EXIST_OR_FREE_BUF(ptrContent, buf);
			*++ptrContent = '\0';
			++ptrContent;
			//提取出链
			page_parser.SetPage(ptrUrl, "", ptrHead, ptrContent, data_len - (ptrContent - buf));
			page_parser.ParseHeaderInfo(page_parser.m_sHeader);
			page_parser.ParseHyperLinks();
			//更新出度
			for (auto cur : page_parser.m_mapLink4SE) {
				md5_parser.GenerateMd5((uint8_t *)cur.first.c_str(),cur.first.size());
				string url_md5 = md5_parser.ToString();
				auto iter = _m_mapUrlId.find(url_md5);
				if (iter == _m_mapUrlId.end()) {
					continue;
				}
				//记录出度；格式:15 20\n 表示15->20
				//输出到一个临时文件之中
				fprintf(temp, "%d %d\n", cur_doc_id, iter->second);
			}
		}
		catch (...) {
			free(buf);
			throw;
		}
		free(buf);
	}
	//
	fseek(temp, 0, SEEK_SET);
	return 0;
}
#undef ENSURE_EXIST_OR_FREE_BUF

//brief:
int Pagesort::_sort_page(FILE *temp, const char *out_put) {
	std::ofstream coo_ofs(out_put, std::ios::trunc | std::ios::out | std::ios::binary);
	std::ostream_iterator<double> coo_os(coo_ofs, "\n");
	timeval end_tv;
	timeval start_tv;
	//迭代计算过程
	log_msg("PageRank():start...");
	gettimeofday(&start_tv, nullptr);
	Pagerank_sparse coo_pagerank_parser(0.85,1e-6,100);
	coo_pagerank_parser.OpenData(temp);
	coo_pagerank_parser.Init();
	coo_pagerank_parser.Calc();
	coo_pagerank_parser.Show(coo_os);
	coo_pagerank_parser.Clear();
	gettimeofday(&end_tv, nullptr);
	log_msg("PageRank used %d ms", (end_tv.tv_sec - start_tv.tv_sec) * 1000 + end_tv.tv_usec - start_tv.tv_usec);
	coo_ofs.close();

	return 0;
}

//brief:辅助函数，加载正向索引数据
int _load_forward_data(vector<pair<int, int>> &forward_index, const char *file_path) {
	if (!file_path) {
		return -1;
	}
	auto flag = ios::in | ios::binary;
	std::fstream ifsDocIndex(file_path, flag);
	if (!ifsDocIndex) {
		return -1;
	}

	//获取doc位置信息
	string strLine;
	while (getline(ifsDocIndex, strLine)) {
		if (strLine.empty() || strLine[0] == '\0' || strLine[0] == '\n' || strLine[0] == '\r' || strLine[0] == '#') {
			continue;
		}
		int docid, position;
		if (2 != sscanf(strLine.c_str(), "%d %d %*s", &docid, &position)) {
			return -1;
		}
		forward_index.emplace_back(std::make_pair(docid, position));
	}
	ifsDocIndex.close();
	return 0;
}

#undef ENSURE_ZERO
#undef ENSURE_EXIST
