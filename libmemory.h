#include <stdio.h>
 #include <stdlib.h>
#include <string.h>
#include <unistd.h>

#ifndef buffsz
#define buffsz
#define maxbuffer 100000

#define string_sz 1000

#endif


#ifndef libfunc
#define libfunc 2

struct buffer_data
{
char *p;
int procint;
int len;
int max;
};

struct string_data
{
char p[string_sz];
int procint;
int len;
};

int init_buffer (struct buffer_data *buffer, const int size);
int getnext (const char *str, const char next, const int start, const int end);
int midstr(const char *major, char *minor, int start, const int end);
int getlast (const char *str, const char next, const int end);
//const int maxbuffer = 100000;
int search (const char *main, const char *minor, int start, int end);
#endif


#ifndef delim_char
#define delim_char
#define delimchar 21
#endif

