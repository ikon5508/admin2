all: admin

admin: admin.cpp admin.h
	clang++ -g -o admin admin.cpp -lcrypto

