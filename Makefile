CC=gcc
CFLAGS=-g -std=c11 -O0
TARGET=program
OBJS=hashmap.c qalloc.c
.PHONY: clean
all: $(TARGET)

test: maptest.o instantiate.c hashmap.c qalloc.c
	$(CC) $(CFLAGS) $^ -o $@ -lpthread -lcmocka

test1: test.c hashmap.c qalloc.c
	$(CC) $(CFLAGS) $^ -o $@ -lpthread -lcmocka
performance: performance.c hashmap.c qalloc.c
	$(CC) $(CFLAGS) $^ -o $@ -lpthread -ltcmalloc #-ltcmalloc_minimal
	#$(CC) $(CFLAGS) $^ -o $@ -lpthread 



clean:
	rm -f *.o
	rm -f test
