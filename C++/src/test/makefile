all:test

TOPDIR=$(shell pwd)
UTILSDIR=../utils
OBJS:=$(wildcard ${UTILSDIR}/*.o)
COLLECTDIR=../collector
OBJS+=$(wildcard ${COLLECTDIR}/*.o)
CC=g++ 
FLAG=-std=c++11 -Wall -I$(UTILSDIR) -I$(COLLECTDIR)
DEBUG=-O0 -ggdb
LINKFLAG=-lz -lpthread

test:
	@echo "test dirs"
	@echo $(UTILSDIR)
	@echo $(COLLECTDIR)
	@echo $(OBJS)

strfun_test: strfun.cpp
	$(CC) $(FLAG) -o strfun_test.out strfun_test.cpp ../utils/str_fun.o
	
page_test:page_test.cpp 	
	$(CC) $(FLAG) $(DEBUG) page_test.cpp  -o page_test.out  ../utils/url.o ../utils/errlog.o ../utils/md5.o ../utils/page.o ../utils/strfun.o
	
crawl_test: crawl_test.cpp $(OBJS)
	$(CC) $(FLAG) -o crawl_test.out crawl_test.cpp $(OBJS) $(DEBUG) $(LINKFLAG) 

.PHONY:clean

clean:
	rm -i *.o *.out || rm ../doc/link* ../doc/format* ../doc/un* ../doc/visited*
	@echo Done!
