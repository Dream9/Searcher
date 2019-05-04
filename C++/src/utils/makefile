all:target

CC=g++ -std=c++11 -Wall

target:strfun.o
	$(CC) -E 

strfun:strfun.cpp
	$(CC) -c $<

md5:md5.cpp
	$(CC) -c $<

.PHONY:clean

clean:
	rm -i strfun.o md5.o
