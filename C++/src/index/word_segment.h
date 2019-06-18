/*************************************************************************
	> File Name: word_segment.h
	> Author: Dream9
	> Brief: 中文分词，
	> Created Time: 2019年06月03日 星期一 14时34分17秒
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

	//弃用
	int SegmentUrl(Dictinary &, string&) const;

private:
	int TranslateURLChar(char *) const;
	int _segment_mm(Dictinary &, const string &, int, string &) const;
	int _segment_rmm(Dictinary &, const string &, int, string &) const;

	//弃用
	int _refine_document(const string &, string &) const;//重复
	
};

#endif
