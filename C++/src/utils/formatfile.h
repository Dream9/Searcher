/*************************************************************************
	> File Name: formatfile.h
	> Author: Dream9
	> Brief:原始网页数据的保存 ,type 为 kORIGINAL_PAGE
	> Created Time: 2019年05月10日 星期五 15时25分43秒
 ************************************************************************/
#ifndef _COLLECTOR_FORMATFILE_H
#define _COLLECTOR_FORMATFILE_H

#include"file_engine.h"

const string kVERSION_ORIGINAL_PAGE("1.0");

class FormatFile:public FileEngine{
public:
    FormatFile(const string &);
	FormatFile();

	virtual ~FormatFile();

	int GetFileType(){
		return kORIGINAL_PAGE;
	}

	//写数据
	virtual int Write(void *arg);
};

#endif

