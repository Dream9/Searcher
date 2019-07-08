all:test

TOPDIR=$(shell pwd)

UTILSDIR=../utils
OBJS:=$(wildcard ${UTILSDIR}/*.o)

COLLECTDIR=../collector
OBJS+=$(wildcard ${COLLECTDIR}/*.o)

INDEXDIR=../index
OBJS_INDEX=$(wildcard $(INDEXDIR)/*.o)
OBJS_INDEX+=../utils/errlog.o ../utils/md5.o ../utils/strfun.o
OBJS_PAGERANK=$(wildcard $(INDEXDIR)/pagerank/*.o)
OBJS_PAGERANK+=../utils/errlog.o
OBJS_PAGESORT=$(wildcard $(INDEXDIR)/pagerank/*.o)
OBJS_PAGESORT+=../utils/errlog.o ../utils/md5.o ../utils/page.o ../utils/strfun.o ../utils/url.o

QUERYDIR=../query
OBJS_CGI=$(wildcard $(QUERYDIR)/*.o)
OBJS_CGI+=../utils/strfun.o ../index/dictinary.o ../index/word_segement.o ../utils/errlog.o ../utils/rio.o ../index/document.o


CC=g++ 
FLAG=-std=c++11 -Wall -I$(UTILSDIR) -I$(COLLECTDIR) -I$(INDEXDIR)
DEBUG=-O0 -ggdb
LINKFLAG=-lz -lpthread

test:
	@echo "test dirs"
	@echo $(UTILSDIR)
	@echo $(COLLECTDIR)
	@echo $(OBJS)

#测试字符处理函数
strfun_test: strfun.cpp
	$(CC) $(FLAG) -o strfun_test.out strfun_test.cpp ../utils/str_fun.o

#测试网页文档分析
page_test:page_test.cpp 	
	$(CC) $(FLAG) $(DEBUG) page_test.cpp  -o page_test.out  ../utils/url.o ../utils/errlog.o ../utils/md5.o ../utils/page.o ../utils/strfun.o

#测试数据采集
crawl_test: crawl_test.cpp $(OBJS)
	$(CC) $(FLAG) -o crawl_test.out crawl_test.cpp $(OBJS) $(DEBUG) $(LINKFLAG) 

#测试分词模块
word_segement_test: word_segement_test.cpp
	$(CC) $(FLAG)  -I$(INDEXDIR)/include -o word_segement_test.out ../index/dictinary.o ../index/word_segement.o ../utils/errlog.o word_segement_test.cpp $(DEBUG)
	
#测试索引
index_test:index_test.cpp
	$(CC) $(FLAG) -I$(INDEXDIR)/include -o index_test.out $< $(OBJS_INDEX) $(DEBUG)

#测试cgi程序
cgi_test:cgi_test.cpp
	$(CC) $(FLAG) -I$(QUERYDIR) -o cgi_test.out $< $(OBJS_CGI) $(DEBUG)  

#测试cgi环境
env_cgi:env_cgi_test.cpp
	$(CC) $(FLAG) -I$(QUERYDIR) -o env_cgi_test.cgi $< $(OBJS_CGI) $(DEBUG)  

#测试搜索服务
search_cgi:search_cgi.cpp
	$(CC) $(FLAG) -I$(QUERYDIR) -o search.cgi $< $(OBJS_CGI) $(DEBUG)  

#测试pagerank
pagerank:pagerank_test.cpp
	$(CC) $(FLAG) -I$(INDEXDIR)/pagerank -o pagerank_test.out $< $(OBJS_PAGERANK) $(DEBUG)

#测试网页排名
pagesort:pagesort_test.cpp
	$(CC) $(FLAG) -I $(INDEXDIR)/pagerank -o pagesort_test.out $< $(OBJS_PAGESORT) $(DEBUG)

.PHONY:clean

clean:
	rm -i *.o *.out || rm ../doc/link* ../doc/format* ../doc/un* ../doc/visited*
	@echo Done!
