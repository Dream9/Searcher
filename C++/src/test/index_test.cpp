/*************************************************************************
	> File Name: index_test.cpp
	> Author: Dream9
	> Brief: 测试
	> Created Time: 2019年06月08日 星期六 10时18分14秒
 ************************************************************************/

#include"index.h"
#include"dictinary.h"
int main() {
	const char *dict_path = "../doc/word.dict";
	const char *raw_path = "../doc/raw.test";
	const char *url_idx_path = "../doc/url.index.test";
	const char *doc_idx_path = "../doc/doc.index.test";
	const char *seg_path = "../doc/doc.seg.test";
	const char *inverse_path = "../doc/inverse.index.test";

	Index idx;
	Dictinary dict;
	dict.OpenDictinary(dict_path,1);
	idx.CreateForwardIndex(raw_path, url_idx_path, doc_idx_path);
	idx.DocumentSegment(raw_path, doc_idx_path,seg_path, dict);
	idx.CreateInverseIndex(seg_path, inverse_path);

	return 0;
}

