/*************************************************************************
	> File Name: dictinary.cpp
	> Author: Dream9
	> Brief:
	> Created Time: 2019��06��03�� ����һ 14ʱ51��54��
 ************************************************************************/
#include"dictinary.h"

 //brief:�򿪴ʵ��ļ�������ȡ����
 //parameter:�ʵ�·�����Ƿ����Ƶ��
 //becare:flag==0ʱ�����
int Dictinary::OpenDictinary(const char *file_path, int flag) {
	if(0 == flag){
		return OpenDictinary(file_path);
	}
	//��մ���
	FILE *ofsDict=fopen(file_path, "rb");
	if (!ofsDict) {
		err_ret("%s:cann't access %s", __FUNCTION__, file_path);
		return -1;
	}
	//��ȡ����
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

//brief:�򿪴ʵ��ļ�������ȡ����,�����Ƶ��
//parameter:�ʵ�·��
int Dictinary::OpenDictinary(const char *file_path) {
	FILE *ofsDict=fopen(file_path, "rb");
	if (!ofsDict) {
		err_ret("%s:cann't access %s", __FUNCTION__, file_path);
		return -1;
	}
	//��ȡ����
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

//brief:�ж��Ƿ�λ�ڴʵ���
int Dictinary::IsWord(const string &str) const {
	//if (hashDict.empty()) {
	//	return -1;
	//}
	if (hashDict.find(str) != hashDict.end()) {
		return 1;
	}
	return 0;
}

//brief:���ͳ�ƴ���
int Dictinary::GetFrequence(const string &str) {
	if (IsWord(str)) {
		return -1;
	}
	return hashDict[str];
}

//brief:����ͳ�ƴ���
int Dictinary::AddFrequence(const string &str) {
	if (IsWord(str)) {
		return -1;
	}
	hashDict[str]++;
	return 0;
}
