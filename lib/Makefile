# Makefile for the library 
# 
# Negoska 		March 2017

OBJS = common.o file.o
LIB = lib.a

CFLAGS = -Wall -pedantic -std=c11 -ggdb
CC = gcc 
MAKE = make

$(LIB): $(OBJS)
	ar cr $@ $^

common.o: common.c common.h file.h
	$(CC) $(CFLAGS) -c common.c

file.o: file.c file.h
	$(CC) $(CFLAGS) -c file.c

clean:
	rm -f *.o
	rm -f lib.a
	rm -f *~
