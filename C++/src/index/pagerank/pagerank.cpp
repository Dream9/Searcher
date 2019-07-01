/*************************************************************************
	> File Name: pagerank.cpp
	> Author: Dream9
	> Brief:
	> Created Time: 2019��06��29�� ������ 10ʱ18��05��
 ************************************************************************/
#include"pagerank.h"

#define ENSURE(x) if(!x){err_ret("%s:fopen() faild", __FUNCTION__);return -1;}
#define MAX_N(x,y) (x>y?x:y)
//brief:��ȡ��ҳ������Ϣ
int Pagerank::OpenData(const char *filepath) {
	FILE *ifs = fopen(filepath, "rb");
#ifdef __GNUC__
	ENSURE(ifs);
#endif
	int urlid;
	int out_urlid;
	while (2 == fscanf(ifs, "%d %d", &urlid, &out_urlid)) {
		auto iter = m_mapOutdegree.find(urlid);
		if (iter != m_mapOutdegree.end()) {
			iter->second.emplace_back(out_urlid);
		}
		else {
			m_mapOutdegree.insert({ urlid,{out_urlid} });
		}
		_max_id = MAX_N(_max_id, (MAX_N(urlid, out_urlid)));
	}
	if (!feof(ifs)) {
		//err_msg("%s:unexpected file end", __FUNCTION__);
		return -1;
	}
	return 0;

}
#undef ENSURE

//brief:
//becare:Ҫ��url id��1��ʼ
//      todo:ϡ������Ż�������
int Pagerank::Init() {
	if (m_mapOutdegree.empty()) {
#ifdef __GNUC__
		err_msg("%s:empty data", __FUNCTION__);
#endif
		return -1;
	}
	int num = _max_id;
	double original_pr = 1.0 / num;//��ʼpr�����
	double dead_page = original_pr;//������ҳ������ֵ
	double jump_value = (1 - _alpha) / num;//�����ת����ֵ
	//��ʼG����,�Ѿ����������ת����ֵ
	g_type(num, vector<double>(num, jump_value)).swap(G);
	//��ʼPRֵ
	pr_type(num, original_pr).swap(PR);

	//���ݳ��Ⱥ�������ҳ����ֵ����G����
	auto iter = m_mapOutdegree.begin();
	for (int i = 0; i < num; ++i) {
		if (iter!= m_mapOutdegree.end() && iter->first == i + 1) {
			//ע�͵���Ϊ�ϸ�ʽ����
			//int out_degree = iter->second.size();
			//double weight = 1.0 / out_degree;
			//double weight_with_alpha=weight*alpha;
			double weight_with_alpha = _alpha * 1.0 / iter->second.size();
			for (int outid : iter->second) {
				G[i][outid-1] = weight_with_alpha;
			}
			++iter;
		}
		else {
			//���������ҳ����Ҫ����һ������ֵ
			for (int j = 0; j < num; ++j) {
				G[j][i] += dead_page;
			}
		}
	}
	//��ղ���Ҫ������
	degre_type().swap(m_mapOutdegree);
	return 0;
}

//brief:�����������
int Pagerank::Calc() {
	if (G.empty() || PR.empty()) {
#ifdef __GNUC__
		err_msg("%s:no data", __FUNCTION__);
#endif
		return -1;
	}
	int depth = 0;
	pr_type lastPR(PR.begin(),PR.end());
	size_t bytes_len = PR.size() * sizeof PR[0];

	while (depth<_max_iteration_depth) {
		//�����µ�PR
		bool continue_flag = false;
		for (int i = 0; i < _max_id; ++i) {
			double new_pr = std::inner_product(G[i].begin(), G[i].end(), lastPR.begin(), 0.0);
			if (!continue_flag && fabs(new_pr - PR[i]) > _threshold) {
				continue_flag=true;
			}
			PR[i] = new_pr;
		}
		//������ֹ��
		if (!continue_flag) {
			break;
		}
		//����
		memcpy(&lastPR[0], &PR[0], bytes_len);
		++depth;
	}
	//��վ���
	g_type().swap(G);
	//std::sort(PR.begin(), PR.end(), std::greater<double>());
	return 0;
}

//brief:���PR���
int Pagerank::Show(std::ostream_iterator<double> &out) {
	std::copy(PR.begin(), PR.end(), out);
	//pr_type().swap(PR);
	return 0;
}
