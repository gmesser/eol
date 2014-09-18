clean :
	rm eol

build : eol

eol : eol.c makefile
	gcc -O3 -Wall -o eol eol.c
