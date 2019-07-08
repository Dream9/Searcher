/*************************************************************************
	> File Name: pagerank_sparse.cpp
	> Author: Dream9
	> Brief:
	> Created Time: 2019��07��01�� ����һ 15ʱ02��21��
 ************************************************************************/

#include"pagerank_sparse.h"

//brief:
//int Pagerank::OpenData(const char *file) {
//	;
//	return 0;
//}

//brief:��ʼ���������
int Pagerank_sparse::Init() {
	if (m_mapOutdegree.empty()) {
#ifdef __GNUC__
		err_msg("%s:empty data",__FUNCTION__);
#endif
		return -1;
	}
	double original_pr = 1.0 / _max_id;//��ʼpr�����
	//��ʼPRֵ
	pr_type(_max_id, original_pr).swap(PR);

	_nonzero_number = 0;
	std::for_each(m_mapOutdegree.begin(), m_mapOutdegree.end(), [&](const degre_type::value_type &c) {_nonzero_number += c.second.size(); });
	if (0 == _nonzero_number) {
#ifdef __GNUC
		err_msg("%s:all is none", __FUNCTION__);
#endif // __GNUC
		return -1;
	}
	//����COO��ʽ
	//CSSѹ���л���Ҫ���ֶ�λ
	//row.resize(_max_id + 1);
	_row.resize(_nonzero_number);
	_column.resize(_nonzero_number);
	_value.resize(_nonzero_number);
	auto iter = m_mapOutdegree.begin();
	int cur = 0;
	int index=0;
	//becare:����Ϊ�����ҳ����һ�������������Ҫ�����ˣ�������
	//becare:url�����Ǵ�1��ʼ��д����Ӧ0��������λ��
	while (iter != m_mapOutdegree.end()) {
		int i = iter->first - 1;
		while(index!=i){
			_dead_page.emplace_back(index++);
		}
		double weight_with_alpha = _alpha * 1.0 / iter->second.size();
		for (int j : iter->second) {
			if(--j==i){//��ֹ�Ի�
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

//brief:����COOϡ��������
int Pagerank_sparse::Calc() {
	if (PR.empty() || _row.empty() || _column.empty() || _value.empty()) {
		err_msg("%s:empty data", __FUNCTION__);
		return -1;
	}
	int iteration_count = 0;
	double jump_dead_page_value = _alpha * 1.0 / _max_id;//dead_page����ֵ
	pr_type last_pr(PR.begin(), PR.end());
	auto ptrPR=&PR;
	
	//test,��� �����Ż���G����
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
		//��Ԫ�ؾ�����˵�Ĭ��ֵ
		double default_jump_value = (1.0 - _alpha) / _max_id * std::accumulate(PR.begin(), PR.end(), 0.0);
		//double dead_page_adjust=std::accumulate(_dead_page.begin(), _dead_page.end(), 0.0, [&ptrPR](double init, int id) {return init + (*ptrPR)[id]; });
		default_jump_value += std::accumulate(_dead_page.begin(), _dead_page.end(), 0.0, [&](double init, int id) {return init + (*ptrPR)[id] * jump_dead_page_value; });
		std::for_each(PR.begin(), PR.end(), [default_jump_value](pr_type::value_type &i) {i = default_jump_value; });
		int len_tmp = _row.size();
		//COOϡ�����˷�
		for (int i = 0; i < len_tmp; ++i) {
			PR[_row[i]] += last_pr[_column[i]] * _value[i];
		}
		//������ֹ��
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
		//����״̬
		++iteration_count;
		memcpy(&last_pr[0], &PR[0], PR.size() * sizeof(PR[0]));
	}
	return 0;
}

//brief:���pr���
int Pagerank_sparse::Show(std::ostream_iterator<double> &os) {
	std::copy(PR.begin(), PR.end(), os);
	return 0;
}

