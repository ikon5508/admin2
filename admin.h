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
struct string_data base_path;
struct string_data editor_path;
};

struct request_data
{
struct string_data uri;
struct string_data path;
struct string_data fullpath;

char method;
int keepalive;
int fd;
int procint;
char boundary [100];
int boundlen;

enum emode
{err, action, file, edit, upload, config, root, favicon} mode;
};


void softclose (const int fd, struct buffer_data *inbuff);

int send_err (const int fd, const int code);

int send_txt (const int fd, const char *txt, int len);

int serv_dir (const struct args_data args, const struct request_data request);

