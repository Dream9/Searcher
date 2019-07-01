/*************************************************************************
	> File Name: pagerank_test.cpp
	> Author: Dream9
	> Brief:
	> Created Time: 2019年07月01日 星期一 18时42分08秒
 ************************************************************************/

#include"pagerank.h"
#include"pagerank_sparse.h"

#include<iostream>
#include<fstream>
#include<time.h>

#include<sys/time.h>

int main() {
	const char *file = "page.dat";
	const char *out_bf = "PR_bf.dat";
	const char *out_coo = "PR_coo.dat";
	//struct timeval tv;
	//std::ostream_iterator<double> os(std::cout, " ");
	timeval end_tv;
	timeval start_tv;

	//测试BF
	std::ofstream ofs(out_bf, std::ios::trunc | std::ios::out | std::ios::binary);
	std::ostream_iterator<double> os(ofs, "\n");
	gettimeofday(&start_tv, nullptr);
	Pagerank pagerank_parser;
	pagerank_parser.OpenData(file);
	pagerank_parser.Init();
	pagerank_parser.Calc();
	pagerank_parser.Show(os);
	pagerank_parser.Clear();
	gettimeofday(&end_tv, nullptr);
	printf("BF used %d ms", (end_tv.tv_sec - start_tv.tv_sec) * 1000 + end_tv.tv_usec - start_tv.tv_usec);
	ofs.close();

	//测试稀疏矩阵
	std::ofstream coo_ofs(out_bf, std::ios::trunc | std::ios::out | std::ios::binary);
	std::ostream_iterator<double> coo_os(coo_ofs, "\n");
	gettimeofday(&start_tv, nullptr);
	Pagerank_sparse coo_pagerank_parser;
	coo_pagerank_parser.OpenData(file);
	coo_pagerank_parser.Init();
	coo_pagerank_parser.Calc();
	coo_pagerank_parser.Show(os);
	coo_pagerank_parser.Clear();
	gettimeofday(&end_tv, nullptr);
	printf("BF used %d ms", (end_tv.tv_sec - start_tv.tv_sec) * 1000 + end_tv.tv_usec - start_tv.tv_usec);
	coo_ofs.close();
	return 0;
}