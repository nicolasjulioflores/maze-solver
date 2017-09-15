LIB = lib
LLIBS = lib/lib.a
LIBSTUFF = lib/common.c lib/common.h lib/amazing.h lib/file.c lib/file.h
PROGS = AmazingClient AMStartup
CFLAGS = -Wall -pedantic -std=c11 -ggdb
CC = gcc
MAKE = make
# for memory-leak tests
VALGRIND = valgrind --leak-check=full --show-leak-kinds=all
all: $(PROGS)
AMStartup: AMStartup.o $(LLIBS)
	$(CC) $(CFLAGS) -o $@ $^ 
AMStartup.o: AMStartup.c
	$(CC) $(CFLAGS) -c $^ -I$(LIB)
AmazingClient: AmazingClient.o $(LLIBS)
	$(CC) $(CFLAGS) -o $@ $^  
AmazingClient.o: AmazingClient.c 
	$(CC) $(CFLAGS) -c $^ -I$(LIB) 
$(LLIBS): $(LIBSTUFF)
	cd $(LIB); $(MAKE) 
clean:
	rm -rf *~ *.o *.dSYM
	rm -f $(PROGS)
	cd $(LIB); $(MAKE) clean
