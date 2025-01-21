CC = gcc
CFLAGS = -g -Wall -lm

build: quadtree

quadtree: quadtree.c
	$(CC) -o quadtree quadtree.c $(CFLAGS)


.PHONY : clean
clean :
	rm -f quadtree
