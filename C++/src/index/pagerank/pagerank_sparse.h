/*************************************************************************
	> File Name: pagerank_sparse.h
	> Author: Dream9
	> Brief:
	> Created Time: 2019��07��01�� ����һ 14ʱ52��59��
 ************************************************************************/

#ifndef _INDEX_PAGERANK_SPARSE_H_
#define _INDEX_PAGERANK_SPARSE_H_

#include"pagerank.h"

#include<functional>

//brief:����ϡ������PageRank
class Pagerank_sparse :public Pagerank {
protected:
	//����CSR��ϡ������¼��ʽ
	using sparse_value_type = vector<double>;
	using sparse_index_type = vector<int>;
	sparse_index_type _column;
	sparse_index_type _row;
	sparse_value_type _value;
	sparse_index_type _dead_page;//��¼����Ϊ0����ҳ
	int _nonzero_number;
public:
	//virtual int OpenData(const char *file);/*�����޸�*/
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