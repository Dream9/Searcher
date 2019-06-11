ALL:target

TOPDIR=$(shell pwd)
INCLUDEDIR=./include
UTILSDIR=../utils

CC=gcc
XX=g++
DEBUG=-ggdb -O0
FLAG=-std=c++11 -Wall -I$(INCLUDEDIR) -I$(UTILSDIR) $(DEBUG)


%.o:%.cpp
	$(XX) $(FLAG) -c $< -o $@

%.o:%.c
	$(XX) $(FLAG) -c $< -o $@

SOURCES=$(wildcard *.c *.cpp)
OBJS=$(patsubst %.c,%.o,$(patsubst %.cpp,%.o,$(SOURCES)))

test:
	@echo $(OBJS)
	@echo $(SOURCES)
	@echo $(INCLUDEIDR)

target:$(OBJS)
	@echo $(OBJS)
	@echo "Done!"

.PHONY:clean

clean:
	rm -f *.o
	@echo Clean!
