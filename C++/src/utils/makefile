all:target

TOPDIR=$(shell pwd)

CC=g++ 
CFLAG=-std=c++11 -I$(TOPDIR)/include -Wall -ggdb

#所有的H文件
HEADERS=$(wildcard *.h)

#所有的目标文件
OBJECT=$(HEADERS:%.h=%.o)

#测试
test:
	echo $(HEADERS)
	echo $(OBJECT)

target:$(OBJECT)
	@echo $(OBJECT)
	@echo "Done!" 

strfun.o:strfun.cpp
	$(CC) $(CFLAG) -c $^

md5.o:md5.cpp
	$(CC) $(CFLAG) -c $^

url.o:url.cpp
	$(CC) $(CFLAG) -c $^

page.o:page.cpp
	$(CC) $(CFLAG) -c $^

errlog.o:errlog.cpp
	$(CC) $(CFLAG) -c $^

crawl.o:crawl.cpp
	$(CC) $(CFLAG) -c $^
	
SPECIAL=$(CC) -D DEBUG_TEST_SINGTHREAD_ $(CFLAG) -c $^

data_engine.o:data_engine.cpp
	$(CC) $(CFLAG) -c $^

file_engine.o:file_engine.cpp
	$(CC) $(CFLAG) -c $^

formatfile.o:formatfile.cpp
	$(CC) $(CFLAG) -c $^

http.o:http.cpp
	$(CC) $(CFLAG) -c $^

isamfile.o:isamfile.cpp
	$(CC) $(CFLAG) -c $^

link4sefile.o:link4sefile.cpp
	$(CC) $(CFLAG) -c $^

rio.o:rio.cpp
	$(CC) $(CFLAG) -c $^

.PHONY:clean

clean:
	rm -f *.o
	@echo Clean Done!
