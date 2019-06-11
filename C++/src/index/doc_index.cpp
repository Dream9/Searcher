/*************************************************************************
	> File Name: doc_index.cpp
	> Author: Dream9
	> Brief: 
	> Created Time: 2019年05月31日 星期五 15时52分52秒
 ************************************************************************/
#include"doc_index.h"

//指定使用外部排序的界限
extern const unsigned int kMAX_SORT_LIMIT;
//版本号
extern const string kVERSION_ORIGINAL_PAGE;

using type_md5 = array<char,33>;
using type_urlid = pair<type_md5, int>;
//最大偏移量offset的位数（近似计算：假设pos(2,10)==pos(10,3)）
static const int kMAX_OFFSET_SIZE = sizeof(Document::m_nLength) * 8 / 10 * 3;
//分隔符
const char kSEPARATE_CHAR_DOC = ' ';

//static int _write_to_urlindex_file(fstream&);

//brief:将原始数据文件重命名
//return:空白文件将返回-2,其他错误-1
int Rename(const char *old_name, const char *new_name){
	struct stat stat_buf;
    if(lstat(old_name, &stat_buf) < 0 ){
		err_ret("lstat() failed:%s",old_name);
		return -1;
	}
	if(stat_buf.st_size==0){
		return -2;
	}

	return rename(old_name, new_name);
}

#define ENSURE_ERROR(x,y) if(!x){err_ret("Open file faild():%s",y);return -1;}
#define DEAL_WITH_FILE_BROKEN(x,y,z) err_msg("File(%s) is Broken at:%d", x, y);\
			z++;\
			if (z > 3) {\
				err_msg("Too Many Errors in File(%s),Quit", x);\
				return -1;\
			}
//brief:根据数据文件建立：
//      url->docid的索引文件url.idx（有序）;
//      docid->data pos的索引文件:doc.idx(这其实就是正向索引文件)
//parameter:原始数据文档路径，url.idx路径，doc.idx路径
//return:错误信息
//becare:会截断原有文件，如需合并，外部调用KMerge;
//      需要经过排序方便后续查找；
//      当超过内部排序的数量上限时，采用外部排序k路归并
int CreateDocIndex(const char *filepath, const char *url_idx_file, 
	               const char *doc_idx_file){
	ifstream ifsData(filepath);
	ENSURE_ERROR(ifsData, filepath);

	auto mode = ios::in | ios::out | ios::binary | ios::trunc;
	fstream ofsUrl(url_idx_file, mode);
	ENSURE_ERROR(ofsUrl, url_idx_file);

	fstream ofsDoc(doc_idx_file, mode);
	ENSURE_ERROR(ofsDoc, doc_idx_file);

	int count=1;
	int file_error_number = 0;
	vector<type_urlid>container;
	string strLine;
	Document doc_parser;
	Md5 md5_parser;
	type_md5 md5_tmp;

	md5_tmp[32] = '\0';
	const string kversion = "version:" + kVERSION_ORIGINAL_PAGE;
	int offset = ifsData.tellg();//becare:记录“version:XXX"出现的位置，故总是延迟出现的（读后才发现）
     /*头部示例
	  version:1.0
      url:http://att.neea.edu.cn/html1/category/1511/1350-71.htm
      date:Thu, 30 May 2019 02:07:00 GMT
      ip:120.220.40.17
      length:27267
	*/
	while (getline(ifsData, strLine)) {
		if (strLine[0] == '\n' || strLine[0] == '\0' || strLine[0] == '#') {
			offset = ifsData.tellg();
			continue;
		}
		if (0 != memcmp(strLine.c_str(), kversion.c_str(), kversion.size())) {
			offset = ifsData.tellg();
			continue;
		}
		//提取文档信息,并作文件完整性检查,超过3处错误将会放弃这个文件
		if (!getline(ifsData, strLine)) {
			err_msg("Unexpected File EOF");
			break;//其他正确的数据应该予以保存
		}
		if (0 == memcmp(strLine.c_str(), "url:", sizeof "url" -1)) {
			string tmp_url = strLine.substr(4);
			md5_parser.GenerateMd5((uint8_t*)tmp_url.c_str(), tmp_url.size());
		}
		else {
			DEAL_WITH_FILE_BROKEN(filepath, offset, file_error_number);
			offset = ifsData.tellg();
		}
		while (getline(ifsData, strLine)) {
			if (0 == memcmp(strLine.c_str(), "length:", sizeof "length:" - 1)) {
				//过大的网页已经被筛出
				sscanf(strLine.c_str(), "%*[^:]:%d", &doc_parser.m_nLength);
				break;
			}
		}
		//跳过分割行
		if(!getline(ifsData, strLine)){
			err_msg("%s:Unexpected file end",__FUNCTION__);
			break;
		}
		//检查合法性
		if (doc_parser.m_nLength < 0) {
			err_msg("Bad Doc length(<0)");
			offset = ifsData.tellg();
			continue;
		}
		//信息记录
		//becare:记录的时version:XXX起始的位置，连续两次的位置差并不是网页信息中的length
		doc_parser.m_Doc.DocId = count;
		doc_parser.m_Doc.Offset = offset;
		memcpy(&md5_tmp[0], md5_parser.ToString().c_str(), 32);
		container.emplace_back(md5_tmp, doc_parser.m_Doc.DocId);

		char *buf = (char*)calloc(sizeof(char), doc_parser.m_nLength + 1);
		ifsData.read(buf, doc_parser.m_nLength);
		md5_parser.GenerateMd5((uint8_t*)buf, doc_parser.m_nLength);

		//ofsDoc信息记录
		ofsDoc << doc_parser.m_Doc.DocId << kSEPARATE_CHAR_DOC \
			<< doc_parser.m_Doc.Offset << kSEPARATE_CHAR_DOC \
			<< md5_parser.ToString() << '\n';
		count++;
		offset = ifsData.tellg();
		//if(count % 102400 == 0) {
		//	ofsDoc.flush();
		//}
	}
	//ofsDoc的终结标记
	ofsDoc << count << kSEPARATE_CHAR_DOC << offset << endl;
	//ofsUrl信息记录
	if (container.size() < kMAX_SORT_LIMIT) {
		std::sort(container.begin(), container.end());
	}
    for (type_urlid cur : container) {
		ofsUrl << &cur.first[0] << kSEPARATE_CHAR_DOC << cur.second << '\n';
	}
	ofsUrl.close();
	ofsDoc.close();
	ifsData.close();
    if (container.size() < kMAX_SORT_LIMIT) {
		//已经排序
		return 0;
	}
	//外部排序
	return KMerge(&url_idx_file,1);
}
#undef ENSURE_ERROR
#undef DEAL_WITH_FILE_BROKEN

//brief:K路归并，内存消耗过大时调用
//becare:当number只有一个元素时，默认是当前文件内容过大，无法内部排序
//       此时，应当将该文件分隔为多个文件后，递归调用该函数
int KMerge(const char **files, int number) {
	//

	return 0;
}
