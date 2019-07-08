/*************************************************************************
	> File Name: pagerank_sparse.cpp
	> Author: Dream9
	> Brief:
	> Created Time: 2019年07月01日 星期一 15时02分21秒
 ************************************************************************/

#include"pagerank_sparse.h"

//brief:
//int Pagerank::OpenData(const char *file) {
//	;
//	return 0;
//}

//brief:初始化计算矩阵
int Pagerank_sparse::Init() {
	if (m_mapOutdegree.empty()) {
#ifdef __GNUC__
		err_msg("%s:empty data",__FUNCTION__);
#endif
		return -1;
	}
	double original_pr = 1.0 / _max_id;//初始pr都相等
	//初始PR值
	pr_type(_max_id, original_pr).swap(PR);

	_nonzero_number = 0;
	std::for_each(m_mapOutdegree.begin(), m_mapOutdegree.end(), [&](const degre_type::value_type &c) {_nonzero_number += c.second.size(); });
	if (0 == _nonzero_number) {
#ifdef __GNUC
		err_msg("%s:all is none", __FUNCTION__);
#endif // __GNUC
		return -1;
	}
	//采用COO格式
	//CSS压缩列还需要二分定位
	//row.resize(_max_id + 1);
	_row.resize(_nonzero_number);
	_column.resize(_nonzero_number);
	_value.resize(_nonzero_number);
	auto iter = m_mapOutdegree.begin();
	int cur = 0;
	int index=0;
	//becare:出度为零的网页存在一项矫正，，，不要忘记了！！！！
	//becare:url索引是从1开始编写，对应0数组索引位置
	while (iter != m_mapOutdegree.end()) {
		int i = iter->first - 1;
		while(index!=i){
			_dead_page.emplace_back(index++);
		}
		double weight_with_alpha = _alpha * 1.0 / iter->second.size();
		for (int j : iter->second) {
			if(--j==i){//防止自环
				continue;
			}
			_row[cur] = j;
			_column[cur] = i;
			_value[cur++] = weight_with_alpha;
		}
		++iter;
		++index;
	}
	while(index<_max_id){
		_dead_page.emplace_back(index++);
	}
	//row[_max_id] = _nonzero_number;
	return 0;
}

//brief:基于COO稀疏矩阵计算
int Pagerank_sparse::Calc() {
	if (PR.empty() || _row.empty() || _column.empty() || _value.empty()) {
		err_msg("%s:empty data", __FUNCTION__);
		return -1;
	}
	int iteration_count = 0;
	double jump_dead_page_value = _alpha * 1.0 / _max_id;//dead_page修正值
	pr_type last_pr(PR.begin(), PR.end());
	auto ptrPR=&PR;
	
	//test,检查 不做优化的G矩阵
	//g_type G(_max_id, vector<double>(_max_id, (1.0-_alpha)*1.0/_max_id));
	//for (int pos : _dead_page) {
	//	for (int j = 0; j < _max_id; ++j) {
	//		G[j][pos] += _alpha * 1.0 / _max_id;
	//	}
	//}
	//for (int i = 0; i < _row.size(); ++i) {
	//	G[_row[i]][_column[i]] += _value[i];
	//}
	
	while (iteration_count < _max_iteration_depth) {
		//零元素矩阵相乘的默认值
		double default_jump_value = (1.0 - _alpha) / _max_id * std::accumulate(PR.begin(), PR.end(), 0.0);
		//double dead_page_adjust=std::accumulate(_dead_page.begin(), _dead_page.end(), 0.0, [&ptrPR](double init, int id) {return init + (*ptrPR)[id]; });
		default_jump_value += std::accumulate(_dead_page.begin(), _dead_page.end(), 0.0, [&](double init, int id) {return init + (*ptrPR)[id] * jump_dead_page_value; });
		std::for_each(PR.begin(), PR.end(), [default_jump_value](pr_type::value_type &i) {i = default_jump_value; });
		int len_tmp = _row.size();
		//COO稀疏矩阵乘法
		for (int i = 0; i < len_tmp; ++i) {
			PR[_row[i]] += last_pr[_column[i]] * _value[i];
		}
		//迭代终止？
		bool continue_flag = false;
		for (int i = 0; i < _max_id; ++i) {
			if (fabs(PR[i] - last_pr[i]) > _threshold) {
				continue_flag = true;
				break;
			}
		}
		if (!continue_flag) {
			break;
		}
		//更新状态
		++iteration_count;
		memcpy(&last_pr[0], &PR[0], PR.size() * sizeof(PR[0]));
	}
	return 0;
}

//brief:输出pr结果
int Pagerank_sparse::Show(std::ostream_iterator<double> &os) {
	std::copy(PR.begin(), PR.end(), os);
	return 0;
}

