all: admin test

shared.o: shared.c shared.h
	clang -g -c shared.c

test: diag.c shared.o
	clang -g -o test diag.c shared.o

admin: admin.c admin.h shared.o
	clang -g -o admin admin.c shared.o -lcrypto

