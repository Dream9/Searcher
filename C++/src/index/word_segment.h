/*************************************************************************
	> File Name: word_segment.h
	> Author: Dream9
	> Brief: ���ķִʣ�
	> Created Time: 2019��06��03�� ����һ 14ʱ34��17��
 ************************************************************************/
#ifndef _INDEX_WORD_SEGMENT_H_
#define _INDEX_WORD_SEGMENT_H_

#include"dictinary.h"
#include"errlog.h"

class WordSegment {
public:
	WordSegment() {};

	string Segment_need_preprose(Dictinary &, const string&) const;
	string Segment(Dictinary &, const string&) const;

	//����
	int SegmentUrl(Dictinary &, string&) const;

private:
	int TranslateURLChar(char *) const;
	int _segment_mm(Dictinary &, const string &, int, string &) const;
	int _segment_rmm(Dictinary &, const string &, int, string &) const;

	//����
	int _refine_document(const string &, string &) const;//�ظ�
	
};

#endif
