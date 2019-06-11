/*************************************************************************
	> File Name: dictinary.cpp
	> Author: Dream9
	> Brief:
	> Created Time: 2019年06月03日 星期一 14时51分54秒
 ************************************************************************/
#include"dictinary.h"

 //brief:打开词典文件，并读取数据
 //parameter:词典路径，是否清空频率
 //becare:flag==0时不清空
int Dictinary::OpenDictinary(const char *file_path, int flag) {
	if(0 == flag){
		return OpenDictinary(file_path);
	}
	//清空处理
	FILE *ofsDict=fopen(file_path, "rb");
	if (!ofsDict) {
		err_ret("%s:cann't access %s", __FUNCTION__, file_path);
		return -1;
	}
	//读取数据
	char word[17];
	memset(word, 0, sizeof word);
	int frequence;
	while (fscanf(ofsDict, "%*d  %16s %d", word, &frequence) != EOF) {
		hashDict[word] = 0;
	}
	if (!feof(ofsDict)) {
		fclose(ofsDict);
		return -1;
	}
	fclose(ofsDict);
	return 0;
}

//brief:打开词典文件，并读取数据,不清空频率
//parameter:词典路径
int Dictinary::OpenDictinary(const char *file_path) {
	FILE *ofsDict=fopen(file_path, "rb");
	if (!ofsDict) {
		err_ret("%s:cann't access %s", __FUNCTION__, file_path);
		return -1;
	}
	//读取数据
	char word[17];
	memset(word, 0, sizeof word);
	int frequence;
	while (fscanf(ofsDict, "%16s %d", word, &frequence) != EOF) {
		hashDict[word] = frequence;
	}
	if (feof(ofsDict)) {
		fclose(ofsDict);
		return -1;
	}
	fclose(ofsDict);
	return 0;
}

//brief:判读是否位于词典中
int Dictinary::IsWord(const string &str) const {
	//if (hashDict.empty()) {
	//	return -1;
	//}
	if (hashDict.find(str) != hashDict.end()) {
		return 1;
	}
	return 0;
}

//brief:获得统计次数
int Dictinary::GetFrequence(const string &str) {
	if (IsWord(str)) {
		return -1;
	}
	return hashDict[str];
}

//brief:增加统计次数
int Dictinary::AddFrequence(const string &str) {
	if (IsWord(str)) {
		return -1;
	}
	hashDict[str]++;
	return 0;
}
