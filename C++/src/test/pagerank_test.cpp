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
	const char *file = "../doc/page.dat";
	const char *out_bf = "../doc/PR_bf.dat";
	const char *out_coo = "../doc/PR_coo.dat";
	//struct timeval tv;
	//std::ostream_iterator<double> os(std::cout, " ");
	timeval end_tv;
	timeval start_tv;

	//测试BF计算
	std::vector<std::pair<int,double>>bf_out;
	std::ofstream ofs(out_bf, std::ios::trunc | std::ios::out | std::ios::binary);
	//std::ostream_iterator<double> os(ofs, "\n");
	gettimeofday(&start_tv, nullptr);
	Pagerank pagerank_parser;
	pagerank_parser.OpenData(file);
	pagerank_parser.Init();
	pagerank_parser.Calc();
	pagerank_parser.GetSortedResult(bf_out);
	std::for_each(bf_out.begin(),bf_out.end(),[&](const std::pair<int,double>&val){ofs<<val.first<<' '<<val.second<<'\n';});
	//pagerank_parser.Show(os);
	pagerank_parser.Clear();
	gettimeofday(&end_tv, nullptr);
	printf("BF used %f s\n", static_cast<double>((end_tv.tv_sec - start_tv.tv_sec) + (end_tv.tv_usec - start_tv.tv_usec)*0.001));
	ofs.close();

	//测试稀疏矩阵
	std::vector<std::pair<int,double>>coo_out;
	std::ofstream coo_ofs(out_coo, std::ios::trunc | std::ios::out | std::ios::binary);
	//std::ostream_iterator<double> coo_os(coo_ofs, "\n");
	gettimeofday(&start_tv, nullptr);
	Pagerank_sparse coo_pagerank_parser;
	coo_pagerank_parser.OpenData(file);
	coo_pagerank_parser.Init();
	coo_pagerank_parser.Calc();
    coo_pagerank_parser.GetSortedResult(coo_out);
	std::for_each(coo_out.begin(),coo_out.end(),[&](const std::pair<int,double>&val){coo_ofs<<val.first<<' '<<val.second<<'\n';});
	//std::copy(coo_out.begin(),coo_out.end(),coo_os);
	//coo_pagerank_parser.Show(coo_os);
	coo_pagerank_parser.Clear();
	gettimeofday(&end_tv, nullptr);
	printf("BF used %f s\n", static_cast<double>((end_tv.tv_sec - start_tv.tv_sec) + (end_tv.tv_usec - start_tv.tv_usec)*0.001));
	coo_ofs.close();
	return 0;
}
