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

SOURCE_FILES=$(wildcard *.c *.cpp)
OBJECTS=$(patsubst %.c,%.o,$(patsubst %.cpp,%.o,$(SOURCE_FILES)))

test:
	@echo $(OBJECTS) $(SOURCE_FILES) $(INCLUDEDIR)

target:$(OBJECTS)
	@echo $(OBJECTS)
	ar crs libquery.a $(OBJECTS) && mv *.a *.o ../lib
	@echo Done

.PYHOY:clean

clean:
	rm -f *.o
	@echo Clean!
