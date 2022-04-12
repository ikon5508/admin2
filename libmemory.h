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
#include <time.h>
#include <netdb.h>


#define sendfileunit 100000
#define maxbuffer 100000
#define nameholder 100
#define string_sz 1000
//#define entry 500

struct buffer_data
{
char *p;
int procint;
int len;
int max;
};
typedef struct buffer_data buffer;

struct ubuffer_data
{
unsigned char *p;
int procint;
int len;
int max;
};
typedef struct ubuffer_data ubuffer;

struct string_data
{
char p[string_sz];
int procint;
int len;
};
typedef struct string_data string;
void softclose (const int fd, struct buffer_data *inbuff);

void send_kill (const int port);
int init_buffer (struct buffer_data *buffer, const int size);
int getnext (const char *base, const int c, const int offset, const int len);
int midstr(const char *major, char *minor, int start, const int end);

int ugetnext (const unsigned char *base, const int c, const int offset, const int len);
int umidstr(const unsigned char *major, char *minor, int start, const int end);
int getlast (const char *str, const int c, int len);

int search (const char *main, const char *minor, const int start, const int end);

int usearch (const unsigned char *main, const char *minor, const int start, const int end);
int sock_buffwrite (const int connfd, struct buffer_data *out);
void buffcatf (struct buffer_data *buff, const char *format, ...);
int prepsocket (const int PORT);
int sock_setnonblock (const int fd);
int sock_writeold (const int connfd, const char *buffer, const int size);
int sock_read (const int connfd, char *buffer, int size);
int sock_write (const int connfd, char *out, const int len);

int usock_read (const int connfd, unsigned char *buffer, int size);
int sendfileM (const char *path, const int fd);
int send_file (const char *path, const int fd);
int sendfileold (const char *path, const int fd);
int init_sockbackdoor (const int type, const char *init);
int init_log (const char *path);
int loggingf (const char *format, ...);
void close_log ();

int countassets (const struct buffer_data buff);

int strsearch (const char *hay, const char *needle, const int offset, const int haylen); 

int buffsearch (const struct buffer_data hay, const char *needle, const int offset, const int roffset);

int ftrim (char *buff);
int rtrim (char *buff);

void save_buffer (const struct buffer_data b, const char *path);
extern const int timeout;
extern int backdoor;

