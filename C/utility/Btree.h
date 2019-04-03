#pragma once
#ifndef _BPLUSTREE_H
#define _BPLUSTREE_H

//调试用
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

//按层遍历测试数据用
#include<queue>

//在另一个命名空间中定义
namespace BTree {

	const ssize_t UNREADABLE = -1;

	//选择1st函数
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
	//证同函数
	template<class _Ty>
	struct _Identity {
		_Ty& operator()(_Ty __x) const {
			return __x;
		}
		const _Ty& operator()(const _Ty __x) const {
			return __x;
		}
	};

	//释放资源
	template<class T>
	inline void _Destroy(T* pointer) {
		pointer->~T();
	}

	//类型的选择
	template<bool cond, class first, class second>
	struct _IF {
		typedef typename first type;
	};
	//间接调用另一个模板，特例化
	template<class first, class second>
	struct _IF<false, first, second> {
		typedef typename second type;
	};


	/*pram中应该提供相应的参数*/
	//在这个node中同时定义了多个不同类型的节点，根据pram提供的参数提供对应的结构，由tree判断需要使用那个接口
	//主要是leaf与internal要区分开来
	template<typename pram>
	struct node {
		typedef typename pram::value_type value_type;
		typedef node<pram> self_type;
		typedef self_type* pointer;
		struct b_base_node;
		//typedef b_base_node* pointer;
		enum {
			_B = (pram::targetBytes - sizeof(b_base_node)) / sizeof(value_type),
			Mtarget = _B < 3 ? 3 : _B,///////是用,分隔，，，，，啊啊
			M = Mtarget >> 1/////Mtarget是指针最多数量，M是最少使用量
		};
		//定义
		struct b_base_node {
			/*enum {
				M = pram::targetBytes;
			};*/
			ssize_t num;//key已经分配数目
			bool is_leaf;//是否为叶子
			pointer parent;//父节点
			//额外追加，
			size_t position;
			bool is_root() {
				//特殊设计，根节点的父节点是一个叶子节点
				return parent->_node.is_leaf;
			}
		};

		//b树特化node
		struct b_leaf_node :b_base_node {

			value_type key[Mtarget - 1];
			//pointer ptr[Mtarget];
		};
		struct b_internal_node :public b_leaf_node {
			pointer ptr[Mtarget];
		};

		//b+树特化node
		struct bplus_leaf_node :public b_leaf_node {
			//增加了两个前后指针
			pointer prev;
			pointer next;
		};
		struct bplus_internal_node :public bplus_leaf_node {
			pointer ptr[Mtarget];
		};

		//创建接口
		//这种转换和static_cast区别在于前者不做分析的转换，后者会在子类转到父类时调整指向
		//分工：由外部分配空间，在此处初始化，相当于construct
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
		//根据参数决定使用那个版本作为一个可调用对象进行初始化
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
		//只有这个占容量
		node_type _node;
	};




	//Tree,,,定义了总的接口
	template<typename pram>
	class Tree {
	public:
		//注意typedef的类型也是收到public影响的
		/*
		**当父类中的typedef是通过模板推倒出来的，子类需要重新定义一下，否则处于不确定状态
		**/
		typedef node<pram> node_type;
		typedef node_type* node_pointer;
		typedef typename node_type::init_leaf init_leaf;
		typedef typename node_type::init_internal init_internal;
		typedef int size_type;
		typedef typename pram::compare compare;/////用于key_type的比较操作
		typedef typename node_type::leaf_node leaf_node;////////leaf_node类型，，与上面的是对应的malloc,construct的
		typedef typename node_type::internal_node internal_node;///////internal_node类型

		//value_type ，key_type在set中一样
		//在map中不同，前者为pair
		//keyofvalue在set中为identity
		//在map中为select1st
		typedef typename pram::value_type value_type;
		typedef typename pram::key_type key_type;
		typedef typename pram::keyofvalue keyofvalue;
		//using Alloc = std::allocator<node_type>;
		enum {
			M = node_type::M///////最少应该分配的pointer个数，最多M<<1个
		};
	protected:
		//整个树的结构借鉴了STL中红黑树的组织，
		//header的父节点为树的root，同时root的父节点也是header
		//header->key[0]指向leftmost
		//header->key[1]指向rightmost
		//因此header本身也作为end存在
		node_pointer header;

		FILE* pfile;
		size_type btree_node_num;

		//INIT相关
		//创造一个tree
		virtual node_pointer btree_create() = 0;
		//创建一个node
		virtual node_pointer btree_node_new(ssize_t i) = 0;////i表示分配的节点类型，，0为leaf,1为internal

		//分裂一个超限的节点
		virtual size_type btree_split_child(node_pointer parent, size_type pos, node_pointer child) = 0;
		//插入一个节点,值为target,辅助版本是在当前节点为满的情况下
		virtual std::pair<node_pointer,bool> btree_insert(node_pointer root, value_type target) = 0;
		//virtual void btree_insert_nofull(node_pointer root, value_type target) = 0;
		//合并两个M-1元素的节点
		virtual void btree_merge_child(node_pointer root, size_type pos, node_pointer y, node_pointer z) = 0;
		//删除一个叶子节点，在root节点至少有M个的情况下
		virtual node_pointer btree_delete(node_pointer root, value_type target) = 0;
		virtual void btree_delete_nonone(node_pointer root, value_type target) = 0;

		//Iterator相关
		//寻找前赴节点
		virtual value_type btree_search_predecessor(node_pointer root) = 0;
		//寻找后附节点
		virtual value_type btree_search_successor(node_pointer root) = 0;
		//前进后退的辅助函数，其实应该单独建一个iterator类
		//virtual void increament(node_pointer root) = 0;
		//virtual void decreament(node_pointer root) = 0;

		//两种旋转
		virtual void btree_shift_to_left(node_pointer root, size_type pos, node_pointer y, node_pointer z) = 0;
		virtual void btree_shift_to_right(node_pointer root, size_type pos, node_pointer y, node_pointer z) = 0;
		//遍历方式,LNR,广度遍历
		virtual void btree_inorder_traversal(node_pointer root) = 0;
		virtual void btree_level_traversal(node_pointer root) = 0;
		//保存
		virtual void Save(node_pointer root) = 0;
	public:
		void _destroy(node_pointer R) {
			//释放R节点及R节点以下的所有信息
			//递归实现
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
				
				//调用析构
				for (ssize_t i = 0; i < len; ++i) {
					_Destroy(&R->_node.key[i]);//注意取掷符号
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

		//clear函数
		void clear() {
			//借鉴了stl中实现思路
			//clear用于调用析构，并deallocate
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
			//需要主动调用初始化tree
		}

		//析构
		virtual ~Tree() {
			if (header != nullptr) {
				clear();
				NODEBUG(printf("Release header at %p\r\n",header));
				free(header);
			}
			if (pfile != nullptr)
				fclose(pfile);
		}
		////////对外提供一下接口
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
		//一下三个函数是为了方便修改结构信息
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

		//常规的routine
		/*
		virtual node_pointer begin() { return leftmost(); }
		virtual const node_pointer begin() const { return leftmost(); }
		virtual node_pointer end() { return header; }
		virtual const node_pointer end() const { return header; }
		*/

	};

	template<class pram>
	class btree_iter {
		//已经继承了std的iterator模板，必要参数已经设置
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
		//采用双层结构设计++/--
		//叶子且未超限时采用increment()，否则调用increment_slow()


		void increment() {
			//等待实现
			NODEBUG(ENSURE(x != nullptr));
			if (x == nullptr)
				return;
			if (x->_node.is_leaf && x->_node.num > ++pos) {
				return;
			}
			increment_slow();
		}

		//自减操作

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

		//几个作为迭代器的常规例程实现
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


		//构造
		btree_iter(node_pointer n, ssize_t p) { x = n; pos = p; }
		btree_iter() { x = nullptr; pos = -1; }
		btree_iter(const self_type& y) { x = y.x; pos = y.pos; }
		//btree_iter() { ; }
	};

	//外部实现
	template<class pram>
	void btree_iter<pram>::increment_slow() {
		if (x->_node.is_leaf) {
			//情况1，叶子超限
			assert(pos >= x->_node.num);
			while (pos == x->_node.num && !x->_node.is_root()) {
				//回到父节点去
				pos = x->_node.position;
				x = x->_node.parent;
			}

			if (pos == x->_node.num) {
				NODEBUG(printf("%d ", pos));
				x = x->_node.parent;////就是header节点
				pos = -1;
			}
		}
		else {
			//情况2，位于中间节点的自增操作
			assert(pos < x->_node.num());
			//++后进入右侧的分支的最左侧
			x = x->_node.ptr[pos + 1];
			while (!x->_node.is_leaf) {
				x = x->_node.ptr[0];
			}
			pos = 0;
		}
	}


	template<class pram>
	void btree_iter<pram>::decrement_slow() {
		//同理
		if (x->_node.is_leaf) {
			//情况1，叶子超限
			assert(pos < 0);
			while (pos < 0 && !x->_node.is_root()) {
				//回到父节点去
				pos = x->_node.position - 1;
				x = x->_node.parent;
			}

			if (pos < 0) {
				NODEBUG(printf("%d ", pos));
				x = x->_node.parent;///对应于树的header
				pos = -1;
			}
		}
		else {
			//情况2，位于中间节点的自增操作
			assert(pos >= 0);
			//++后进入zuo侧的分支的最you侧
			x = x->_node.ptr[pos];
			while (!x->_node.is_leaf) {
				x = x->_node.ptr[x->_node.num];
			}
			pos = x->_node.num - 1;
		}
	}

	/*Btree类*/
	//继承自Tree
	//总体架构是header是一个internal，但是被标记为leaf,它指向最左和最右，自身相当于end,它和树的root互相最为父节点，以此区分
	template<typename pram>
	class Btree :public Tree<pram> {
		//不成熟的迭代器
	public:
		typedef btree_iter<pram> iterator;   ////迭代器
		typedef Tree<pram> super_type;
		typedef super_type::node_type node_type;
		typedef super_type::node_pointer node_pointer;
		typedef super_type::size_type size_type;
		typedef super_type::init_leaf init_leaf;////////可调用对象，用于初始化leaf
		typedef super_type::init_internal init_internal;//////用于初始化internal
		typedef super_type::compare compare;///比较操作
		typedef typename node_type::leaf_node leaf_node;////////leaf_node类型，，与上面的是对应的malloc,construct的
		typedef typename node_type::internal_node internal_node;///////internal_node类型
		//value_type ，key_type在set中一样
		//在map中不同，前者为pair
		//keyofvalue在set中为identity
		//在map中为select1st
		typedef typename pram::value_type value_type;
		typedef typename pram::key_type key_type;
		typedef typename pram::keyofvalue keyofvalue;
		//下面的几个必须要using之后才会被编译器示例化！！！！！啊啊啊
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
		//INIT相关，构造函数调用之
		//初始化tree
		virtual node_pointer btree_create();
		//创建一个node
		virtual node_pointer btree_node_new(ssize_t i); //0为internal,非零为leaf
		//辅助函数
		virtual node_pointer btree_node_new_leaf();//
		virtual node_pointer btree_node_new_internal();

		//分裂一个超限的节点
		virtual size_type btree_split_child(node_pointer parent, size_type pos, node_pointer child);
		//插入一个节点,值为target,辅助版本是在当前节点为满的情况下
		virtual std::pair<node_pointer,bool> btree_insert(node_pointer root, value_type target);
		virtual std::pair<node_pointer,bool> btree_insert_nofull(node_pointer root, value_type target);
		//合并两个M-1元素的节点
		virtual void btree_merge_child(node_pointer root, size_type pos, node_pointer y, node_pointer z);
		//删除一个叶子节点，在root节点至少有M个的情况下
		virtual node_pointer btree_delete(node_pointer root, value_type target);
		virtual void btree_delete_nonone(node_pointer root, value_type target);

		//Iterator 相关
		//寻找前赴节点
		virtual value_type btree_search_predecessor(node_pointer root);
		//寻找后附节点
		virtual value_type btree_search_successor(node_pointer root);
		//前进后退的辅助函数，其实应该单独建一个iterator类
		//virtual void increament(node_pointer root);
		//virtual void decreament(node_pointer root);

		//两种旋转
		virtual void btree_shift_to_left(node_pointer root, size_type pos, node_pointer y, node_pointer z);
		virtual void btree_shift_to_right(node_pointer root, size_type pos, node_pointer y, node_pointer z);
		//遍历方式,LNR,广度遍历
		virtual void btree_inorder_traversal(node_pointer root);
		virtual void btree_level_traversal(node_pointer root);
		//保存
		virtual void Save(node_pointer root);
		//int laji();

	public:
		std::pair<node_pointer, bool> insert_equal(value_type target);
		//常规的routine
		iterator begin() { return iterator(leftmost(),0); }
		const iterator begin() const { return iterator(leftmost(),0); }
		iterator end() { return iterator(header,-1); }/////约定一下end的结构
		const iterator end() const { return iterator(header,-1); }


		//测试使用
#ifdef DEBUG_1
		value_type ceshi() {
			return header->_node.parent->_node.key[0];
		}
#endif

	public:
		Btree();
		~Btree();


		

	};


	

	/*Btree实现*/
	template<typename pram>
	typename Btree<pram>::node_pointer Btree<pram>::btree_create() {
		//init_tree
		header = btree_node_new(0);
		//嫌麻烦没有额外定义一个root节点，用一个internal节点代替
		ENSURE(header);
		if (header == nullptr)
			exit(EXIT_FAILURE);
		//初始化数据结构
		header->_node.is_leaf = true;///把一个中间节点当成leaf用
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
		//作用就是分发，双层结构
		return i == 0 ? btree_node_new_internal() : btree_node_new_leaf();
	}

	template<typename pram>
	typename Btree<pram>::node_pointer Btree<pram>::btree_node_new_leaf() {
		//作用是malloc一个node节点
		leaf_node* newnode = reinterpret_cast<leaf_node*>(malloc(sizeof(leaf_node)));
		NODEBUG(printf("New Leaf Node at %p\r\n", newnode));
		NODEBUG(printf("len: %d\r\n", sizeof(leaf_node)));
		ENSURE(newnode);
		if (newnode == nullptr)
			return nullptr;

		//为这个node初始化数据
		//相当于construct
		//可以单独分离出来
		//默认是个叶子
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
		//初始化单独分离实现：
		return init_leaf()(newnode, nullptr);
		//return node;
	}
	template<typename pram>
	typename Btree<pram>::node_pointer Btree<pram>::btree_node_new_internal() {
		//作用是malloc一个node节点,并初始化
		internal_node* newnode = reinterpret_cast<internal_node*>(malloc(sizeof(internal_node)));
		NODEBUG(printf("New Internal Node at %p\r\n", newnode));
		NODEBUG(printf("len: %d\r\n", sizeof(internal_node)));
		ENSURE(newnode);
		if (newnode == nullptr)
			return nullptr;
		return init_internal()(newnode, nullptr);
	}
	//以上与初始化相关

	//一下与分裂相关
	template<typename pram>
	typename Btree<pram>::size_type Btree<pram>::btree_split_child(Btree<pram>::node_pointer parent,
		Btree<pram>::size_type pos, Btree<pram>::node_pointer child) {
		//这里采用了一种预先分裂的处理放肆
		//当一个节点的key达到了2*M-1个时，遍历到此，就主动分离之
		//分成了三部分，M-1个到左，一个上升到parent,M-1个到右侧
		//由于采用预先分离，所以即便上升一个到parent中，仍然不会超限，不需要递归balance
		ssize_t nodetype = child->_node.is_leaf;//新分裂的节点应该与原来是同种类型
		node_pointer new_child = btree_node_new(nodetype);
		ENSURE(new_child);
		new_child->_node.num = M - 1;///到右侧去
		//也可以循环复制，前提时value_type定义了拷贝构造函数
		//copy的逻辑与之差不多，但是对于plain_type更快一些
		std::copy(child->_node.key + M, child->_node.key + (M << 1) - 1, new_child->_node.key);
		if (nodetype == 0) {
			//中间节点还要拷贝ptr
			//注意指针多一个
			std::copy(child->_node.ptr, child->_node.ptr + (M << 1), new_child->_node.ptr);
		}
		//处理剩下的(前半段)一半，只需要更改一下已分配空间就可以了
		child->_node.num = M - 1;
		//对父节点的插入一个key和ptr
		ssize_t Pnum = parent->_node.num;//父节点已使用情况
		//std::copy_backward(parent->_node.ptr + pos + 1, parent->_node.ptr + Pnum, parent->_node.ptr + Pnum + 1);//ennmmmm
		std::copy_backward(parent->_node.ptr + pos + 1, parent->_node.ptr + Pnum+1, parent->_node.ptr + Pnum + 2);
		parent->_node.ptr[pos + 1] = new_child;//指向右节点
		//NODEBUG(print("前：%d",parent->_node.key[pos]));
		std::copy_backward(parent->_node.key + pos, parent->_node.key + Pnum, parent->_node.key + Pnum + 1);
		parent->_node.key[pos] = child->_node.key[M - 1];
		parent->_node.num += 1;
		NODEBUG(printf("%d\r\n", parent->_node.num));
		return 0;
	}

	//应该再额外实现一个insert_unique,,下面的版本是insert_equal
	template<typename pram>
	std::pair<typename Btree<pram>::node_pointer,bool> Btree<pram>::btree_insert_nofull(Btree::node_pointer node, Btree::value_type target) {
		//在未满的情况下，num<2*M-1
		//保证条件就是在寻找过程中，自动balance
		//这个insert操作是在已node为根时寻找插入点，然后insert
		if (node->_node.is_leaf == 1) {
			//已经到达叶子
			//从key中寻找，由于Btree自然有序，所以这里可以二分搜索
			//但是通过反向遍历，可以顺便copy_backward,,,
			ssize_t pos = node->_node.num;
			while (pos >= 1 && compare()(target, node->_node.key[pos - 1])) {
				node->_node.key[pos] = node->_node.key[pos - 1];
				--pos;
			}
			node->_node.key[pos] = target;
			node->_node.num += 1;
			btree_node_num += 1;

			//应该自己定义一个iterator
			//否则返回的都是一个node,而没有精确到key
			return std::make_pair(node, true);
		}
		else {
			//需要向下寻找
			ssize_t pos = node->_node.num;
			while (pos >= 1 && compare()(target, node->_node.key[pos - 1])) {
				--pos;
			}
			ssize_t length = (M << 1) - 1;
			if (length == node->_node.ptr[pos]->_node.num) {
				//需要分裂
				//此处
				ENSURE(0==btree_split_child(node, pos, node->_node.ptr[pos]));//enmm
				if (compare()(target, node->_node.key[pos]))
					;//新上升来一个节点
				else
					++pos;
			}
			return btree_insert_nofull(node->_node.ptr[pos], target);//递归
		}
	}

	//有问题，待改进
	template<typename pram>
	std::pair<typename Btree<pram>::node_pointer,bool> Btree<pram>::btree_insert(Btree::node_pointer node, Btree::value_type target) {
		//做一个检查，然后转调bree_insert_nofull函数
		//同时要维护header的相关属性
		/*又上一层保证
		if (node == nullptr) {
			return std::make_pair(node,false);
		}
		*/
		ssize_t length = (M << 1) - 1;
		if (length == node->_node.num) {
			//增加树高，需要分裂，产生一个parent,必然是internal 
			node_pointer newnode = btree_node_new(0);/////经过这一步之后，root才不是叶子
			//newnode->_node.num = 1;   //这是多余的
			header->_node.parent = newnode;
			newnode->_node.ptr[0] = node;
			btree_split_child(newnode, 0, node);//合并时更新num
			return btree_insert_nofull(newnode, target);
		}
		else {
			return btree_insert_nofull(node, target);
			//return std::make_pair(node,true);
		}
	}
	//insert的改进
	template<typename pram>
	std::pair<typename Btree<pram>::node_pointer, bool> Btree<pram>::insert_equal(Btree::value_type target) {
		//node_pointer y = header;
		node_pointer x = root();
		if (x == nullptr) {
			x = btree_node_new(1);//////修改第一个root应该是一个leaf
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

	//合并操作
	template<typename pram>
	void Btree<pram>::btree_merge_child(Btree::node_pointer node, Btree::size_type pos, Btree::node_pointer left, Btree::node_pointer right) {
		//正好是split_child的反向操作
		//用于在delete时进行自动balance
		//将noed的key[pos],和right合并到left中去
		ssize_t length = (M << 1) - 1;
		//修改容量
		left->_node.num = length;
		//拷贝key
		left->_node.key[M - 1] = node->_node.key[pos];
		std::copy(right->_node.key, right->_node.key + M - 1, left->_node.key + M);
		//修改ptr
		if (right->_node.is_leaf == 0) {
			std::copy(right->_node.ptr, right->_node.ptr + M, left->_node.ptr + M);
		}
		//根节点调整
		length = node->_node.num;
		std::copy(node->_node.ptr + pos + 2, node->_node.ptr + length + 1, node->_node.ptr + pos + 1);
		std::copy(node->_node.key + pos + 1, node->_node.key + length, node->_node.key + pos);
		node->_node.num -= 1;
		//释放空间
		_destroy(right);
	}

	//delete节点
	template<typename pram>
	typename Btree<pram>::node_pointer Btree<pram>::btree_delete(Btree::node_pointer node, Btree::value_type target) {
		//和合并一样的道理
		//排除一些特殊情况，然后调用接口
		//就是值得是涉事的三个节点都是临界值，M-1，1，M-1，此时无论删除哪一个必然要降低树的高度，释放掉根
		if (nullptr == node)
			return nullptr;
		if (node->_node.num == 1) {
			node_pointer left = node->_node.ptr[0];
			node_pointer right = node->_node.ptr[1];
			if (left && right && left->_node.num == M - 1 && right->_node.num == M - 1) {
				//全部合并之后为2*M-1个，再删除一个几点，不存在回溯的考虑了
				btree_merge_child(node, 0, left, right);
				_destroy(node);//上一步一定把node的num调整为0了
				btree_delete_nonone(left, target);
				return left;
			}
		}
		btree_delete_nonone(node, target);
		return node;
	}

	template<typename pram>
	void Btree<pram>::btree_delete_nonone(Btree::node_pointer node, Btree::value_type target) {
		//再保证node节点不会出错的情况下
		//至多调整一下node节点，且不会完全整个删除node节点
		if (1 == node->_node.is_leaf) {
			//针对叶子节点
			//和insert_equal不同，删除的元素不一定存在
			//必然要遍历，二分？线性？
			//可以把策略也封装一下
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
				//没有找到
				NODEBUG(printf("404!\r\n"));
			}
		}
		else {
			//针对中间节点
			//首先判断是否再node的key之中，策略就是寻找前赴后继节点替换一下即可
			ssize_t index = 0;
			ssize_t cap = node->_node.num;
			node_pointer left;
			node_pointer right;
			//以可考虑封装这个寻找策略
			//其实还应该根据compare是否具有判断==能力重新分发一下，要不然还要额外操作
			while (index < cap && compare()(node->_node.key[index], target))++index;
			if (index < cap && !compare()(target, node->_node.key[index])) {
				//find it
				//前赴
				left = node->_node.ptr[index];
				right = node->_node.ptr[index + 1];
				if (left->_node.num > M - 1) {
					//left足够多,移过来
					value_type prekey = btree_search_predecessor(left);
					node->_node.key[index] = prekey;
					btree_delete_nonone(left, prekey);
				}
				else if (right->_node.num > M - 1) {
					//right足够多
					value_type nextkey = btree_search_successor(right);
					node->_node.key[index] = nextkey;
					btree_delete_nonone(right, nextkey);
				}
				else {
					//此时无论删除那个，都会导致子节点不符合条件，需要合并
					btree_merge_child(node, index, left, right);
					btree_delete(left, target);
				}
			}
			else {
				//不在node中，需要望下去
				//此时key[index]>target,而ptr[index]指向的都<key[index]
				left = node->_node.ptr[index];
				//left是否足够删除，否则需要shift
				//两种shift方式，left-1 -》left   or   left+1 -》left
				
				if (left->_node.num == M - 1) {
					//确保没问题
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
						//无法shift时，合并，然后删除
                        node_pointer last = node->_node.ptr[index - 1];
						btree_merge_child(node, index - 1, last, left);
						left = last;
					}
					else {
						//从另一个方向合并
                        right = node->_node.ptr[index + 1];
						btree_merge_child(node, index, left, right);
					}
				}
				//确保没问题了,递归下去
				btree_delete_nonone(left, target);
				//}
				//else {
				//	//一开始就没问题
				//	btree_delete_nonone(left, target);
				//}
			}
		}
	}

	//前赴后继
	template<typename pram>
	typename Btree<pram>::value_type Btree<pram>::btree_search_predecessor(Btree::node_pointer node) {
		//找到node上面的最大值
		node_pointer most = node;
		while (most->_node.is_leaf == 0) {
			most = most->_node.ptr[most->_node.num];
		}
		return most->_node.key[most->_node.num - 1];
	}

	template<typename pram>
	typename Btree<pram>::value_type Btree<pram>::btree_search_successor(Btree::node_pointer node) {
		//找到node上面最小值
		node_pointer most = node;
		while (most->_node.is_leaf == 0) {
			most = most->_node.ptr[most->_node.num];
		}
		return most->_node.key[most->_node.num - 1];
	}

	//shift 函数
	//接一个点过来，注意所有的保证都由调用者完成（借完之后不会超限）
	template<typename pram>
	void Btree<pram>::btree_shift_to_left(Btree::node_pointer node, Btree::size_type pos, Btree::node_pointer left, Btree::node_pointer right) {
		//外部保证right的num>M-1
		//借的点可能是叶子，也可能是中间节点
		//需要分开判断
		//实际上是node的pos给了left,right的最小点给了node
		//先移动key
		ssize_t cap = right->_node.num;
		left->_node.num += 1;
		left->_node.key[left->_node.num - 1] = node->_node.key[pos];
		node->_node.key[pos] = right->_node.key[0];
		std::copy(right->_node.key + 1, right->_node.key + cap, right->_node.key);
		//然后移动ptr
		if (left->_node.is_leaf == 0) {
			left->_node.ptr[left->_node.num] = right->_node.ptr[0];
			std::copy(right->_node.ptr + 1, right->_node.ptr + (cap + 1), right->_node.ptr);
			//这是必要的吗？？
			right->_node.ptr[cap] = nullptr;
		}
		right->_node.num -= 1;
		NODEBUG(printf("%d", right->_node.num));
	}

	template<typename pram>
	void Btree<pram>::btree_shift_to_right(Btree::node_pointer node, Btree::size_type pos, Btree::node_pointer left, Btree::node_pointer right) {
		//另一个对称的shift
		//把left的最大->root,root->right
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

	//中序遍历，测试用
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

	//广度遍历，测试用
	template<typename pram>
	void Btree<pram>::btree_level_traversal(Btree::node_pointer node) {
		//借助于queue实现
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
		//等待扩展
		//通过内存映射把数据写入？
		;
	}

	//构造函数，转调接口即可
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

