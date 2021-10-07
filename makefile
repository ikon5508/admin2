admin: admin.c admin.h libmemory.c libmemory.h socket.c socket.h logging.h logging.c
	cc -g -o admin admin.c libmemory.c socket.c logging.c
	
test: admin.c admin.h libmemory.c libmemory.h socket.c socket.h
	cc -o test admin.c libmemory.c socket.c
