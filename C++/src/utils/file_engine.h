/*************************************************************************
	> File Name: file_engine.h
	> Author: Dream9
	> Brief:通过文件的形式进行数据保存，规定了必要的API接口
	> Created Time: 2019年05月10日 星期五 16时11分43秒
 ************************************************************************/

#ifndef _UTILS_FILEENGINE_H_
#define _UTILS_FILEENGINE_H_

#include"data_engine.h"
#include"url.h"
#include"page.h"
#include"errlog.h"

#include<fstream>

using std::string;
using std::ofstream;
using std::ios;

enum file_engine_type{
	kISAM=1,        //索引顺序存储
	kORIGINAL_PAGE, //原始网页转储
	kLINK4SE        //LINK信息
};


struct file_arg{
	Url *ptrUrl;
	Page *ptrPage;
};

//becare:依然是一个纯虚类
class FileEngine:public DataEngine{
public:
	ofstream m_ofsFile;//out stream

public:
	FileEngine(const string &);
	FileEngine();
	virtual ~FileEngine();

	int GetEngineType() const {
		return kFILE_ENGINE;
	}

	int Open(const string &);
	void Close(){
		m_ofsFile.close();
	}
   //virtual int Write(void *){};
};

#endif
