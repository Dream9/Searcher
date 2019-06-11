/*************************************************************************
	> File Name: word_segment_test.cpp
	> Author: Dream9
	> Brief: 测试
	> Created Time: 2019年06月09日 星期日 17时14分12秒
 ************************************************************************/

#include"dictinary.h"
#include"word_segement.h"

#include<iostream>
#include<fstream>
using namespace std;

int main(int argc, char *argv[]) {
	/*
	if (argc != 2) {
		cout << "Usage: filename" << endl;
		exit(0);
	}
	*/
	const char *path = "../doc/wordseg_test.txt";
	const char *dict_path = "../doc/word.dict";

	string FileName = path;
	ifstream fin(FileName.c_str());
	ofstream fout((FileName + ".seg").c_str());
	log_msg("file:%s",path);

	Dictinary dict;
	dict.OpenDictinary(dict_path, 1);

	string line;
	WordSegment word_parser; 
	while (getline(fin, line)) {
		line = word_parser.Segment_need_preprose(dict, line);
		if(line.empty()){
			continue;
		}
		fout << line << endl;
	}

	fin.close();
	fout.flush();
	fout.close();

	return(0);
}
