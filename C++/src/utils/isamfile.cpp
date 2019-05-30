/*************************************************************************
	> File Name: isamfile.cpp
	> Author: Dream9
	> Brief: 
	> Created Time: 2019年05月12日 星期日 15时21分53秒
 ************************************************************************/

#include"isamfile.h"

//弃用
//extern const string kISAM_DATA_FILE_NAME;//data路径
//extern const string kISAM_INDEX_FILE_NAME;//index路径


//construct routine
IsamFile::IsamFile(){
	fpDataFile=nullptr;
	fpIndexFile=nullptr;
}

IsamFile::IsamFile(const string &str):FileEngine(str){
	fpDataFile=nullptr;
	fpIndexFile=nullptr;
}


//brief:记录的分别是和索引名称，并打开索引文件和数据文件，追加模式（总是在最后）
IsamFile::IsamFile(const string &strData, const string &strIndex){
    m_str=strData;
	m_sIndexFileName=strIndex;

	fpDataFile=fopen(m_str.c_str(),"a");
	fpIndexFile=fopen(m_sIndexFileName.c_str(),"a");
	if(!fpDataFile || !fpIndexFile){
		//err_ret("Can't access file");
		fpDataFile=nullptr;
		fpIndexFile=nullptr;
	}
}

//brief:析构
IsamFile::~IsamFile(){
	Close();
}


//brief:打开文件流,追加模式
//return:错误信息，
int IsamFile::Open(const string &strData, const string &strIndex){
	m_str=strData;
	m_sIndexFileName=strIndex;

	fpDataFile=fopen(m_str.c_str(),"a");
	if(!fpDataFile){
		err_ret("Can't open file :%s",m_str.c_str());
		return -1;
	}
	fpIndexFile=fopen(m_sIndexFileName.c_str(),"a");
	if(!fpIndexFile){
		err_ret("Can't open file :%s",m_sIndexFileName.c_str());
		return -2;
	}

	return 0;
}

//brief:关闭流
void IsamFile::Close(){
	//if(EOF==fclose(fpIndexFile) || EOF==fclose(fpDataFile)){
	//	err_ret("Failed to close the FILE*");
	//}
	fclose(fpDataFile);
	fclose(fpIndexFile);
}


//brief:写入数据，注意，待改进
//todo:b+tree实现
int IsamFile::Write(void *arg)
{
	if (!arg || !fpIndexFile || !fpDataFile) {
		return false;
	}

	file_arg *pFile = (file_arg *)arg;

	Url *url_parser = pFile->ptrUrl;
	Page *page_parser = pFile->ptrPage;

	const char* url = NULL;
	const char* buffer = NULL;

	if (page_parser->m_sLocation.length() == 0) {
		url = url_parser->m_sUrl.c_str();
	}
	else {
		url = page_parser->m_sLocation.c_str();
	}

	buffer = page_parser->m_sContent.c_str();
	int len = page_parser->m_sContent.length();
	/////////////////////////////////////////////

	int offsett = ftell(fpDataFile);
	fprintf(fpIndexFile, "%10d", offsett);
	fprintf(fpIndexFile, "%256s\n", url);
	fflush(fpIndexFile);

	fwrite(buffer, 1, len, fpDataFile);

	//write 25 spaces in the file
	for (int i = 0; i < 25; i++) {
		fputc(0, fpDataFile);
	}

	//write 3 '1' in the file
	fputc(1, fpDataFile);
	fputc(1, fpDataFile);
	fputc(1, fpDataFile);

	//write [url] in the file
	fputc(91, fpDataFile);
	fwrite(url, 1, strlen(url), fpDataFile);
	fputc(93, fpDataFile);

	for (int i = 0; i < 25; i++) {
		fputc(0, fpDataFile);
	}

	fflush(fpDataFile);

	return true;
}
