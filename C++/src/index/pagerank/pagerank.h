/*************************************************************************
	> File Name: pagerank.h
	> Author: Dream9
	> Brief:
	> Created Time: 2019��06��29�� ������ 10ʱ12��45��
 ************************************************************************/

#ifndef _INDEX_PAGERANK_H
#define _INDEX_PAGERANK_H

#include"errlog.h"

#include<map>
#include<cmath>
#include<vector>
#include<string.h>
#include<cstdlib>
#include<cstdio>
#include<algorithm>
#include<numeric>
#include<iterator>

using std::string;
using std::map;
using std::vector;
using std::pair;
const double kTHRESHOLD = 1e-6;
const int kITERATION_DEPTH = 50;


//brief:BF������ҳPRֵ
class Pagerank{
protected:
	typedef vector<int> outpage_container_type;
	typedef map<int, outpage_container_type> degre_type;
	typedef vector<double> pr_type;
	typedef vector<vector<double>> g_type;

	//����PR���й�״ֵ̬
	degre_type m_mapOutdegree;
	pr_type PR;
	g_type G;

	double _alpha;//��ת�Ƹ��ʣ�Ĭ��0.85
	double _threshold;//������ֵ
	int _max_iteration_depth;//���������
	int _max_id;//becare:�������url�����1��_max_id����todo

public:
	typedef vector<pair<int,double>> out_type;

	virtual int OpenData(const char *filepath);
	virtual int Init();
	virtual int Calc();
	virtual int Show(std::ostream_iterator<double> &out);
	virtual void Clear() {
		pr_type().swap(PR);
		g_type().swap(G);
		degre_type().swap(m_mapOutdegree);
	}
	virtual int GetSortedResult(out_type&);

	Pagerank(double alpha = 0.85, double threshold = kTHRESHOLD, int depth = kITERATION_DEPTH) : _alpha(alpha),
		_threshold(threshold), _max_iteration_depth(depth), _max_id(0){  }

};


#endif

