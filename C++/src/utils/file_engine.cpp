/*************************************************************************
	> File Name: file_engine.cpp
	> Author: Dream9
	> Brief: 
	> Created Time: 2019年05月10日 星期五 17时20分18秒
 ************************************************************************/
#include"file_engine.h"

FileEngine::FileEngine(const string &str) : DataEngine(str) {
	//注意二进制模式打开可以避免CRLF混乱的问题
	m_ofsFile.open(m_str.c_str(),ios::out|ios::app|ios::binary);
	if(!m_ofsFile){
		err_ret("Can't open %s",str);
	}
}

FileEngine::FileEngine(){}

FileEngine::~FileEngine(){
    m_ofsFile.close();
}

int FileEngine::Open(const string &str){
	m_str=str;
	m_ofsFile.open(m_str,ios::out|ios::app|ios::binary);

	if(!m_ofsFile){
		err_ret("Can't open %s",str);
		return -1;
	}

	return 0;
}
