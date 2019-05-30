/*************************************************************************
	> File Name: crawl_test.cpp
	> Author: Dream9
	> Brief: 
	> Created Time: 2019年05月23日 星期四 17时00分41秒
 ************************************************************************/

#include"crawl.h"
int main() {
	Crawl c("seed", "out", 2,  100, 10);
	c.Collect();
	printf("end");
}
