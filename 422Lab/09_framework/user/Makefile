CC = arm-linux-gnueabihf-gcc
INCLUDE_DIR=$(PWD)/../include
CFLAGS =-I$(INCLUDE_DIR) -Wall

all: test monitor dense_mm fibonacci

clean:
	rm -f test monitor dense_mm fibonacci

test: test.c
	$(CC) $(CFLAGS) test.c -o test

monitor: monitor.c
	$(CC) $(CFLAGS) monitor.c -o monitor

dense_mm: dense_mm.c
	$(CC) $(CFLAGS) -fopenmp dense_mm.c -o dense_mm

fibonacci: fibonacci.c
	$(CC) fibonacci.c -o fibonacci
