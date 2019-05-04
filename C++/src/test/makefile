all:target

MYPATH=../utils
OBJS=$(wildcard ${MYPATH}/*.o)
CC=g++ -std=c++11 -Wall -I$(MYPATH)
DEBUG=-O0 -ggdb

target:
	@echo "Todo"

strfun_test: 
	$(CC) -o strfun_test.out strfun_test.cpp
	
page_test: $(OBJS)
	@echo $(OBJS)
	$(CC) $(DEBUG)  $^ page_test.cpp  -o page_test.out

.PHONY:clean

clean:
	rm -i strfun_test.out page_test.out
