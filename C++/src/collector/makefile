all:target

TOPDIR=$(shell pwd)
UTILSDIR= $(TOPDIR)/../utils

CC=g++ 
CFLAG=-std=c++11 -I$(TOPDIR)/include -I$(UTILSDIR) -Wall -ggdb

#所有的H文件
HEADERS=$(wildcard *.h)

#所有的目标文件
OBJECT=$(HEADERS:%.h=%.o)

#测试
SPECIAL=$(CC) -D DEBUG_TEST_SINGTHREAD_ $(CFLAG) -c $^
test:
	@echo $(HEADERS)
	@echo $(OBJECT)
	@echo $(UTILSDIR)
	@echo $(CFLAG)

target:$(OBJECT)
	@echo $(OBJECT)
	ar crs libcrawer.a $(OBJECT) && mv *.a *.o ../lib	
	@echo "Done!" 

crawl.o:crawl.cpp
	$(CC) $(CFLAG) -c $^
	#$(SPECIAL) -c $^
	
	


.PHONY:clean

clean:
	rm -f *.o
	@echo Clean Done!
