admin: admin.c admin.h libmemory.c libmemory.h
	clang -g -o admin admin.c libmemory.c

ssladmin: ssladmin.c admin.h libmemory.c libmemory.h
	clang -g -o ssladmin ssladmin.c libmemory.c -lssl -lcrypto
