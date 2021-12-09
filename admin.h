#include "libmemory.h"
#include <dirent.h>
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

const char *contmp4 = "Content-Type: video/mp4\n";

const char *connclose = "Connection: close\n";
const char *connka = "Connection: timeout=5, max=10\n";
const char *contlen = "Content-Length: ";



struct args_data {
int port;
int ssl;
int showaction; 
// 0 for  non, 1 for show action page 2 for preview file
char base_path [string_sz];
char editor_path [string_sz];

char dedicated_ip [INET_ADDRSTRLEN];    
    
};

struct request_data
{
char uri [string_sz];
char path [string_sz];
char fullpath [string_sz];

char resourcename [200];
char ext [20];

const struct buffer_data *mainbuff;
char method;
int fd;

int procint;
int content_len;
char *procpnt;

char user_agent [200];
char code [100];
int codelen;

enum emode
{err, action, file, edit, upload, config, root, favicon, websock, postdump} mode;
};

void safe_fname (const struct request_data request, const char *fname, char *rtn);
int servico (const int fd);
void softclose (const int fd, struct buffer_data *inbuff);
int send_err (const int fd, const int code);
int send_txt (const int fd, const char *txt);
int send_ftxt (const int fd, const char *format, ...);
int serv_dir (const struct args_data args, const struct request_data request);
int serv_file (const struct args_data args, const struct request_data request, const int size);
int get_file (const struct args_data args, const struct request_data request);

struct request_data process_request (const int fd, const struct args_data args, const struct buffer_data inbuff);

int put_edit (const struct request_data request);
int get_edit_file (const struct args_data args, const struct request_data request);
int get_config (const struct args_data args, const struct request_data request);
int put_file (const struct request_data request);












