#pragma once
#ifndef _BPLUSTREE_H
#define _BPLUSTREE_H

//������
#define DEBUG_1
#define ERROUT(x) printf("bad :  %s:%d--!!!"#x,__FUNCTION__,__LINE__);
#define ENSURE(x) if(x);else {ERROUT(x);system("pause");exit(EXIT_FAILURE);}
#define NODEBUG(x) x;
//#define NODEBUG(x)

#ifndef ssize_t
    #define ssize_t int
#endif

#include<cstdlib>
#include<cstdio>
#include<utility>
#include<memory>
#include<cassert>

#ifdef LINUX
    #inlucde<unistd.h>
#else
    #include<iostream>
#endif

//�����������������
#include<queue>

//����һ�������ռ��ж���
namespace BTree {

	const ssize_t UNREADABLE = -1;

	//ѡ��1st����
	//
	template < class _Pair>
	struct _Select1st {
		typename _Pair::first_type & operator()(_Pair& __x) const {
			return __x.first;
		}
		const typename _Pair::first_type & operator()(const _Pair& __x) const {
			return __x.first;
		}
	};
	//֤ͬ����
	template<class _Ty>
	struct _Identity {
		_Ty& operator()(_Ty __x) const {
			return __x;
		}
		const _Ty& operator()(const _Ty __x) const {
			return __x;
		}
	};

	//�ͷ���Դ
	template<class T>
	inline void _Destroy(T* pointer) {
		pointer->~T();
	}

	//���͵�ѡ��
	template<bool cond, class first, class second>
	struct _IF {
		typedef typename first type;
	};
	//��ӵ�����һ��ģ�壬������
	template<class first, class second>
	struct _IF<false, first, second> {
		typedef typename second type;
	};


	/*pram��Ӧ���ṩ��Ӧ�Ĳ���*/
	//�����node��ͬʱ�����˶����ͬ���͵Ľڵ㣬����pram�ṩ�Ĳ����ṩ��Ӧ�Ľṹ����tree�ж���Ҫʹ���Ǹ��ӿ�
	//��Ҫ��leaf��internalҪ���ֿ���
	template<typename pram>
	struct node {
		typedef typename pram::value_type value_type;
		typedef node<pram> self_type;
		typedef self_type* pointer;
		struct b_base_node;
		//typedef b_base_node* pointer;
		enum {
			_B = (pram::targetBytes - sizeof(b_base_node)) / sizeof(value_type),
			Mtarget = _B < 3 ? 3 : _B,///////����,�ָ���������������
			M = Mtarget >> 1/////Mtarget��ָ�����������M������ʹ����
		};
		//����
		struct b_base_node {
			/*enum {
				M = pram::targetBytes;
			};*/
			ssize_t num;//key�Ѿ�������Ŀ
			bool is_leaf;//�Ƿ�ΪҶ��
			pointer parent;//���ڵ�
			//����׷�ӣ�
			size_t position;
			bool is_root() {
				//������ƣ����ڵ�ĸ��ڵ���һ��Ҷ�ӽڵ�
				return parent->_node.is_leaf;
			}
		};

		//b���ػ�node
		struct b_leaf_node :b_base_node {

			value_type key[Mtarget - 1];
			//pointer ptr[Mtarget];
		};
		struct b_internal_node :public b_leaf_node {
			pointer ptr[Mtarget];
		};

		//b+���ػ�node
		struct bplus_leaf_node :public b_leaf_node {
			//����������ǰ��ָ��
			pointer prev;
			pointer next;
		};
		struct bplus_internal_node :public bplus_leaf_node {
			pointer ptr[Mtarget];
		};

		//�����ӿ�
		//����ת����static_cast��������ǰ�߲���������ת�������߻�������ת������ʱ����ָ��
		//�ֹ������ⲿ����ռ䣬�ڴ˴���ʼ�����൱��construct
		struct init_b_leaf_node {
			pointer operator()(b_leaf_node* f, pointer parent = nullptr,size_t pos=-1) {
				pointer n = reinterpret_cast<pointer>(f);/////////
				f->num = 0;
				f->is_leaf = true;
				f->parent = parent;
				f->position = pos;
				NODEBUG(printf("init b_leaf_node,key len:%d \r\n", sizeof(f->key)));
				ENSURE(memset(&f->key, 0, sizeof(f->key)));

				return n;
			}
		};

		struct init_b_internal_node {
			pointer operator()(b_internal_node* f, pointer parent = nullptr,size_t pos=-1) {
				pointer n = init_b_leaf_node()(f, parent,pos);
				f->is_leaf = false;
				f->parent = parent;
				f->position = pos;
                NODEBUG(printf("init b_internal_node,ptr len:%d \r\n", sizeof(f->ptr)));
				ENSURE(memset(&f->ptr, 0, sizeof(f->ptr)));

				return n;
			}
		};

		struct init_bplus_leaf_node {
			pointer operator()(bplus_leaf_node* f, pointer parent = nullptr,size_t pos=-1) {
				pointer n = reinterpret_cast<pointer>(f);///////
				f->num = 0;
				f->is_leaf = true;
				f->parent = parent;
				f->position = pos;
				f->prev = nullptr;
				f->next = nullptr;
				ENSURE(memset(&f->key, 0, sizeof(f->key)));

				return n;
			}
		};

		struct init_bplus_internal_node {
			pointer operator()(bplus_internal_node* f, pointer parent = nullptr,size_t pos=-1) {
				pointer n = init_bplus_leaf_node()(f, parent,pos);
				f->is_leaf = false;
				ENSURE(memset(&f->ptr, 0, sizeof(f->ptr)));

				return n;
			}
		};
		//btree_node *n = reinterpret_cast<btree_node*>(f);
		//���ݲ�������ʹ���Ǹ��汾��Ϊһ���ɵ��ö�����г�ʼ��
		//
		typedef typename _IF<pram::type, bplus_internal_node, b_internal_node>::type node_type;
		typedef typename _IF<pram::type, init_bplus_internal_node, init_b_internal_node>::type init_internal;
		typedef typename _IF<pram::type, bplus_internal_node, b_internal_node>::type internal_node;
#ifndef DEBUG_1
		typedef typename _IF<pram::type, bplus_leaf_node, b_leaf_node>::type leaf_node;
		typedef typename _IF<pram::type, init_bplus_leaf_node, init_b_leaf_node>::type init_leaf;
#else
		typedef typename _IF<pram::type, bplus_internal_node, b_internal_node>::type leaf_node;
		typedef typename _IF<pram::type, init_bplus_leaf_node, init_b_leaf_node>::type init_leaf;
#endif


	public:
		//ֻ�����ռ����
		node_type _node;
	};




	//Tree,,,�������ܵĽӿ�
	template<typename pram>
	class Tree {
	public:
		//ע��typedef������Ҳ���յ�publicӰ���
		/*
		**�������е�typedef��ͨ��ģ���Ƶ������ģ�������Ҫ���¶���һ�£������ڲ�ȷ��״̬
		**/
		typedef node<pram> node_type;
		typedef node_type* node_pointer;
		typedef typename node_type::init_leaf init_leaf;
		typedef typename node_type::init_internal init_internal;
		typedef int size_type;
		typedef typename pram::compare compare;/////����key_type�ıȽϲ���
		typedef typename node_type::leaf_node leaf_node;////////leaf_node���ͣ�����������Ƕ�Ӧ��malloc,construct��
		typedef typename node_type::internal_node internal_node;///////internal_node����

		//value_type ��key_type��set��һ��
		//��map�в�ͬ��ǰ��Ϊpair
		//keyofvalue��set��Ϊidentity
		//��map��Ϊselect1st
		typedef typename pram::value_type value_type;
		typedef typename pram::key_type key_type;
		typedef typename pram::keyofvalue keyofvalue;
		//using Alloc = std::allocator<node_type>;
		enum {
			M = node_type::M///////����Ӧ�÷����pointer���������M<<1��
		};
	protected:
		//�������Ľṹ�����STL�к��������֯��
		//header�ĸ��ڵ�Ϊ����root��ͬʱroot�ĸ��ڵ�Ҳ��header
		//header->key[0]ָ��leftmost
		//header->key[1]ָ��rightmost
		//���header����Ҳ��Ϊend����
		node_pointer header;

		FILE* pfile;
		size_type btree_node_num;

		//INIT���
		//����һ��tree
		virtual node_pointer btree_create() = 0;
		//����һ��node
		virtual node_pointer btree_node_new(ssize_t i) = 0;////i��ʾ����Ľڵ����ͣ���0Ϊleaf,1Ϊinternal

		//����һ�����޵Ľڵ�
		virtual size_type btree_split_child(node_pointer parent, size_type pos, node_pointer child) = 0;
		//����һ���ڵ�,ֵΪtarget,�����汾���ڵ�ǰ�ڵ�Ϊ���������
		virtual std::pair<node_pointer,bool> btree_insert(node_pointer root, value_type target) = 0;
		//virtual void btree_insert_nofull(node_pointer root, value_type target) = 0;
		//�ϲ�����M-1Ԫ�صĽڵ�
		virtual void btree_merge_child(node_pointer root, size_type pos, node_pointer y, node_pointer z) = 0;
		//ɾ��һ��Ҷ�ӽڵ㣬��root�ڵ�������M���������
		virtual node_pointer btree_delete(node_pointer root, value_type target) = 0;
		virtual void btree_delete_nonone(node_pointer root, value_type target) = 0;

		//Iterator���
		//Ѱ��ǰ���ڵ�
		virtual value_type btree_search_predecessor(node_pointer root) = 0;
		//Ѱ�Һ󸽽ڵ�
		virtual value_type btree_search_successor(node_pointer root) = 0;
		//ǰ�����˵ĸ�����������ʵӦ�õ�����һ��iterator��
		//virtual void increament(node_pointer root) = 0;
		//virtual void decreament(node_pointer root) = 0;

		//������ת
		virtual void btree_shift_to_left(node_pointer root, size_type pos, node_pointer y, node_pointer z) = 0;
		virtual void btree_shift_to_right(node_pointer root, size_type pos, node_pointer y, node_pointer z) = 0;
		//������ʽ,LNR,��ȱ���
		virtual void btree_inorder_traversal(node_pointer root) = 0;
		virtual void btree_level_traversal(node_pointer root) = 0;
		//����
		virtual void Save(node_pointer root) = 0;
	public:
		void _destroy(node_pointer R) {
			//�ͷ�R�ڵ㼰R�ڵ����µ�������Ϣ
			//�ݹ�ʵ��
			free(R);
			R = nullptr;
			return;
			if (R) {
				ssize_t len = R->_node.num;
				/*
				for (ssize_t i = 0; i < len; ++i) {
					_Destroy(R->_node.key[i]);
				}
				*/
				for (ssize_t i = 0; i <= len; ++i) {
					_destroy(R->_node.ptr[i]);
				}
				free(R);
			}
		}
		void __erase(node_pointer R) {
            if (R) {
				ssize_t len = R->_node.num;
				
				//��������
				for (ssize_t i = 0; i < len; ++i) {
					_Destroy(&R->_node.key[i]);//ע��ȡ������
				}
				if (!R->_node.is_leaf) {
					for (ssize_t i = 0; i <= len; ++i) {
						__erase(R->_node.ptr[i]);
					}
				}
				NODEBUG(printf("Ready for releasing memory at %p\r\n", R));
				free(R);
			}

		}

		//clear����
		void clear() {
			//�����stl��ʵ��˼·
			//clear���ڵ�����������deallocate
			if (btree_node_num != 0) {
				__erase(root());
				leftmost() = header;
				root() = 0;
				rightmost() = header;
				btree_node_num = 0;
			}
		}

		Tree() {
			btree_node_num = 0;
			//��Ҫ�������ó�ʼ��tree
		}

		//����
		virtual ~Tree() {
			if (header != nullptr) {
				clear();
				NODEBUG(printf("Release header at %p\r\n",header));
				free(header);
			}
			if (pfile != nullptr)
				fclose(pfile);
		}
		////////�����ṩһ�½ӿ�
		void insert(value_type target) {
			header = btree_insert(header, target);
		}
		void level_traversal() {
			btree_level_traversal(header);
		}
		void del(value_type target) {
			header = btree_delete(header, target);
		}
		void inorder_traversal() {
			btree_inorder_traversal();
		}
		//һ������������Ϊ�˷����޸Ľṹ��Ϣ
		node_pointer& leftmost() const {
			NODEBUG(ENSURE(header));
			return header->_node.ptr[0];
		}
		node_pointer& rightmost() const {
			NODEBUG(ENSURE(header));
			return header->_node.ptr[1];
		}
		node_pointer& root() const {
			NODEBUG(ENSURE(header));
			return header->_node.parent;
		}

		//�����routine
		/*
		virtual node_pointer begin() { return leftmost(); }
		virtual const node_pointer begin() const { return leftmost(); }
		virtual node_pointer end() { return header; }
		virtual const node_pointer end() const { return header; }
		*/

	};

	template<class pram>
	class btree_iter {
		//�Ѿ��̳���std��iteratorģ�壬��Ҫ�����Ѿ�����
	public:
		typedef node<pram> node_type;
		typedef node_type* node_pointer;
		typedef btree_iter<pram> self_type;

		using iterator_category = std::bidirectional_iterator_tag;
		using value_type = typename pram::value_type;
		using pointer = value_type * ;
		using reference = value_type & ;
		using distance = ptrdiff_t;
	protected:
		node_pointer x;
		ssize_t pos;
	public:
		void increment_slow();
		void decrement_slow();
		//����˫��ṹ���++/--
		//Ҷ����δ����ʱ����increment()���������increment_slow()


		void increment() {
			//�ȴ�ʵ��
			NODEBUG(ENSURE(x != nullptr));
			if (x == nullptr)
				return;
			if (x->_node.is_leaf && x->_node.num > ++pos) {
				return;
			}
			increment_slow();
		}

		//�Լ�����

		void decrement() {
			//
			NODEBUG(ENSURE(x != nullptr));
			if (x == nullptr)
				return;
			if (x->_node.is_leaf && --pos >= 0) {
				return;
			}
			decrement_slow();
		}

		//������Ϊ�������ĳ�������ʵ��
		reference operator*() {
			return x->_node.key[pos];
		}
		pointer operator->() {
			return &(operator*());
		}
		self_type operator++(int) {
			self_type tmp = *this;
			increment();
			return tmp;
		}
		self_type& operator++() {
			increment();
			return *this;
		}
		self_type operator--(int) {
			self_type tmp = *this;
			decrement();
			return tmp;
		}
		self_type& operator--() {
			decrement();
			return *this;
		}
		self_type& operator=(const self_type& y) {
			x = y.x;
			pos = y.pos;
			return *this;
		}
		bool operator==(const self_type& y) {
			return y.x == x && y.pos == pos;
		}
		bool operator!=(const self_type& y) {
			return y.x != x || y.pos != pos;
		}


		//����
		btree_iter(node_pointer n, ssize_t p) { x = n; pos = p; }
		btree_iter() { x = nullptr; pos = -1; }
		btree_iter(const self_type& y) { x = y.x; pos = y.pos; }
		//btree_iter() { ; }
	};

	//�ⲿʵ��
	template<class pram>
	void btree_iter<pram>::increment_slow() {
		if (x->_node.is_leaf) {
			//���1��Ҷ�ӳ���
			assert(pos >= x->_node.num);
			while (pos == x->_node.num && !x->_node.is_root()) {
				//�ص����ڵ�ȥ
				pos = x->_node.position;
				x = x->_node.parent;
			}

			if (pos == x->_node.num) {
				NODEBUG(printf("%d ", pos));
				x = x->_node.parent;////����header�ڵ�
				pos = -1;
			}
		}
		else {
			//���2��λ���м�ڵ����������
			assert(pos < x->_node.num());
			//++������Ҳ�ķ�֧�������
			x = x->_node.ptr[pos + 1];
			while (!x->_node.is_leaf) {
				x = x->_node.ptr[0];
			}
			pos = 0;
		}
	}


	template<class pram>
	void btree_iter<pram>::decrement_slow() {
		//ͬ��
		if (x->_node.is_leaf) {
			//���1��Ҷ�ӳ���
			assert(pos < 0);
			while (pos < 0 && !x->_node.is_root()) {
				//�ص����ڵ�ȥ
				pos = x->_node.position - 1;
				x = x->_node.parent;
			}

			if (pos < 0) {
				NODEBUG(printf("%d ", pos));
				x = x->_node.parent;///��Ӧ������header
				pos = -1;
			}
		}
		else {
			//���2��λ���м�ڵ����������
			assert(pos >= 0);
			//++�����zuo��ķ�֧����you��
			x = x->_node.ptr[pos];
			while (!x->_node.is_leaf) {
				x = x->_node.ptr[x->_node.num];
			}
			pos = x->_node.num - 1;
		}
	}

	/*Btree��*/
	//�̳���Tree
	//����ܹ���header��һ��internal�����Ǳ����Ϊleaf,��ָ����������ң������൱��end,��������root������Ϊ���ڵ㣬�Դ�����
	template<typename pram>
	class Btree :public Tree<pram> {
		//������ĵ�����
	public:
		typedef btree_iter<pram> iterator;   ////������
		typedef Tree<pram> super_type;
		typedef super_type::node_type node_type;
		typedef super_type::node_pointer node_pointer;
		typedef super_type::size_type size_type;
		typedef super_type::init_leaf init_leaf;////////�ɵ��ö������ڳ�ʼ��leaf
		typedef super_type::init_internal init_internal;//////���ڳ�ʼ��internal
		typedef super_type::compare compare;///�Ƚϲ���
		typedef typename node_type::leaf_node leaf_node;////////leaf_node���ͣ�����������Ƕ�Ӧ��malloc,construct��
		typedef typename node_type::internal_node internal_node;///////internal_node����
		//value_type ��key_type��set��һ��
		//��map�в�ͬ��ǰ��Ϊpair
		//keyofvalue��set��Ϊidentity
		//��map��Ϊselect1st
		typedef typename pram::value_type value_type;
		typedef typename pram::key_type key_type;
		typedef typename pram::keyofvalue keyofvalue;
		//����ļ�������Ҫusing֮��Żᱻ������ʾ��������������������
		using super_type::root;
		using super_type::header;
		using super_type::leftmost;
		using super_type::rightmost;
		using super_type::M;
		using super_type::btree_node_num;
		using super_type::pfile;
		using super_type::_destroy;
		using super_type::clear;
	protected:
		//INIT��أ����캯������֮
		//��ʼ��tree
		virtual node_pointer btree_create();
		//����һ��node
		virtual node_pointer btree_node_new(ssize_t i); //0Ϊinternal,����Ϊleaf
		//��������
		virtual node_pointer btree_node_new_leaf();//
		virtual node_pointer btree_node_new_internal();

		//����һ�����޵Ľڵ�
		virtual size_type btree_split_child(node_pointer parent, size_type pos, node_pointer child);
		//����һ���ڵ�,ֵΪtarget,�����汾���ڵ�ǰ�ڵ�Ϊ���������
		virtual std::pair<node_pointer,bool> btree_insert(node_pointer root, value_type target);
		virtual std::pair<node_pointer,bool> btree_insert_nofull(node_pointer root, value_type target);
		//�ϲ�����M-1Ԫ�صĽڵ�
		virtual void btree_merge_child(node_pointer root, size_type pos, node_pointer y, node_pointer z);
		//ɾ��һ��Ҷ�ӽڵ㣬��root�ڵ�������M���������
		virtual node_pointer btree_delete(node_pointer root, value_type target);
		virtual void btree_delete_nonone(node_pointer root, value_type target);

		//Iterator ���
		//Ѱ��ǰ���ڵ�
		virtual value_type btree_search_predecessor(node_pointer root);
		//Ѱ�Һ󸽽ڵ�
		virtual value_type btree_search_successor(node_pointer root);
		//ǰ�����˵ĸ�����������ʵӦ�õ�����һ��iterator��
		//virtual void increament(node_pointer root);
		//virtual void decreament(node_pointer root);

		//������ת
		virtual void btree_shift_to_left(node_pointer root, size_type pos, node_pointer y, node_pointer z);
		virtual void btree_shift_to_right(node_pointer root, size_type pos, node_pointer y, node_pointer z);
		//������ʽ,LNR,��ȱ���
		virtual void btree_inorder_traversal(node_pointer root);
		virtual void btree_level_traversal(node_pointer root);
		//����
		virtual void Save(node_pointer root);
		//int laji();

	public:
		std::pair<node_pointer, bool> insert_equal(value_type target);
		//�����routine
		iterator begin() { return iterator(leftmost(),0); }
		const iterator begin() const { return iterator(leftmost(),0); }
		iterator end() { return iterator(header,-1); }/////Լ��һ��end�Ľṹ
		const iterator end() const { return iterator(header,-1); }


		//����ʹ��
#ifdef DEBUG_1
		value_type ceshi() {
			return header->_node.parent->_node.key[0];
		}
#endif

	public:
		Btree();
		~Btree();


		

	};


	

	/*Btreeʵ��*/
	template<typename pram>
	typename Btree<pram>::node_pointer Btree<pram>::btree_create() {
		//init_tree
		header = btree_node_new(0);
		//���鷳û�ж��ⶨ��һ��root�ڵ㣬��һ��internal�ڵ����
		ENSURE(header);
		if (header == nullptr)
			exit(EXIT_FAILURE);
		//��ʼ�����ݽṹ
		header->_node.is_leaf = true;///��һ���м�ڵ㵱��leaf��
		//header->_node.num = 1;
		root() = nullptr;
		leftmost() = header;
		rightmost() = header;
		btree_node_num = 0;
		NODEBUG(printf("Tree init: header at %p\r\n", header));
		return header;
	}

	template<typename pram>
	typename Btree<pram>::node_pointer Btree<pram>::btree_node_new(ssize_t i) {
		//���þ��Ƿַ���˫��ṹ
		return i == 0 ? btree_node_new_internal() : btree_node_new_leaf();
	}

	template<typename pram>
	typename Btree<pram>::node_pointer Btree<pram>::btree_node_new_leaf() {
		//������mallocһ��node�ڵ�
		leaf_node* newnode = reinterpret_cast<leaf_node*>(malloc(sizeof(leaf_node)));
		NODEBUG(printf("New Leaf Node at %p\r\n", newnode));
		NODEBUG(printf("len: %d\r\n", sizeof(leaf_node)));
		ENSURE(newnode);
		if (newnode == nullptr)
			return nullptr;

		//Ϊ���node��ʼ������
		//�൱��construct
		//���Ե����������
		//Ĭ���Ǹ�Ҷ��
		/*
		size_type len = M << 1;
		for (size_type i = 0; i < len - 1; ++i) {
			newnode->key[i] = UNREADABLE;
		}
		for (size_type i = 0; i < len; ++i) {
			newnode->ptr[i] = nullptr;
		}
		node->num = 0;
		noed->is_leaf = true;
		*/
		//��ʼ����������ʵ�֣�
		return init_leaf()(newnode, nullptr);
		//return node;
	}
	template<typename pram>
	typename Btree<pram>::node_pointer Btree<pram>::btree_node_new_internal() {
		//������mallocһ��node�ڵ�,����ʼ��
		internal_node* newnode = reinterpret_cast<internal_node*>(malloc(sizeof(internal_node)));
		NODEBUG(printf("New Internal Node at %p\r\n", newnode));
		NODEBUG(printf("len: %d\r\n", sizeof(internal_node)));
		ENSURE(newnode);
		if (newnode == nullptr)
			return nullptr;
		return init_internal()(newnode, nullptr);
	}
	//�������ʼ�����

	//һ����������
	template<typename pram>
	typename Btree<pram>::size_type Btree<pram>::btree_split_child(Btree<pram>::node_pointer parent,
		Btree<pram>::size_type pos, Btree<pram>::node_pointer child) {
		//���������һ��Ԥ�ȷ��ѵĴ������
		//��һ���ڵ��key�ﵽ��2*M-1��ʱ���������ˣ�����������֮
		//�ֳ��������֣�M-1������һ��������parent,M-1�����Ҳ�
		//���ڲ���Ԥ�ȷ��룬���Լ�������һ����parent�У���Ȼ���ᳬ�ޣ�����Ҫ�ݹ�balance
		ssize_t nodetype = child->_node.is_leaf;//�·��ѵĽڵ�Ӧ����ԭ����ͬ������
		node_pointer new_child = btree_node_new(nodetype);
		ENSURE(new_child);
		new_child->_node.num = M - 1;///���Ҳ�ȥ
		//Ҳ����ѭ�����ƣ�ǰ��ʱvalue_type�����˿������캯��
		//copy���߼���֮��࣬���Ƕ���plain_type����һЩ
		std::copy(child->_node.key + M, child->_node.key + (M << 1) - 1, new_child->_node.key);
		if (nodetype == 0) {
			//�м�ڵ㻹Ҫ����ptr
			//ע��ָ���һ��
			std::copy(child->_node.ptr, child->_node.ptr + (M << 1), new_child->_node.ptr);
		}
		//����ʣ�µ�(ǰ���)һ�룬ֻ��Ҫ����һ���ѷ���ռ�Ϳ�����
		child->_node.num = M - 1;
		//�Ը��ڵ�Ĳ���һ��key��ptr
		ssize_t Pnum = parent->_node.num;//���ڵ���ʹ�����
		//std::copy_backward(parent->_node.ptr + pos + 1, parent->_node.ptr + Pnum, parent->_node.ptr + Pnum + 1);//ennmmmm
		std::copy_backward(parent->_node.ptr + pos + 1, parent->_node.ptr + Pnum+1, parent->_node.ptr + Pnum + 2);
		parent->_node.ptr[pos + 1] = new_child;//ָ���ҽڵ�
		//NODEBUG(print("ǰ��%d",parent->_node.key[pos]));
		std::copy_backward(parent->_node.key + pos, parent->_node.key + Pnum, parent->_node.key + Pnum + 1);
		parent->_node.key[pos] = child->_node.key[M - 1];
		parent->_node.num += 1;
		NODEBUG(printf("%d\r\n", parent->_node.num));
		return 0;
	}

	//Ӧ���ٶ���ʵ��һ��insert_unique,,����İ汾��insert_equal
	template<typename pram>
	std::pair<typename Btree<pram>::node_pointer,bool> Btree<pram>::btree_insert_nofull(Btree::node_pointer node, Btree::value_type target) {
		//��δ��������£�num<2*M-1
		//��֤����������Ѱ�ҹ����У��Զ�balance
		//���insert����������nodeΪ��ʱѰ�Ҳ���㣬Ȼ��insert
		if (node->_node.is_leaf == 1) {
			//�Ѿ�����Ҷ��
			//��key��Ѱ�ң�����Btree��Ȼ��������������Զ�������
			//����ͨ���������������˳��copy_backward,,,
			ssize_t pos = node->_node.num;
			while (pos >= 1 && compare()(target, node->_node.key[pos - 1])) {
				node->_node.key[pos] = node->_node.key[pos - 1];
				--pos;
			}
			node->_node.key[pos] = target;
			node->_node.num += 1;
			btree_node_num += 1;

			//Ӧ���Լ�����һ��iterator
			//���򷵻صĶ���һ��node,��û�о�ȷ��key
			return std::make_pair(node, true);
		}
		else {
			//��Ҫ����Ѱ��
			ssize_t pos = node->_node.num;
			while (pos >= 1 && compare()(target, node->_node.key[pos - 1])) {
				--pos;
			}
			ssize_t length = (M << 1) - 1;
			if (length == node->_node.ptr[pos]->_node.num) {
				//��Ҫ����
				//�˴�
				ENSURE(0==btree_split_child(node, pos, node->_node.ptr[pos]));//enmm
				if (compare()(target, node->_node.key[pos]))
					;//��������һ���ڵ�
				else
					++pos;
			}
			return btree_insert_nofull(node->_node.ptr[pos], target);//�ݹ�
		}
	}

	//�����⣬���Ľ�
	template<typename pram>
	std::pair<typename Btree<pram>::node_pointer,bool> Btree<pram>::btree_insert(Btree::node_pointer node, Btree::value_type target) {
		//��һ����飬Ȼ��ת��bree_insert_nofull����
		//ͬʱҪά��header���������
		/*����һ�㱣֤
		if (node == nullptr) {
			return std::make_pair(node,false);
		}
		*/
		ssize_t length = (M << 1) - 1;
		if (length == node->_node.num) {
			//�������ߣ���Ҫ���ѣ�����һ��parent,��Ȼ��internal 
			node_pointer newnode = btree_node_new(0);/////������һ��֮��root�Ų���Ҷ��
			//newnode->_node.num = 1;   //���Ƕ����
			header->_node.parent = newnode;
			newnode->_node.ptr[0] = node;
			btree_split_child(newnode, 0, node);//�ϲ�ʱ����num
			return btree_insert_nofull(newnode, target);
		}
		else {
			return btree_insert_nofull(node, target);
			//return std::make_pair(node,true);
		}
	}
	//insert�ĸĽ�
	template<typename pram>
	std::pair<typename Btree<pram>::node_pointer, bool> Btree<pram>::insert_equal(Btree::value_type target) {
		//node_pointer y = header;
		node_pointer x = root();
		if (x == nullptr) {
			x = btree_node_new(1);//////�޸ĵ�һ��rootӦ����һ��leaf
			x->_node.parent = header;
			header->_node.num = 1;
			x->_node.num = 1;
			x->_node.key[0] = target;
			x->_node.ptr[0] = nullptr;
			x->_node.ptr[1] = nullptr;
			btree_node_num += 1;
			leftmost() = x;
			rightmost() = x;
			root() = x;
			NODEBUG(printf("Insert first node,root at %p\r\n", x));
			return  std::make_pair(x, true);
		}
		return btree_insert(x, target);

	}

	//�ϲ�����
	template<typename pram>
	void Btree<pram>::btree_merge_child(Btree::node_pointer node, Btree::size_type pos, Btree::node_pointer left, Btree::node_pointer right) {
		//������split_child�ķ������
		//������deleteʱ�����Զ�balance
		//��noed��key[pos],��right�ϲ���left��ȥ
		ssize_t length = (M << 1) - 1;
		//�޸�����
		left->_node.num = length;
		//����key
		left->_node.key[M - 1] = node->_node.key[pos];
		std::copy(right->_node.key, right->_node.key + M - 1, left->_node.key + M);
		//�޸�ptr
		if (right->_node.is_leaf == 0) {
			std::copy(right->_node.ptr, right->_node.ptr + M, left->_node.ptr + M);
		}
		//���ڵ����
		length = node->_node.num;
		std::copy(node->_node.ptr + pos + 2, node->_node.ptr + length + 1, node->_node.ptr + pos + 1);
		std::copy(node->_node.key + pos + 1, node->_node.key + length, node->_node.key + pos);
		node->_node.num -= 1;
		//�ͷſռ�
		_destroy(right);
	}

	//delete�ڵ�
	template<typename pram>
	typename Btree<pram>::node_pointer Btree<pram>::btree_delete(Btree::node_pointer node, Btree::value_type target) {
		//�ͺϲ�һ���ĵ���
		//�ų�һЩ���������Ȼ����ýӿ�
		//����ֵ�������µ������ڵ㶼���ٽ�ֵ��M-1��1��M-1����ʱ����ɾ����һ����ȻҪ�������ĸ߶ȣ��ͷŵ���
		if (nullptr == node)
			return nullptr;
		if (node->_node.num == 1) {
			node_pointer left = node->_node.ptr[0];
			node_pointer right = node->_node.ptr[1];
			if (left && right && left->_node.num == M - 1 && right->_node.num == M - 1) {
				//ȫ���ϲ�֮��Ϊ2*M-1������ɾ��һ�����㣬�����ڻ��ݵĿ�����
				btree_merge_child(node, 0, left, right);
				_destroy(node);//��һ��һ����node��num����Ϊ0��
				btree_delete_nonone(left, target);
				return left;
			}
		}
		btree_delete_nonone(node, target);
		return node;
	}

	template<typename pram>
	void Btree<pram>::btree_delete_nonone(Btree::node_pointer node, Btree::value_type target) {
		//�ٱ�֤node�ڵ㲻�����������
		//�������һ��node�ڵ㣬�Ҳ�����ȫ����ɾ��node�ڵ�
		if (1 == node->_node.is_leaf) {
			//���Ҷ�ӽڵ�
			//��insert_equal��ͬ��ɾ����Ԫ�ز�һ������
			//��ȻҪ���������֣����ԣ�
			//���԰Ѳ���Ҳ��װһ��
			ssize_t index = 0;
			ssize_t cap = node->_node.num;
			while (index < cap && compare()(node->_node.key[index], target)) ++index;
			NODEBUG(ENSURE(index < 0));
			if (index < cap && !compare()(target, node->_node.key[index])) {
				std::copy(node->_node.key + index + 1, node->_node.key + cap, node->_node.key + index);
				node->_node.num -= 1;
				btree_node_num -= 1;
			}
			else {
				//û���ҵ�
				NODEBUG(printf("404!\r\n"));
			}
		}
		else {
			//����м�ڵ�
			//�����ж��Ƿ���node��key֮�У����Ծ���Ѱ��ǰ����̽ڵ��滻һ�¼���
			ssize_t index = 0;
			ssize_t cap = node->_node.num;
			node_pointer left;
			node_pointer right;
			//�Կɿ��Ƿ�װ���Ѱ�Ҳ���
			//��ʵ��Ӧ�ø���compare�Ƿ�����ж�==�������·ַ�һ�£�Ҫ��Ȼ��Ҫ�������
			while (index < cap && compare()(node->_node.key[index], target))++index;
			if (index < cap && !compare()(target, node->_node.key[index])) {
				//find it
				//ǰ��
				left = node->_node.ptr[index];
				right = node->_node.ptr[index + 1];
				if (left->_node.num > M - 1) {
					//left�㹻��,�ƹ���
					value_type prekey = btree_search_predecessor(left);
					node->_node.key[index] = prekey;
					btree_delete_nonone(left, prekey);
				}
				else if (right->_node.num > M - 1) {
					//right�㹻��
					value_type nextkey = btree_search_successor(right);
					node->_node.key[index] = nextkey;
					btree_delete_nonone(right, nextkey);
				}
				else {
					//��ʱ����ɾ���Ǹ������ᵼ���ӽڵ㲻������������Ҫ�ϲ�
					btree_merge_child(node, index, left, right);
					btree_delete(left, target);
				}
			}
			else {
				//����node�У���Ҫ����ȥ
				//��ʱkey[index]>target,��ptr[index]ָ��Ķ�<key[index]
				left = node->_node.ptr[index];
				//left�Ƿ��㹻ɾ����������Ҫshift
				//����shift��ʽ��left-1 -��left   or   left+1 -��left
				
				if (left->_node.num == M - 1) {
					//ȷ��û����
					if (index != 0) {
						node_pointer last = node->_node.ptr[index - 1]; 
						if(last->_node.num > M - 1)
							btree_shift_to_right(node, index - 1, last, left);
					}
					else if (index<cap) {
					    right = node->_node.ptr[index + 1];
						if(right->_node.num>M - 1)
							btree_shift_to_right(node, index, left, right);
					}
					else if (index != 0) {
						//�޷�shiftʱ���ϲ���Ȼ��ɾ��
                        node_pointer last = node->_node.ptr[index - 1];
						btree_merge_child(node, index - 1, last, left);
						left = last;
					}
					else {
						//����һ������ϲ�
                        right = node->_node.ptr[index + 1];
						btree_merge_child(node, index, left, right);
					}
				}
				//ȷ��û������,�ݹ���ȥ
				btree_delete_nonone(left, target);
				//}
				//else {
				//	//һ��ʼ��û����
				//	btree_delete_nonone(left, target);
				//}
			}
		}
	}

	//ǰ�����
	template<typename pram>
	typename Btree<pram>::value_type Btree<pram>::btree_search_predecessor(Btree::node_pointer node) {
		//�ҵ�node��������ֵ
		node_pointer most = node;
		while (most->_node.is_leaf == 0) {
			most = most->_node.ptr[most->_node.num];
		}
		return most->_node.key[most->_node.num - 1];
	}

	template<typename pram>
	typename Btree<pram>::value_type Btree<pram>::btree_search_successor(Btree::node_pointer node) {
		//�ҵ�node������Сֵ
		node_pointer most = node;
		while (most->_node.is_leaf == 0) {
			most = most->_node.ptr[most->_node.num];
		}
		return most->_node.key[most->_node.num - 1];
	}

	//shift ����
	//��һ���������ע�����еı�֤���ɵ�������ɣ�����֮�󲻻ᳬ�ޣ�
	template<typename pram>
	void Btree<pram>::btree_shift_to_left(Btree::node_pointer node, Btree::size_type pos, Btree::node_pointer left, Btree::node_pointer right) {
		//�ⲿ��֤right��num>M-1
		//��ĵ������Ҷ�ӣ�Ҳ�������м�ڵ�
		//��Ҫ�ֿ��ж�
		//ʵ������node��pos����left,right����С�����node
		//���ƶ�key
		ssize_t cap = right->_node.num;
		left->_node.num += 1;
		left->_node.key[left->_node.num - 1] = node->_node.key[pos];
		node->_node.key[pos] = right->_node.key[0];
		std::copy(right->_node.key + 1, right->_node.key + cap, right->_node.key);
		//Ȼ���ƶ�ptr
		if (left->_node.is_leaf == 0) {
			left->_node.ptr[left->_node.num] = right->_node.ptr[0];
			std::copy(right->_node.ptr + 1, right->_node.ptr + (cap + 1), right->_node.ptr);
			//���Ǳ�Ҫ���𣿣�
			right->_node.ptr[cap] = nullptr;
		}
		right->_node.num -= 1;
		NODEBUG(printf("%d", right->_node.num));
	}

	template<typename pram>
	void Btree<pram>::btree_shift_to_right(Btree::node_pointer node, Btree::size_type pos, Btree::node_pointer left, Btree::node_pointer right) {
		//��һ���ԳƵ�shift
		//��left�����->root,root->right
		ssize_t cap = right->_node.num;
		ssize_t cap_l = left->_node.num;
		right->_node.num += 1;
		std::copy_backward(right->_node.key, right->_node.key + cap, right->_node.key + cap + 1);
		right->_node.key[0] = node->_node.key[pos];
		node->_node.key[pos] = left->_node.key[cap_l - 1];
		if (left->_node.is_leaf==0) {
			std::copy_backward(right->_node.ptr, right->_node.ptr + cap,	right->_node.ptr + cap + 1);
			right->_node.ptr[0] = left->_node.ptr[cap_l - 1];
			left->_node.ptr[cap_l - 1] = nullptr;
		}
		left->_node.num -= 1;
		NODEBUG(printf("%d", left->_node.num));
	}

	//���������������
	template<typename pram>
	void Btree<pram>::btree_inorder_traversal(Btree::node_pointer node) {
		if (node != nullptr)
			btree_inorder_traversal(node->_node.ptr[0]);
		ssize_t cap = node->_node.num;
		for (ssize_t i = 0; i < cap; ++i) {
			printf("%d,", node->_node.key[i]);
			btree_inorder_traversal(node->_node.ptr[i + 1]);
		}
	}

	//��ȱ�����������
	template<typename pram>
	void Btree<pram>::btree_level_traversal(Btree::node_pointer node) {
		//������queueʵ��
		if (node == nullptr)
			return;
		std::queue<node_pointer>Q;
		Q.push(node);
		while (!Q.empty()) {
			ssize_t cap = node->_node.num;
			node_pointer cur = Q.front();
			Q.pop();
			if (cur == nullptr)
				continue;

			printf("[");
			for (ssize_t i = 0; i < cap; ++i) {
				printf("%d,", cur->_node.key[i]);

			}
			printf("]");
			if (cur->_node.is_leaf == 1) {
				for (ssize_t i = 0; i <= cap; ++i) {
					Q.push(cur->_node.ptr[i]);
				}
			}
		}
	}

	template<typename pram>
	void Btree<pram>::Save(Btree::node_pointer node) {
		//�ȴ���չ
		//ͨ���ڴ�ӳ�������д�룿
		;
	}

	//���캯����ת���ӿڼ���
	template<typename pram>
	Btree<pram>::Btree() {
		btree_create();
	}

	//
	template<typename pram>
	Btree<pram>::~Btree() {
		//Tree::~Tree();
	}
}/////namespace BTree


#endif

