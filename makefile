admin: admin.c admin.h libmemory.c libmemory.h
	clang -g -o admin admin.c libmemory.c

test: admin.c admin.h libmemory.c libmemory.h
	cc -g -o test admin.c libmemory.c
