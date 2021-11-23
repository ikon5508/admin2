admin: admin.c admin.h libmemory.c libmemory.h
	clang -g -o admin admin.c libmemory.c -lcrypto

ssl_server: ssl_server.c
	clang -g -o ssl_server ssl_server.c -lssl -lcrypto

hellow: hellow.c 
	clang -g -o hellow hellow.c -lssl -lcrypto
