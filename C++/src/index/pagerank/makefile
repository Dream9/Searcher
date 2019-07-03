ALL:target

UTILSDIR=../../utils
CC=gcc
XX=g++
DEBUG=-ggdb -O0
FLAG=-std=c++11 -Wall -I$(UTILSDIR)

%.o:%.cpp
	$(XX) $(FLAG) -c $< -o $@

%..o:%.c
	$(XX) $(FLAG) -c $< -o $@

SOURCES=$(wildcard *.c *.cpp)
OBJS=$(patsubst %.c,%.o,$(patsubst %.cpp,%.o,$(SOURCES)))

target:$(OBJS)
	@echo $(OBJS)
	@echo

.PHONY:clean

clean:
	@rm -f *.o *.out
	@echo done
