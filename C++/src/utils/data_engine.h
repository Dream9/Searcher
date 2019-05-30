/*************************************************************************
	> File Name: data_engine.h
	> Author: Dream9
	> Brief:数据存储的纯虚类，定义了必须要实现的几个借口
	> Created Time: 2019年05月10日 星期五 16时13分16秒
 ************************************************************************/
#ifndef _UTILS_DATAENGINE_H_
#define _UTILS_DATAENGINE_H_

#include<string>
//#include"dnse.h"
using std::string;

enum data_engine_type{
	kFILE_ENGINE=1,
	kDATABASE_ENGINE=2
};

class DataEngine{
public:
	//可以用来存储路径或者数据库访问的字符串
	string m_str;

public:
	DataEngine(const string &);
	DataEngine();
	virtual ~DataEngine();

	virtual int GetEngineType () const=0;
	virtual int Open(const string &)=0;
	virtual int Write(void *arg)=0;
	virtual void Close()=0;
};



#endif/*_UTILS_DATAENGINE_H_*/
