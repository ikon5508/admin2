#include "shared.h"
#include <dirent.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netinet/tcp.h>
#include <string.h>
#include <time.h>
#include <netdb.h>
#include <signal.h>
#include <openssl/sha.h>

#include <poll.h>


#define sendfileunit 10000000
#define maxbuffer 100000
#define nameholder 100
#define string_sz 2048
//#define entry 500



struct string_data
{
char p[string_sz];
int procint;
int len;
};
//typedef struct string_data string;

extern const int timeout;



const char *closehead = "HTTP/1.1 200 OK\nContent-Type: text/html\nConnection: close\nContent-Length: ";

//const char *hello = "HTTP/1.1 200 OK\nContent-Type: text/plain\nContent-Length: 0000012\n\nHello world!";

const char *hthead = "HTTP/1.1 200 OK\n";

const char *conticon = "Content-Type: image/x-icon\n";
const char *contjava = "Content-Type: text/javascript\n";
const char *conthtml = "Content-Type: text/html; charset=utf-8\n";
const char *conttxt = "Content-Type: text/plain\n";
const char *contjpg = "Content-Type: image/jpg\n";
const char *contpng = "Content-Type: image/png\n";
const char *contcss = "Content-Type: text/css\n";
const char *contpdf = "Content-Type: application/pdf\n";
const char *contmp4 = "Content-Type: video/mp4\n";

const char *contoctet = "Content-Type: application/octet-stream\n";

const char *connclose = "Connection: close\n";
const char *connka = "Connection: timeout=5, max=10\n";
const char *contlen = "Content-Length: ";



struct settings_data {
int port;
//int ssl;
int showaction; 

// 0 for  non, 1 for show action page 2 for preview file
char base_path [string_sz];
char internal [string_sz];
char editor [string_sz];

    
}settings = {9999, 1, ".", "internal", "aceeditor.htm"};

enum emode
{root, err, file, edit, action, upload, config, favicon, websock, postdump};

enum rtype
{none, reg, dir, altreg, altdir};

struct request_data
{
char url [string_sz];
char url_params [string_sz];
const char *mode_text;
char path [string_sz];
char full_path [string_sz];

char filename [default_sz];
char ext [default_sz];
unsigned long content_len;

const struct buffer_data *mainbuff;
char method;
int fd;
enum emode mode;
enum rtype type;
};

struct post_file_data {
char boundary [default_sz];
int boundary_len;
unsigned long fsize;
unsigned long content_prog;
char fname [default_sz];
int stat_fname;
int loopint;

int fd;
int offset;

}; // post file data



int get_fname (struct post_file_data *filedata, const buffer_t inbuff);

int get_boundary (struct post_file_data *filedata, const buffer_t inbuff); 

void softclose (const int fd, struct buffer_data *inbuff);

int getnext (const char *base, const int c, const int offset, const int len);



int sock_buffwrite (const int connfd, struct buffer_data *out);
int prepsocket (const int PORT);
int sock_setnonblock (const int fd);
int sock_writeold (const int connfd, const char *buffer, const int size);
int sock_read (const int connfd, void *buffer, int size);
int sock_write (const int connfd, char *out, const int len);

int send_file (const char *path, const int fd);

void save_buffer (const struct buffer_data b, const char *path);


void safe_fname (const struct request_data request, const char *fname, char *rtn);
int servico (const int fd);
void softclose (const int fd, struct buffer_data *inbuff);
int send_err (const int fd, const int code);
int send_txt (const int fd, const char *txt);
int send_ftxt (const int fd, const char *format, ...);
int serv_dir (const struct request_data request);
int serv_file (const struct request_data request);
int get_file (const struct settings_data settings, const struct request_data request);

int process_request (struct request_data *request, const int fd, const struct buffer_data inbuff);

int post_edit (const struct request_data request);
int get_edit (const struct request_data request);
int get_config (const struct settings_data settings, const struct request_data request);
//int post_file (const struct request_data request);




