/*************************************************************************
	> File Name: md5.h
	> Author: Dream9
	> Brief: 
	> Created Time: 2019年04月24日 星期三 22时10分15秒
 ************************************************************************/
#ifndef _UTILS_MD5_H_
#define _UTILS_MD5_H_

#include"errlog.h"

#include<string>

using namespace std;


//负责MD5值的产生
class Md5{
private:
	struct Md5_Context{
		uint32_t total[2];
		uint32_t state[4];
		uint8_t buffer[64];
	};

	void md5_starts(Md5_Context *);
	void md5_process(Md5_Context *, uint8_t [64]);
	void md5_update(Md5_Context *,uint8_t *,uint32_t);
	void md5_finish(Md5_Context *,uint8_t [16]);

public:
    void GenerateMd5(unsigned char *,int);

	Md5();
	Md5(const char *);
	Md5(uint32_t *);
	Md5 operator +(const Md5 &);
    Md5& operator +=(const Md5 &);
	bool operator ==(const Md5 &) const;
	bool operator !=(const Md5 &) const;

    //输出为string
	string ToString();

private:
	//4*32=128位的MD5结果
	uint32_t m_data[4];
};

#endif

