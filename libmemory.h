#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netinet/tcp.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <time.h>

#define maxbuffer 100000

#define string_sz 1500

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

struct search_data
{
int rtn;
int offset;
};

int init_buffer (struct buffer_data *buffer, const int size);
int getnext (const char *base, const int c, const int offset, const int len);
int midstr(const char *major, char *minor, int start, const int end);
int getlast (const char *str, const int c);
int search (const char *main, const char *minor, const int start, const int end);
int sock_buffwrite (const int connfd, struct buffer_data *out);
int buffcatf (struct buffer_data *buff, const char *format, ...);
int prepsocket (const int PORT);
int sock_setnonblock (const int fd);
int sock_writeold (const int connfd, const char *buffer, const int size);
int sock_read (const int connfd, char *buffer, int size);
int sock_write (const int connfd, char *out, const int len);
int sendfile (const char *path, const int fd);
int init_sockbackdoor (const char *init);
int init_log (const char *path);
int loggingf (const char *format, ...);
void close_log ();


int strsearch (const char *hay, const char *needle, const int offset, const int haylen); 

int buffsearch (const struct buffer_data hay, const char *needle, const int offset, const int roffset);

extern const int timeout;
