all:target

TOPDIR=$(shell pwd)

CC=g++ 
CFLAG=-std=c++11 -I$(TOPDIR)/include -Wall -ggdb

#所有的H文件
HEADERS=$(wildcard *.h)
SOURCES=$(wildcard *.c *.cpp)

#所有的目标文件
#OBJECT=$(HEADERS:%.h=%.o)
OBJECT=$(patsubst %.c,%.o,$(patsubst %.cpp,%.o,$(SOURCES)))

#测试
test:
	@echo $(SOURCES)
	@echo $(HEADERS)
	@echo $(OBJECT)

target:$(OBJECT)
	@echo $(OBJECT)
	ar crs libutils.a $(OBJECT) && mv *.a *.o ../lib
	@echo "Done!" 


SPECIAL=$(CC) -D DEBUG_TEST_SINGTHREAD_ $(CFLAG) -c $^
##crawl.o:crawl.cpp
##	$(CC) $(CFLAG) -c $^
##	$(CC) $(CFLAG) -c $^ $(SPECIAL)

#通配规则
%.o:%.cpp
	$(CC) $(CFLAG) -c $^

%.o:%.c
	$(CC) $(CFLAG) -c S^

.PHONY:clean

clean:
	rm -f *.o
	@echo Clean Done!
