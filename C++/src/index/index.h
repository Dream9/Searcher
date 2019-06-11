/*************************************************************************
	> File Name: index.h
	> Author: Dream9
	> Brief: 
	> Created Time: 2019��06��06�� ������ 15ʱ28��45��
 ************************************************************************/

#ifndef _INDEX_INDEX_H_
#define _INDEX_INDEX_H_

#include"doc_index.h"
#include"dictinary.h"
#include"word_segement.h"
#include"charset_transfer.h"
#include"errlog.h"
#include"md5.h"
#include"strfun.h"
#include"index_config.h"

#include<fstream>
#include<cstdlib>

#include<sys/wait.h>
#include<unistd.h>

//brief:����������Ϣ������
//     ����˳��Ϊ1��2��3
class Index {
public:
	//1.ͨ��ԭʼ�ĵ�������������
	int CreateForwardIndex(const char *, const char *, const char *);
    //2.����������������ĵ��д�
	int DocumentSegment(const char *, const char *, const char *, Dictinary &);
	//3.ͨ���д��ĵ����ɷ�������
	int CreateInverseIndex(const char *, const char*);

	//todo:
	int MergeInverseFile(const char **, int);
private:
	int _group_and_sort_segment_file(const char *, const char *);
};



#endif
