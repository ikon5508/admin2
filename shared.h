#define _GNU_SOURCE
#ifndef shared_h
#define shared_h 
#include <errno.h>

#include <fcntl.h>
#include <time.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>

#define default_sz 128
#define smbuff_sz 2048
#define mdbuff_sz 10000
#define lgbuff_sz 100000

struct codeco {
int pos;
char dchar;
};

struct _stopwatch
{
struct timespec start, end;
int sec, msec;
};

typedef struct _stopwatch stopwatch_t;

struct buffer_data
{
char *p;
int procint;
int len;
int max;
};
typedef struct buffer_data buffer_t;

int extract_SC (const buffer_t *src, char *ex, const int exmax, const char *d1, const char d2);
int FAR (buffer_t *base, const char *delim, const buffer_t rep);
int extract_CC (const buffer_t src, char *ex, const int exmax, const char d1, const char d2);
struct buffer_data init_buffer (const int sz);
void buffcatf (struct buffer_data *buff, const char *format, ...);

void build_template (buffer_t *buff, const int ents, ...);

//void stopwatch_start (stopwatch_t *t);
//void stopwatch_stop (stopwatch_t *t);
//void result (int *sec, int *ms);


int strsearch (const char *main, const char *minor, const int start);
int midstr(const char *major, char *minor, int start, const int end);
int getlast (const char *str, const int c, int len);
int getnext (const char *base, const int c, const int offset, const int len);

void err_ctl (const int rslt, const char *msg);
void killme (const char *msg);
char *parse_line (char *dest, const char *src);
int split_value (char *, const char, char *);
int trim (char *totrim);

int utrim (unsigned char *totrim);

buffer_t JSON_decode (const buffer_t in);
void buffer_sanity (buffer_t *buff, const int req, const int inc);
buffer_t HTML_encode (const buffer_t in, const int level);

void save_buffer (const struct buffer_data b, const char *path);
int URL_decode (const char *in, char *out);
//typedef struct blueprint blueprint_t;

#endif
