/*************************************************************************
	> File Name: document.h
	> Author: Dream9
	> Brief: 
	> Created Time: 2019年05月31日 星期五 15时15分23秒
 ************************************************************************/

#ifndef _INDEX_DOCUMNET_H_
#define _INDEX_DOCUMENT_H_

#include"strfun.h"
#include"charset_transfer.h"

#include<string>
using std::string;

//重新调整，记录数据文件的名称
//
//becare:所有的数据文件应该位于同一个文件中，
//       offset为此文档在这个唯一文件中的位置
//       否则还需要记录路径

class Document {
private:
	struct DocIdx {
		int DocId;
		int Offset;
	};
public:
	DocIdx m_Doc;
	string m_sMd5;


	//string m_sUrl;
	string m_sContent_Refined;
	int m_nLength;
	int m_nFile;

public:
	Document(){
		m_Doc.DocId=-1;
		m_Doc.Offset=-1;
		m_nLength=-1;
		m_sMd5="";
		m_sContent_Refined = "";
		m_nFile=-1;
	}
	int Refine(const char *, string &);

};

#endif
