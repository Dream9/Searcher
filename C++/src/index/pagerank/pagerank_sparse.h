/*************************************************************************
	> File Name: pagerank_sparse.h
	> Author: Dream9
	> Brief:
	> Created Time: 2019年07月01日 星期一 14时52分59秒
 ************************************************************************/

#ifndef _INDEX_PAGERANK_SPARSE_H_
#define _INDEX_PAGERANK_SPARSE_H_

#include"pagerank.h"

#include<functional>

//brief:基于稀疏矩阵的PageRank
class Pagerank_sparse :public Pagerank {
protected:
	//采用CSR的稀疏矩阵记录方式
	using sparse_value_type = vector<double>;
	using sparse_index_type = vector<int>;
	sparse_index_type _column;
	sparse_index_type _row;
	sparse_value_type _value;
	sparse_index_type _dead_page;//记录出度为0的网页
	int _nonzero_number;
public:
	//virtual int OpenData(const char *file);/*不作修改*/
	virtual int Init();
	virtual int Calc();
	virtual int Show(std::ostream_iterator<double> &os);
	virtual void Clear() {
		Pagerank::Clear();
		sparse_index_type().swap(_row);
		sparse_index_type().swap(_column);
		sparse_value_type().swap(_value);
	}
	
	Pagerank_sparse(double alpha = 0.85, double threshold = kTHRESHOLD, int depth = kITERATION_DEPTH) : Pagerank(alpha,threshold,depth){  }

};


#endif