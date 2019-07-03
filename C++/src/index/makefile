ALL:target

TOPDIR=$(shell pwd)
SUBDIR=pagerank
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
	@echo $(SUBDIR)

target:$(OBJS) $(SUBDIR)
	@echo $(OBJS)
	@echo "Done!"

$(SUBDIR): ECHO
	$(MAKE) -C $@
	
ECHO:
	@echo $(SUBDIR)

.PHONY:clean

clean:
	@for dir in $(SUBDIR);\
		do $(MAKE) -C $$dir clean || exit 1;\
		done
	rm -f *.o
	@echo Clean!
