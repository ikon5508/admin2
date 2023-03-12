#define _GNU_SOURCE
#include <fcntl.h>
#include <time.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
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
#include <signal.h>
#include <pthread.h>
#include <stdbool.h>
#include <openssl/ssl.h>
#include <openssl/err.h>

#define sendfileunit 10000000 
#define maxbuffer 100000
#define nameholder 100
#define string_sz 2048
#define default_sz 128
#define smbuff_sz 2048
#define mdbuff_sz 10000
#define lgbuff_sz 100000

#define upload_mode "/upload"
#define edit_mode "/edit"
#define action_mode "/action"
#define ace_builds_mode "/ace-builds"
#define config_mode "/config"

const int upload_mode_len = strlen (upload_mode);
const int action_mode_len = strlen (action_mode);
const int edit_mode_len = strlen (edit_mode);
const int ace_builds_mode_len = strlen (ace_builds_mode);
const int config_mode_len = strlen (config_mode);

#define THREAD_POOL_SIZE 5
pthread_t thread_pool[THREAD_POOL_SIZE];
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t condition_var = PTHREAD_COND_INITIALIZER;

struct far_entry {
int delim_len;
int rep_len;
const char *delim;
const char *rep;
int delim_pos;
struct far_entry *next;
};

struct far_builder {
char *base;
int base_len;
int req_len;

struct far_entry *start;
};

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

struct sthread_db {
int id;
}thread_db [THREAD_POOL_SIZE];

struct trig {
bool use;
int connfd;
}trigger = {false, 0};

struct string_data
{
char p[string_sz];
int procint;
int len;
};
//typedef struct string_data string;

struct io_control {
SSL *ssl;
int connfd;
};
typedef struct io_control io_t;

struct settings_data {
int port;
bool tls;
int showaction; 
char base_path [string_sz];
char ace_builds [string_sz];
char editor [string_sz];
}settings = {9999, true, 1, ".", ".", "aceeditor.htm"};

enum emode
{root, err, file, edit, action, upload, config, favicon, websock, ace_builds};

enum rtype
{none, reg, dir};

struct request_data
{
char url [string_sz];
char params [string_sz];
const char *mode_text;
char path [string_sz];
char full_path [string_sz];
char filename [default_sz];
char ext [default_sz];
unsigned long content_len;

const char *mime_txt;
io_t io;
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
const char  BOOKMARK [] = {'/', '/', ' ', 'b', 'm', ' '};

SSL_CTX *ctx;
const int timeout = 3;

int extract_SC (const buffer_t *src, char *ex, const int exmax, const char *d1, const char d2);
int FAR (buffer_t *base, const char *delim, const buffer_t rep);
int extract_CC (const buffer_t src, char *ex, const int exmax, const char d1, const char d2);
struct buffer_data init_buffer (const int sz);
void buffcatf (struct buffer_data *buff, const char *format, ...);
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
buffer_t JSON_decode (const buffer_t in);
void buffer_sanity (buffer_t *buff, const int req, const int inc);
buffer_t HTML_encode (const buffer_t in, const int level);
void save_buffer (const struct buffer_data b, const char *path);
int URL_decode (const char *in, char *out);
//typedef struct blueprint blueprint_t;
struct far_builder far_init (const buffer_t *in);
int far_add (struct far_builder *b, const char *delim, const char *rep, int rep_len);
buffer_t far_build (struct far_builder *b);
void far_clear (struct far_builder *b);
int get_fname (struct post_file_data *filedata, const buffer_t inbuff);
int get_boundary (struct post_file_data *filedata, const buffer_t inbuff); 
void softclose (const int fd, struct buffer_data *inbuff);
int getnext (const char *base, const int c, const int offset, const int len);
int prepsocket (const int PORT);
int sock_setnonblock (const int fd);
int send_file2 (const char *path, const int fd);
//int send_file (const io_t io, const char *path);
void save_buffer (const struct buffer_data b, const char *path);
void safe_fname (const struct request_data request, const char *fname, char *rtn);
int servico (const io_t);
void softclose (const int fd, struct buffer_data *inbuff);
int send_err (const int fd, const int code);
int send_txt2 (const int fd, const char *txt);
int send_ftxt (const int fd, const char *format, ...);
int serv_dir (const struct request_data request);
int serv_file (const struct request_data request);
int get_file (const struct settings_data settings, const struct request_data request);
int parse_request (struct request_data *request, const int fd, const struct buffer_data inbuff);
int post_edit (const struct request_data request);
int get_edit (const struct request_data request);
int get_config (const struct settings_data settings, const struct request_data request);
//int post_file (const struct request_data request);
int send_txt (const io_t io, const char *txt);

// old io funcs
int sock_1writeold (const int connfd, const char *buffer, const int size);
int sock_buffwrite (const int connfd, struct buffer_data *out);
int sock_readold (const int connfd, void *buffer, int size);
int sock_writeold (const int connfd, char *out, const int len);

int sock_write1 (const io_t, const char *, const int);
int sock_write (const io_t, const char *, const int);
int sock_read (const io_t, char *, const int);
int sock_read1 (const io_t, char *, const int);
int sock_send_file (const io_t, const char *);

// bm func def
int tls_read1 (const io_t, char *, const int);
int tls_read (const io_t, char *, const int);
int tls_write (const io_t, const char *, const int);
int tls_write1 (const io_t, const char *, const int);
int tls_send_file (const io_t, const char *);

int (*io_read1) (const io_t, char *, const int) = tls_read1;
int (*io_read) (const io_t, char *, const int) = tls_read;
int (*io_write) (const io_t, const char *, const int) = tls_write;
int (*io_write1) (const io_t, const char *, const int) = tls_write1;
int (*send_file) (const io_t, const char *) = tls_send_file;


SSL_CTX *create_context()
{
    const SSL_METHOD *method;
    SSL_CTX *ctx;

    method = TLS_server_method();

    ctx = SSL_CTX_new(method);
    if (!ctx) {
        perror("Unable to create SSL context");
        ERR_print_errors_fp(stderr);
        exit(EXIT_FAILURE);
    }

    return ctx;
}

void configure_context(SSL_CTX *ctx)
{
    /* Set the key and cert */
    if (SSL_CTX_use_certificate_file(ctx, "server.crt", SSL_FILETYPE_PEM) <= 0) {
        ERR_print_errors_fp(stderr);
        exit(EXIT_FAILURE);
    }

    if (SSL_CTX_use_PrivateKey_file(ctx, "server.key", SSL_FILETYPE_PEM) <= 0 ) {
        ERR_print_errors_fp(stderr);
        exit(EXIT_FAILURE);
    }
}

int get_action (const struct request_data request)
{
// bm getaction    
    char outb [maxbuffer];
struct buffer_data out;
out.p = outb;
out.len = 0;
out.max = maxbuffer;

int showedit = 0;
enum eviewtype
{none, txt, img} viewtype;

if (strcmp(request.ext, ".txt") == 0)
		viewtype = txt;
if (strcmp(request.ext, ".c") == 0)
		viewtype = txt;

if (strcmp(request.ext, ".h") == 0)
		viewtype = txt;

if (strcmp(request.ext, ".cpp") == 0)
		viewtype = txt;

if (strcmp(request.ext, ".hpp") == 0)
		viewtype = txt;

if (strcmp(request.ext, ".htm") == 0)
		viewtype = none;

if (strcmp(request.ext, ".html") == 0)
		viewtype = none;

if (strcmp(request.ext, ".jpg") == 0)
		viewtype = img;

if (strcmp(request.ext, ".jpeg") == 0)
		viewtype = img;

if (strcmp(request.ext, ".png") == 0)
		viewtype = img;

if (viewtype == txt)
	showedit = 1;

buffcatf (&out, "<!DOCTYPE html>\n<html>\n<head>\n");

buffcatf (&out,"<style>\n");
buffcatf (&out,"body\n{\ntext-align:left;\nmargin-left:70px;\nbackground-color:aqua;\nfont-size:24px;\n}\n");
buffcatf (&out, "a:link\n{\ncolor:midnightblue;\ntext-decoration:none;\n}\n");

buffcatf (&out, ".button {\npadding-left:20px;\npadding-right:20px;\nfont-size:18px;\n}");


buffcatf (&out, "</style>\n</head>\n<body>\n");

buffcatf (&out, "<script>\n");
buffcatf (&out, "let fname = \"%s\";\n", request.filename);

buffcatf (&out, "function postdata (action, data) {\n");

//var a = editor.getValue(); // or session.getValue
 
buffcatf (&out, "form = document.createElement(\'form\');\n");
buffcatf (&out, "form.setAttribute(\'method\', \'POST\');\n");
buffcatf (&out, "form.setAttribute(\'enctype\', \'multipart/form-data\');\n");
buffcatf (&out, "form.setAttribute(\'action\', window.location);\n");

buffcatf (&out, "myvar = document.createElement(\'input\');\n");
buffcatf (&out, "myvar.setAttribute(\'name\', action);\n");
buffcatf (&out, "myvar.setAttribute(\'type\', \'hidden\');\n");
buffcatf (&out, "myvar.setAttribute(\'value\', data);\n");


buffcatf (&out, "form.appendChild(myvar);\n");
buffcatf (&out, "document.body.appendChild(form);\n");
buffcatf (&out, "form.submit();\n}\n");  
// end function

buffcatf (&out, "function frename () {\n");
//buffcatf (&out, "window.alert (action);\n}\n");
buffcatf(&out, "var newfname = prompt (\"New File Name:\", \"%s\");\n", request.filename);

buffcatf (&out, "if (newfname == null) return;\n");


buffcatf (&out, "window.alert (newfname); \npostdata (\'rename\', newfname);\n}\n");

buffcatf (&out, "</script>\n");

buffcatf (&out, "%s<br>\n", request.filename);

buffcatf (&out, "<a href=\"%s\">View File</a><br>\n", request.path);
if (showedit)
	buffcatf (&out, "<a href=\"/edit%s\">Edit File</a><br>\n", request.path);

buffcatf (&out, "<input type=\"button\" class=\"button\" value=\"Rename\" onclick=\"frename()\">");
buffcatf (&out, "<input type=\"button\" class=\"button\" value=\"Copy\" onclick=\"fcopy()\">");
buffcatf (&out, "<input type=\"button\" class=\"button\" value=\"Move\" onclick=\"fmove()\">");
buffcatf (&out, "<input type=\"button\" class=\"button\" value=\"Delete\" onclick=\"fdelete()\"><br>\n");

if (viewtype == img)
	buffcatf (&out, "<img src=\"%s\" width=\"600px\" height=\"auto\"></img>", request.path);

if (viewtype == txt)
{
    printf ("generating text preview\n");
int localfd = open (request.full_path, O_RDONLY);
if (localfd == -1)
	goto skip;

//struct string_data txtdata;
struct buffer_data in;
char indata [maxbuffer];
char outdata [maxbuffer];
struct buffer_data pout;
in.p = indata;
pout.p = outdata;
pout.len = 0;

in.len = read (localfd, in.p, maxbuffer);
close (localfd);

for (int i = 0; i < in.len; ++i)
{
if (pout.len > maxbuffer - 8)
break;

if (in.p[i] == '\n')
{
int l = strlen ("<br>\n");
memcpy (pout.p + pout.len, "<br>\n", l);
pout.len += l;
continue;
}

if (in.p[i] == '>')
{
int l = strlen (">");
memcpy (pout.p + pout.len, ">", l);
pout.len += l;
continue;
}

if (in.p[i] == '<')
{
int l = strlen ("<");
memcpy (pout.p + pout.len, "<", l);
pout.len += l;
continue;
}

pout.p[pout.len] = in.p[i];
++pout.len;
}

memcpy (out.p + out.len, pout.p, pout.len);
out.len += pout.len;

skip:;
} // if text preview

buffcatf (&out, "</body></html>");
struct string_data head;

head.len = sprintf (head.p, "%s%s%s%s%d\n\n", hthead, connka, conthtml, contlen, out.len);
//sock_writeold (request.fd, head.p, head.len);
//save_buffer (out, "actionpage.htm");
//sock_writeold (request.fd, out.p, out.len);

io_write (request.io, head.p, head.len);
io_write (request.io, out.p, out.len);

return 1;
    

} // get_action


int send_mredirect (const int fd, const char *msg, const char *uri)
{ // bm send m-redirect
char outbuff [string_sz];
char head [string_sz];

int doclen = sprintf (outbuff, "<html><body><script>window.alert(\"%s\"); window.location=\"%s\";</script></body></html>", msg, uri);



int headlen = sprintf (head, "%s%s%s%s%d\n\n", hthead, conthtml, connclose, contlen, doclen);

sock_writeold (fd, head, headlen);
sock_writeold (fd, outbuff, doclen);

return 1;
} //send_mredirect

int send_redirect (const int fd, const char *uri)
{ // bm send redirect
char outbuff [string_sz];
char head [string_sz];

int doclen = sprintf (outbuff, "<html><body><script>window.location=\"%s\";</script></body></html>", uri);



int headlen = sprintf (head, "%s%s%s%s%d\n\n", hthead, conthtml, connclose, contlen, doclen);

sock_writeold (fd, head, headlen);
sock_writeold (fd, outbuff, doclen);

return 1;
} //send_redirect

/*
int Base64Encode(const unsigned char* buffer, size_t length, char **b64text) { //Encodes a binary safe base 64 string
	BIO *bio, *b64;
	BUF_MEM *bufferPtr;

	b64 = BIO_new(BIO_f_base64());
	bio = BIO_new(BIO_s_mem());
	bio = BIO_push(b64, bio);

	BIO_set_flags(bio, BIO_FLAGS_BASE64_NO_NL); //Ignore newlines - write everything in one line
	BIO_write(bio, buffer, length);
	BIO_flush(bio);
	BIO_get_mem_ptr(bio, &bufferPtr);
	BIO_set_close(bio, BIO_NOCLOSE);
	BIO_free_all(bio);

	*b64text=(*bufferPtr).data;

	return (0); //success
}
*/
// bm websock
void getwebsock (const struct request_data request)
{
/*

d1 = strsearch (inbuff.p, "Sec-WebSocket-Key: ", d1, inbuff.len);
d2 = getnext (inbuff.p, 13, d1, inbuff.len);
strncpy (request.code, inbuff.p + d1, d2 - d1);
//request.codelen = midstr (inbuff.p, request.code, d1, d2);
request.codelen = d2 - d1;
return request;

   printf ("getwebsock\n");

struct string_data inbuff;

const char *response = "HTTP/1.1 101 Switching Protocols\r\nUpgrade: websocket\r\nConnection: Upgrade\r\nSec-WebSocket-Accept: ";

char *append = "258EAFA5-E914-47DA-95CA-C5AB0DC85B11";

char tosha [string_sz];
//size_t len = sprintf (tosha, "%s%s", request.code, append);

//strcpy (tosha, request.code);
//strcat (tosha, append);
//size_t len = strlen (tosha);

unsigned char *conv = (unsigned char*)tosha;
//char data[] = "Hello, world!";
//size_t length = strlen(data);
unsigned char hash[SHA_DIGEST_LENGTH];
//SHA1(conv, len, hash); // hash now contains the 20-byte SHA-1 hash

//printf ("hash: %s\n", hash);

char *b64rtn;


  Base64Encode(hash, SHA_DIGEST_LENGTH, &b64rtn);


char handshake [200];
len = sprintf (handshake, "%s%s\r\n\r\n", response, b64rtn);

int a = sock_1writeold (request.fd, handshake, len);

if (a != len)
	printf ("sorry\n");

//printf ("[ %s ] done\n", handshake);
//
inbuff.len = sock_readold (request.fd, inbuff.p, string_sz);
//printf ("%s\n", inbuff.p);

//int localfd = open (request.full_path, O_WRONLY | O_TRUNC| O_CREAT, S_IRUSR | S_IWUSR);
int loc = open ("frame.bin",  O_WRONLY | O_TRUNC| O_CREAT, S_IRUSR | S_IWUSR);
if (loc ==-1)
{printf ("error"); exit (0);}

write (loc, inbuff.p, inbuff.len);

printf ("len: %d\n", inbuff.len);

*/
} // websock

/*
int get_nextsize (struct string_data *fdata)
{
int n = getnext (fdata->p, ':', fdata->procint + 1, fdata->len);
if (n ==-1) n = fdata->len;

char temp [10];

midstr (fdata->p, temp, fdata->procint + 1, n);

int rtn = atoi (temp);

fdata->procint = n;

return rtn;

}
*/

int process_get (const io_t io, const struct request_data request)
{ // bm process_get
printf ("GET: %s\n", request.url);
switch (request.mode)
{
case root:
//if (request.mode == root) {
	if (request.type == reg) serv_file (request);
	if (request.type == dir) serv_dir (request);

//printf ("root mode\n");
break;
case ace_builds:
	serv_file (request);
break;
case action:
	get_action (request);
break;
case favicon:
	servico (io);
break;
case edit:
	get_edit (request);
break;
default:

//char modetxt [100];
// err, upload, config, websock


send_txt (io, "unexpected error");
printf ("default %s\n", request.mode_text);
//}else if (request.mode == ace_builds) {


}//switch mode
return 1;
} // end process_get

int post_file (const int connfd, const struct request_data request)
{ // bm post file
/*
//	post_file (request);
//printf ("[%s]\n", inbuff.p);
memset (&filedata, 0, sizeof (struct post_file_data));

int rtn = get_boundary (&filedata, inbuff);
if (!rtn) {printf ("unable to get boundary\n"); goto make_dead;}
//printf ("boundary is: [%s]\n", file_data.boundary);
rtn = get_fname (&filedata, inbuff);
if (!rtn) 
{
filedata.offset = 0;
printf ("didn't locate filename...with head\n");
inbuff.len = sock_readold (connfd, inbuff.p, inbuff.max);
if (inbuff.len == -1)  {printf ("timed out recieving\n"); goto make_dead;}
int res = get_fname (&filedata, inbuff);
if (res == 0 || res == -1) {printf ("criticsl error\n"); goto make_dead;}
}

filedata.offset = 0;
while (filedata.content_prog < request.content_len)
{
inbuff.len = sock_readold (connfd, inbuff.p, inbuff.max);
if (inbuff.len == -1)  {printf ("timed out recieving\n"); goto make_dead;}
//if (filedata.stat_fname == 0) 
//if (get_fname (&filedata, inbuff) == -1) killme ("critical error")
write (filedata.fd, inbuff.p, inbuff.len);
filedata.content_prog += inbuff.len;  
filedata.fsize += inbuff.len;
printf ("\rrecieving %lu / %lu", filedata.content_prog, request.content_len); 
} // reader loop

printf ("\ndone recieving\n");
unsigned long final = (filedata.fsize - filedata.boundary_len - 8);

ftruncate (filedata.fd, final);

char temp [smbuff_sz] = "";
sprintf (temp, "file saved: %lu\n", final);
printf ("%s", temp);
send_txt2 (connfd, temp);

close (connfd);
break;
default:
	send_txt2 (connfd, "Unhandled mode");
*/
return 1;
} // post file

int run_make (const struct request_data request)
{// bm run make
printf ("run make! %s\n", request.mainbuff->p);

int pipefd[2];
pipe2(pipefd, O_NONBLOCK);
// 0 is child
if (fork() == 0)
{
printf ("hello from child\n");
close(pipefd[0]);    // close reading end in the child

dup2(pipefd[1], 1);  // send stdout to the pipe
dup2(pipefd[1], 2);  // send stderr to the pipe

char dirpath [smbuff_sz];
int rtn = getlast (request.full_path, (int) '/', strlen(request.full_path));
if (rtn == -1) killme ("unexpected occurence");
printf ("rtn %d\n", rtn);
memcpy (dirpath, request.full_path, rtn);
dirpath [rtn] = 0;
printf ("dirpath [%s]\n", dirpath);

execlp ("make", "make", "-C", dirpath, (char *) NULL);
//execlp("./hellow", "hellow", "hello world", NULL);
return 0;
}else{
printf ("hello from parent\n");
    // parent
char buffer[smbuff_sz];
char cres [mdbuff_sz];
buffer_t res;
res.p = cres;
res.max = mdbuff_sz;
res.len = 0;

close(pipefd[1]);  // close the write end of the pipe in the parent

struct pollfd pll;
pll.fd = pipefd[0];
pll.events = POLLIN;
//while (1)
for (int i = 0; i < 50; ++i)
{
printf ("poll wait (run make) ... ");
int events = poll (&pll, 1, 5000);
printf ("events %d\n", events);
if (events <= 0) {printf ("no events\n"); break;}
int len = read (pipefd[0], buffer, smbuff_sz);
if (len == 0) break;
buffer[len] = 0;
//printf ("(%d) len %d, %.*s\n", i, len, len, buffer);

res.len += snprintf (res.p + res.len, res.max, "%s", buffer);

} // while

res.p [res.len] = 0;
//buffer [len] = 0;
//printf ("recieved(%d): %.*s\n", len, len, buffer);
send_txt2 (request.fd, res.p);
return 1;
} // if else child

} // run make

int post_config (const struct request_data request)
{ // bm post config
 /*   send_txt2 (request.fd, "post config");

int frtn = fork ();
if (frtn == -1) killme ("config fork err");
else if (frtn == 0)
{

    int r = execvp (argv [0], argv);
    printf ("%d post config\n", r);

}
else killme ("kill parent");  
  
  
 */ 
    return 1;
} // post config

int process_post (const io_t io, const struct request_data request)
{ // bm process_post
printf ("POST: %s\n", request.url);

switch (request.mode) {
case edit:
 	if (!strcmp (request.params, "make")) run_make (request);
	else post_edit (request);

break;
case upload:
	post_file (io.connfd, request);
	
break;
case config:
    post_config (request);

break;
default:
	send_txt (io, "Unhandled mode");
} // switch
return 1;
} // process post

int tls_accept (io_t *io)
{ // bm tls accept

while (1)
{
struct pollfd pfd;
pfd.fd = io->connfd;
pfd.events = POLLIN | POLLOUT;
int rtn = poll (&pfd, 1, timeout);
if (rtn < 1) {printf ("tls accept failure 754\n"); return -1;} 
int rt = SSL_accept(io->ssl);

if (rt > 0)
    return 1;

else if (rt <= 0)
{
int rt2 = SSL_get_error(io->ssl, rt);

if (rt2 == SSL_ERROR_WANT_WRITE || rt2 == SSL_ERROR_WANT_READ) {
//printf ("want read / write\n");
continue;

} else if (rt2 == SSL_ERROR_WANT_CONNECT || rt2 == SSL_ERROR_WANT_ACCEPT){
//printf ("want connect / accept\n");
continue;
    
} else {
printf ("non recoverable error\n");
   return -1;
} //if rt2

} // if rt-1

} // while

return -1;
}

void *thread_function (void *arg)
{// bm thread func
struct sthread_db *data = arg;
printf ("thread init id %d\n", data->id);
while (1) {
pthread_mutex_lock(&mutex);
pthread_cond_wait(&condition_var, &mutex);
int connfd = trigger.connfd;
trigger.use = false;
pthread_mutex_unlock(&mutex);



char inbuffer [string_sz];
struct buffer_data inbuff;
inbuff.p = inbuffer;
inbuff.max = (string_sz);


io_t io;
io.connfd = connfd;
if (settings.tls == true)
{
io.ssl = SSL_new(ctx);
SSL_set_fd(io.ssl, connfd);
int rtn = tls_accept (&io);
if (rtn == -1) continue;
printf ("thread func ssl %p\n", io.ssl);
}

// bm keep alive loop
while (1)
{
//inbuff.len = sock_readold (connfd, inbuff.p, inbuff.max);
inbuff.len = io_read1 (io, inbuff.p, inbuff.max);
if (inbuff.len == -1)
{ printf ("[%d, (%d)] client timed out\n", data->id, connfd);  close (connfd); break; }
inbuff.p[inbuff.len] = 0;

//struct post_file_data filedata;
struct request_data request;
int ret = parse_request (&request, connfd, inbuff);
if (ret == 0) {close (connfd); break;}

request.mainbuff = &inbuff;
request.io = io;
//send_txt (io, "testing worked");


if (request.method == 'G') {
process_get (io, request);
//printf ("get from thread %d\n", data->id);
} else if (request.method == 'P') {
process_post (io, request);
} // else if POST


} // keep alive loop

if (settings.tls == true) 
{
SSL_shutdown(io.ssl);
SSL_free(io.ssl);
}
//send_txt2 (connfd, "it works, thanks");
close (io.connfd);

} // thread loop
return NULL;
} // thread function

int main (int argc, char **argv)
{ // bm main top
signal(SIGPIPE, SIG_IGN);
for (int i = 1; i < argc; ++i)
{

if (!strcmp (argv[i], "-p"))
settings.port = atoi (argv[i+1]);


if (!strcmp (argv[i], "-dir"))
strcpy (settings.base_path, argv[i+1]);

if (!strcmp (argv[i], "-editor"))
strcpy (settings.editor, argv[i+1]);

if (!strcmp (argv[i], "-notls"))
settings.tls = false;

} // for settings

struct sockaddr_in address;
socklen_t addrlen = sizeof(address);

int servfd = prepsocket (settings.port);

if (settings.tls == true)
{ctx = create_context(); configure_context(ctx);}

printf ("admin load\nPort: %d\nPath: %s\nEditor: %s\n", settings.port, settings.base_path, settings.editor);

char inbuffer [string_sz];
struct buffer_data inbuff;
inbuff.p = inbuffer;
inbuff.max = (string_sz);

int do_fork = 0;
int loop_int = 1;

if (THREAD_POOL_SIZE) {
for (int i = 0; i < THREAD_POOL_SIZE; i++) {
thread_db [i].id = i;
pthread_create(&thread_pool[i], NULL, thread_function, &thread_db [i]);
}
} // if thread_pool_size

if (settings.tls == false)
{
io_read1 = sock_read1;
io_read = sock_read;
io_write1 = sock_write1;
io_write = sock_write;
send_file = sock_send_file;
}
// main loop
while (loop_int)
{
printf ("waiting\n");

int connfd = 0;
connfd = accept(servfd, (struct sockaddr *)&address, (socklen_t*)&addrlen);
if (connfd == -1) {printf ("connfd -1, pid: %d\n", getpid()); exit (0); continue;}
sock_setnonblock (connfd);

char str[INET_ADDRSTRLEN];
  inet_ntop(address.sin_family, &address.sin_addr, str, INET_ADDRSTRLEN);
printf("new connection from (%d) %s:%d\n", connfd, str, ntohs(address.sin_port));


if (THREAD_POOL_SIZE) {
pthread_mutex_lock(&mutex);

if (trigger.use == false)
{
trigger.use = true;
trigger.connfd = connfd;
pthread_cond_signal(&condition_var);
pthread_mutex_unlock(&mutex);
//close (connfd); broke it
continue;
}else{
killme ("no idle threads");
// no available threads
} //if trigger not freed

pthread_mutex_unlock(&mutex);
} // if do threads

if (do_fork)
{
int frtn = fork ();
if (frtn == -1) killme ("fork error");
else if (frtn == 0) loop_int = 0;
else {printf ("fid: %d\n", frtn); close (connfd); continue;}
}

// if do tls // here
/*
// keep alive loop
while (1)
{
inbuff.len = sock_readold (connfd, inbuff.p, inbuff.max);
if (inbuff.len == -1)
{ printf ("client timed out (main-loop)\n");  close (connfd); break; }
inbuff.p[inbuff.len] = 0;

struct post_file_data filedata;
struct request_data request;
int ret = parse_request (&request, connfd, inbuff);
if (ret == 0) {close (connfd); break;}

request.mainbuff = &inbuff;

//send_txt2 (connfd, "testing worked");
//break;

if (request.method == 'G') {
process_get (connfd, request);
printf ("get from main\n");
} else if (request.method == 'P') {
process_post (argv, servfd, connfd, request);
} // else if POST


} // keep alive loop
make_dead:;
 close (connfd); 
*/
// need to mamage connection, but must be through handling functions?
} // main loop
//printf ("exit\n");
//
if (settings.tls == true)    
SSL_CTX_free(ctx);
} // main

int get_fname (struct post_file_data *filedata, const buffer_t inbuff)
{ // bm get_fname
if (filedata->stat_fname) return 0; // if file already open return;

int offset = filedata->offset;
char *bp = memmem (inbuff.p + offset, inbuff.len - offset, filedata->boundary, filedata->boundary_len);
if (bp == NULL) {printf ("no init boundary data not started?\n"); return 0;}
int tempoffset = bp - inbuff.p;

filedata->content_prog = inbuff.len - tempoffset + 2;

char *p1 = (char *) memmem (inbuff.p, inbuff.len, "filename=", 9);
if (p1 == NULL)  {printf ("filename\n"); return -1;}
int start = p1 - inbuff.p + 10;
char *p2 = (char *) memchr (inbuff.p + start, 10, inbuff.len - start);
if (p2 == NULL ) {printf ("end file name\n"); return -1;}

int end = p2 - inbuff.p;
int len = end - start - 2;

//offset = end;
//get rid of MIME data included from browser
end += 15;
p1 = (char *) memchr (inbuff.p + end, 10, inbuff.len - end);
if (p1 == NULL) {printf ("cant find delim\n"); return -1;}
filedata->offset = p1 - inbuff.p + 3;

memcpy (filedata->fname, inbuff.p + start, len);
filedata->stat_fname = 1;

char fullpath [smbuff_sz];
sprintf (fullpath, "testdir/%s", filedata->fname);

int fd = open (fullpath, O_WRONLY | O_TRUNC| O_CREAT, S_IRUSR | S_IWUSR);
if (fd < 0) killme ("unable to open new file 523");

filedata->fsize = write (fd, inbuff.p + filedata->offset, inbuff.len - filedata->offset);

filedata->fd = fd;

//printf ("file name: [%s]\n", filedata->fname);

return 1;
} // get fname

int get_boundary (struct post_file_data *filedata, const buffer_t inbuff) 
{ // bm get_boundary

char *boundary_p1 = (char *) memmem (inbuff.p, inbuff.len, "boundary=", 9);
if (boundary_p1 == NULL) {return 0;}
int start = boundary_p1 - inbuff.p;

char *len_p = (char *) memchr (inbuff.p + start, 10, inbuff.len - start);
if (len_p == NULL) {return 0;}
int len = len_p - boundary_p1;

memcpy (filedata->boundary, inbuff.p + start + 9, len - 9);
filedata->boundary_len = trim (filedata->boundary);

filedata->offset = len_p - inbuff.p + filedata->boundary_len;
return 1;
} // get boundary

/*
int post_file2 (const struct request_data request)
{ // bm post_file
// at present, not reading whole file, and still saving the complete file
// some how!!!

//printf ("post file!!\n%s\n\n", request.mainbuff->p);
char boundary [default_sz] = "";
int boundary_len = 0;
int offset = 0;
{
//char *filelen = NULL;
char *boundary_p1 = (char *) memmem (request.mainbuff->p, request.mainbuff->len, "boundary=", 9);
if (boundary_p1 == NULL) {printf ("boundary not found\n"); return 0;}
int start = boundary_p1 - request.mainbuff->p;

char *len_p = (char *) memchr (boundary_p1, 10, request.mainbuff->len - start);
if (len_p == NULL) {printf ("no end of boundary!!!\n"); return 0;}

int end = len_p - boundary_p1;
offset = len_p - request.mainbuff->p;
memcpy (boundary, boundary_p1 + 9, end - 9);
boundary_len = trim (boundary);
} // bm boundary logic
//printf ("boundary is: [%s]\n", boundary);

int fd = 0;
unsigned long fsize;
int filenum = 0;
int file_count = -1;
int filelen_offset = 0;
char filelen_data [smbuff_sz] = "";
int filelen_data_len = 0;
unsigned long content_prog = 0;
unsigned long file_prog = 0;
//const int reciever_unit = 150000000; // 150 MB
const int reciever_unit = 50000000; // 50 MB

// set inbuff to content_len or reciever unit....whichever is smaller
buffer_t inbuff; 
if (request.content_len < reciever_unit) {
inbuff = init_buffer (request.content_len);
}else{inbuff = init_buffer (reciever_unit);}

struct b {
unsigned char *p;
int len;
int procint;
int max;
}inbuff;
memset (inbuff.p, 0, inbuff.max);
inbuff.p = (unsigned char *) calloc (1, reciever_unit);
if (inbuff.p == NULL) killme ("malloc 529");

char *res = (char *) memmem (request.mainbuff->p + offset + boundary_len, request.mainbuff->len - offset - boundary_len, boundary, boundary_len);
if (res != NULL) {
printf ("data started with head & does not include boundary\n");
offset = res - request.mainbuff->p + boundary_len;

int len = request.mainbuff->len - offset;

memcpy (inbuff.p, request.mainbuff->p + offset, len);
inbuff.len = len;
inbuff.p[len] = 0;
content_prog = len + boundary_len + 2;
//printf ("Content of data included with head\n[%s]\n", inbuff.p);
//save_buffer (inbuff, "post_file_inbuff.txt");
} // if data started with head info


while (content_prog < request.content_len) {
int len  = sock_readold (request.fd, inbuff.p + inbuff.len, smbuff_sz);
if (len == -1) {printf ("read -1\n"); break;}
if (len == 0) {printf ("read 0\n"); break;}
content_prog += len;
inbuff.len += len;
printf ("\rreading from socket %lu : %lu\n", content_prog, request.content_len);
//if ((inbuff.max - inbuff.len) < smbuff_sz) break;
} // end reader loop

unsigned char *p1 = memmem (inbuff.p + 5, inbuff.len -5, "fsize", 5);
if (p1 == NULL) {printf ("unexpected input 540\n"); return 0;}
//offset = p1 - inbuff.p + 8;
int start = p1 - inbuff.p + 8;
//printf ("start: %d\n", start);

unsigned char *p2 = memmem (inbuff.p + start, inbuff.len - start, boundary, boundary_len);
if (p2 == NULL) {printf ("unexpected input 544\n"); return 0;}
offset = p2 - inbuff.p;
//printf ("offset (end) %d\n", offset);
int len = offset - start - 2;
memcpy (filelen_data, inbuff.p + start, len);
filelen_data_len = trim (filelen_data);
//printf ("file len data [%s]\n", filelen_data);

p1 = strchr (filelen_data, (int) ':');
if (p1 == NULL) {printf ("unexpected input or 0 files? 570\n"); return (0);}

filelen_offset = p1 - filelen_data;

char temp [default_sz] = "";
memcpy (temp, filelen_data, filelen_offset);

printf ("file count: %s\n", temp);
file_count = atoi (temp);


p1 = memmem (inbuff.p + offset, inbuff.len - offset, "filename=", 9);
if (p1 == NULL) {printf ("unexpected input 601\n"); return 0;}
start = p1 - inbuff.p + 10;
p2 = memchr (inbuff.p + start, 10, inbuff.len - start);
if (p2 == NULL ) {printf ("unexpected input 604\n"); return 0;}

int end = p2 - inbuff.p;
len = end - start - 2;
offset = end;
//printf ("start - offset(end): [%d][%d]\n", start, offset);

char fname [default_sz]  = "";
memcpy (fname, inbuff.p + start, len);

printf ("file name: [%s]\n", fname);

char fullpath [smbuff_sz];
snprintf (fullpath, smbuff_sz, "%s/%s", "testdir", fname);


fd = open (fullpath, O_WRONLY | O_TRUNC| O_CREAT, S_IRUSR | S_IWUSR);
if (fd < 0) killme ("unable to open new file 600");

// get file length from data
start = filelen_offset;
// get fsize after fname
p2 = strchr (filelen_data + start + 1, (int) ':');
if (p2 == NULL) {filelen_offset = filelen_data_len;}
else {filelen_offset = p2 - filelen_data;}

memset (temp, 0, default_sz);
memcpy (temp, filelen_data + start+1, filelen_offset - start-1);

printf ("fsize: %s\n", temp);    
fsize = atol (temp);    
++filenum;

// found filename, and file length, now open
//get rid of MIME data included from browser
offset += 15;
p1 = memchr (inbuff.p + offset, 10, inbuff.len - offset);
if (p1 == NULL) {printf ("unexpected 626\n"); return 0;}
offset = p1 - inbuff.p + 2;

//printf ("offset / len %d/%d\n", offset, inbuff.len);
write (fd, inbuff.p + offset, fsize);
close (fd);
fd = 0;

//printf ("inbuff [%.*s]\n", inbuff.len, inbuff.p);

//save_buffer (inbuff, "saved_inbuff.txt");

send_txt2 (request.fd, "recieve file here\n");

//free (sock_buff.p); 
free (inbuff.p);

close (request.fd);

return 1;
}// post file

*/





int serv_dir (const struct request_data request)
{ // bm serv_dir
const char *template_path = "internal/dir.htm";
//printf ("servdir!!!!!!!\n");
struct stat finfo;
if (stat (template_path, &finfo) != 0)
	{send_txt2 (request.fd, "cant stat dir template"); return 0;}

buffer_t dir_template = init_buffer (finfo.st_size);
int dirfd = open (template_path, O_RDONLY);
if (dirfd < 0) {send_txt2 (request.fd, "cannot open dir template"); return 0;}
dir_template.len = read (dirfd, dir_template.p, dir_template.max);
close (dirfd);


DIR *dp;
struct dirent *ep;

buffer_t dir_list = init_buffer (lgbuff_sz);

dp = opendir (request.full_path);
if (dp == NULL)
	{send_txt2 (request.fd, "OOPS"); return -1;}

while ((ep = readdir (dp)))
{
if (ep->d_name[0] == '.')
	continue;
	
// 4 is dir 8 is file

if (ep->d_type == 4)

(request.url[strlen (request.url) - 1] == '/')?
buffcatf (&dir_list, "<a href=\"%s%s\">%s/</a><br>\n", request.url, ep->d_name, ep->d_name):
buffcatf (&dir_list, "<a href=\"%s/%s\">%s/</a><br>\n", request.url, ep->d_name, ep->d_name);

//printf ("char: %c, dirname is: %s\n", request.url[strlen (request.url) - 1], request.url.p);
} // while

closedir (dp);
dp = opendir (request.full_path);

while ((ep = readdir (dp)))
{
if (ep->d_name[0] == '.')
	continue;
	
if (ep->d_type == 8)

if (settings.showaction > 0)	
(request.url[strlen (request.url) - 1] == '/')?
buffcatf (&dir_list, "<a href=\"/action%s%s\">%s</a><br>\n", request.path, ep->d_name, ep->d_name):
buffcatf (&dir_list, "<a href=\"/action%s/%s\">%s</a><br>\n", request.path, ep->d_name, ep->d_name);


if (settings.showaction == 0)	
(request.url[strlen (request.url) - 1] == '/')?
buffcatf (&dir_list, "<a href=\"/%s%s\">%s</a><br>\n", request.path, ep->d_name, ep->d_name):
buffcatf (&dir_list, "<a href=\"/%s/%s\">%s</a><br>\n", request.path, ep->d_name, ep->d_name);


} // while

//<!--linklist-->

buffer_t bdp;
char cdp [smbuff_sz];
strcpy (cdp, request.path);
bdp.p = cdp;
bdp.len = strlen (request.path);

FAR (&dir_template, "<!--linklist-->", dir_list);
FAR (&dir_template, "DIR_PATH", bdp);

struct string_data head;

head.len = sprintf (head.p, "%s%s%s%d\n\n", hthead, conthtml, contlen, dir_template.len);

//sock_1writeold (request.fd, head.p, head.len);
//sock_buffwrite (request.fd, &dir_template);

io_write (request.io, head.p, head.len);
io_write (request.io, dir_template.p, dir_template.len);
closedir (dp);
//save_buffer (dir_list, "dir_list.txt");
//save_buffer (dir_template, "dir_template.txt");


free (dir_template.p);
free (dir_list.p);
return 1;

} // serv_dir

int serv_dirold (const struct request_data request)
{//bm serv_dirold
DIR *dp;
struct dirent *ep;

char outb [maxbuffer];
struct buffer_data out;
out.p = outb;
out.len = 0;
out.max = maxbuffer;


 buffcatf (&out, "<!DOCTYPE html>\n<html>\n<head>\n");

 buffcatf (&out,"<style>\n");
 buffcatf (&out,"body\n{\ntext-align:left;\nmargin-left:70px;\nbackground-color:aqua;\nfont-size:24px;\n}\n");
 buffcatf (&out, "a:link\n{\ncolor:midnightblue;\ntext-decoration:none;\n}\n");


buffcatf (&out, ".button {\npadding-left:20px;\npadding-right:20px;\nfont-size:18px;\n}");
 buffcatf (&out, "</style>\n</head>\n<body>\n");

buffcatf (&out, "<script>");
buffcatf (&out, "function get_multi () { \n");

buffcatf (&out, "var count = window.prompt (\"How Many\", 2);\n");
buffcatf (&out, "var content = \"<form enctype=\'multipart/form-data\' action=\'%s\' method=\'post\'>\";\n", request.url);
buffcatf (&out, "var i;\n");
buffcatf (&out, "for (i= 0; i < count; ++i)\n");
buffcatf (&out, "content += \"<input type=\'file\' class=\'button\' name=\'myFile\'>\";\n");
buffcatf (&out, "content += \"<input type=\'submit\' class=\'button\'  value=\'upload\'>\";\n");
buffcatf (&out, "content += \"</form>\";\n");
buffcatf (&out, "document.getElementById(\"uploader\").innerHTML = content;\n}\n");


// end function

buffcatf (&out, "</script>\n");


dp = opendir (request.full_path);
if (dp == NULL)
	{send_txt2 (request.fd, "OOPS"); return -1;}

buffcatf (&out, "%s<br>\n", request.path);

buffcatf (&out, "<input type=\"button\" class=\"button\" value=\"make\">\n");

buffcatf (&out, "<input type=\"button\" class=\"button\" value=\"mkdir\">\n");
buffcatf (&out, "<input type=\"button\" class=\"button\" value=\"new\"><br>\n");

buffcatf (&out, "<input type=\"button\" class=\"button\" value=\"search\">\n");
// addonclick here
buffcatf(&out, "<input type=\"button\" class=\"button\" value=\"Multiple Upload\" onclick=\"get_multi ()\">\n");
//

buffcatf (&out, "<div id=\"uploader\">\n");
buffcatf (&out, "<form enctype=\"multipart/form-data\" action=\"%s\" method=\"post\">\n", request.url);
buffcatf (&out, "<input type=\"file\" class=\"button\" name=\"myFile\">\n");
buffcatf (&out, "<input type=\"submit\" class=\"button\"  value=\"upload\">\n</form>\n");
buffcatf (&out, "</div>\n");

while ((ep = readdir (dp)))
{
if (ep->d_name[0] == '.')
	continue;
	
// 4 is dir 8 is file

if (ep->d_type == 4)

(request.url[strlen (request.url) - 1] == '/')?
buffcatf (&out, "<a href=\"%s%s\">%s/</a><br>\n", request.url, ep->d_name, ep->d_name):
buffcatf (&out, "<a href=\"%s/%s\">%s/</a><br>\n", request.url, ep->d_name, ep->d_name);

//printf ("char: %c, dirname is: %s\n", request.url[strlen (request.url) - 1], request.url.p);
} // while

closedir (dp);
dp = opendir (request.full_path);

while ((ep = readdir (dp)))
{
if (ep->d_name[0] == '.')
	continue;
	
if (ep->d_type == 8)

if (settings.showaction > 0)	
(request.url[strlen (request.url) - 1] == '/')?
buffcatf (&out, "<a href=\"/action%s%s\">%s</a><br>\n", request.path, ep->d_name, ep->d_name):
buffcatf (&out, "<a href=\"/action%s/%s\">%s</a><br>\n", request.path, ep->d_name, ep->d_name);


if (settings.showaction == 0)	
(request.url[strlen (request.url) - 1] == '/')?
buffcatf (&out, "<a href=\"/file%s%s\">%s</a><br>\n", request.path, ep->d_name, ep->d_name):
buffcatf (&out, "<a href=\"/file%s/%s\">%s</a><br>\n", request.path, ep->d_name, ep->d_name);


//stradd (outbuff.p, ep->d_name, &outbuff.len);

} // while



buffcatf (&out, "</body>\n</html>");
// save page
save_buffer (out, "dir.htm");
//savepage
printf ("%d: bytes directory info\n", out.len);
struct string_data head;

head.len = sprintf (head.p, "%s%s%s%d\n\n", hthead, conthtml, contlen, out.len);
sock_1writeold (request.fd, head.p, head.len);
sock_buffwrite (request.fd, &out);

closedir (dp);

return 1;
} // serv_dir

int serv_file (const struct request_data request)
{ // bm serv_file
//printf ("serv_file fd: %d\n", request.fd);

struct string_data outbuff;
const char *mime_txt;

if (strcmp(request.ext, ".txt") == 0) {
mime_txt = conttxt;

}else if (strcmp(request.ext, ".c") == 0) {
mime_txt = conttxt;

}else if (strcmp(request.ext, ".cpp") == 0) {
mime_txt = conttxt;

}else if (strcmp(request.ext, ".h") == 0) {
mime_txt = conttxt;

}else if (strcmp(request.ext, ".hpp") == 0) {
mime_txt = conttxt;

}else if (strcmp(request.ext, ".htm") == 0) { 
mime_txt = conthtml;

}else if (strcmp(request.ext, ".html") == 0) {
mime_txt = conthtml;

}else if (strcmp(request.ext, ".js") == 0) {
mime_txt = contjava;

}else if (strcmp(request.ext, ".jpg") == 0) {
mime_txt = contjpg;

}else if (strcmp(request.ext, ".jpeg") == 0) {
mime_txt = contjpg;

}else if (strcmp(request.ext, ".css") == 0) {
mime_txt = contcss;

}else if (strcmp(request.ext, ".ico") == 0) {
mime_txt = conticon;

}else if (strcmp(request.ext, ".png") == 0) {
mime_txt = contpng;

}else if (strcmp(request.ext, ".mp4") == 0) {
mime_txt = contmp4;

}else if (strcmp(request.ext, ".pdf") == 0) {
mime_txt = contpdf;

}else {mime_txt = contoctet;}

//printf ("mime %s\n", mime_txt);

//struct stat finfo;
//stat (request.full_path, &finfo);

outbuff.len = sprintf (outbuff.p, "%s%s%s%ld\n\n", hthead, mime_txt, contlen, request.content_len);

//printf ("%ld bytes: %s", request.content_len, mime_txt);

sock_1writeold (request.fd, outbuff.p, outbuff.len);
//printf ("serv file fullname :%s\n", request.full_path);

int r = send_file (request.io, request.full_path);
//int r = send_file2 (request.full_path, request.fd);
printf ("send file: %d\n", r);
return r;

} // serv_file`



int parse_request (struct request_data *request, const int fd, const struct buffer_data inbuff)
{ // bm parse_request
memset (request, 0, sizeof (struct request_data));

request->method = inbuff.p [0];
request->mode = root;

//printf ("request method is: %c\n", request->method);

request->fd = fd;

const int url_len = extract_CC (inbuff, request->url, smbuff_sz, ' ', ' ');
if (url_len == 0) return 0;

//printf ("URL found: [%s]\n", request->url);

if (strcmp (request->url, "/favicon.ico") == 0)
	{request->mode = favicon; return 1;}

//printf ("PASSED MOTHERFUCKING FAVICON>>>>>FUCKING MAGIC!!!!!\n");

int path_end = url_len;
int path_start = 0;

char *p1 = strchr (request->url, '?');
if (p1 != NULL) {
path_end = p1 - request->url;
memcpy (request->params, request->url + path_end + 1, url_len - path_end - 1);
request->params [url_len - path_end] = 0;
//printf ("URL Params: %s\n", request->params);
} // if uri params

p1 = strstr (request->url, edit_mode);
if (p1 != NULL) {
path_start = edit_mode_len;
request->mode = edit;
request->mode_text = edit_mode;
printf ("edit mode det!\n");
} // if edit_mode

p1 = strstr (request->url, action_mode);
if (p1 != NULL) {
path_start = action_mode_len;
request->mode = action;
request->mode_text = action_mode;
} // if action_mode

p1 = strstr (request->url, config_mode);
if (p1 != NULL) {
path_start = config_mode_len;
request->mode = config;
request->mode_text = config_mode;
printf ("config mode det\n");
} // if config_mode


p1 = strstr (request->url, upload_mode);
if (p1 != NULL) {
path_start = upload_mode_len;
request->mode = upload;
request->mode_text = upload_mode;
} // if upload_mode

p1 = strstr (request->url, ace_builds_mode);
if (p1 != NULL) {
//path_start = 0;
request->mode = ace_builds;
request->mode_text = ace_builds_mode;
int pathlen = snprintf (request->full_path, string_sz, "%s%s", settings.ace_builds, request->url);

struct stat finfo;
if (stat (request->full_path, &finfo) == 0)
request->content_len = finfo.st_size; else return 0;

int rtn = getlast (request->full_path, (int) '.', pathlen);
if (rtn != -1) {
memcpy (request->ext, request->full_path + rtn, pathlen - rtn);
request->ext [pathlen - rtn] = 0;
//printf ("Ext: [%s]\n", request->ext);
} // if ext found

rtn = getlast (request->full_path, (int) '/', pathlen);
if (rtn != -1) {
memcpy (request->filename, request->full_path + rtn, pathlen - rtn);
request->ext [pathlen - rtn] = 0;
//printf ("filename: [%s]\n", request->filename);
} // if filename found

//printf ("ace parse det: %s\n", request->full_path);
return 1;
} // if ace_builds


char encoded_url [smbuff_sz];
int path_len = path_end - path_start;
memcpy (encoded_url, request->url + path_start, path_len);
encoded_url [path_len] = 0;
//printf ("temp path (%d) [%s]", path_len, temp);

path_len = URL_decode (encoded_url, request->path);
//printf ("URL:(%d) [%s]\n", url_len, request->url);
//printf ("Path: (%d) [%s]\n", path_len, request->path);

int rtn = getlast (request->path, (int) '/', path_len);
if (rtn != -1) {
memcpy (request->filename, request->path + rtn + 1, path_len - rtn - 1);
request->filename [path_len - rtn] = 0;
//printf ("filename: [%s]\n", request->filename);

} // if filename found

rtn = getlast (request->path, (int) '.', path_len);
if (rtn != -1) {
memcpy (request->ext, request->path + rtn, path_len - rtn);
request->ext [path_len - rtn] = 0;
//printf ("Ext: [%s]\n", request->ext);

} // if ext found

// now stat file for get requests
// populate content len for get file
// if POST request populate content len

snprintf (request->full_path, default_sz, "%s%s", settings.base_path, request->path);
if (request->method == 'G')
{
struct stat finfo;
if (stat (request->full_path, &finfo) == 0)
{
if (S_ISDIR(finfo.st_mode))
	request->type = dir;
if (S_ISREG(finfo.st_mode)) // is file
	{request->type = reg; request->content_len = finfo.st_size;}
} // if type
return 1;
}


if (request->method == 'P') {
int termlen = strlen ("Content-Length: ");
char *p1 = (char *) strcasestr (inbuff.p, "Content-Length: ");
if (p1 == NULL) return 0;

int d1 = p1 - inbuff.p;

char *p2 = strchr (inbuff.p + d1, 10);
int d2 = p2 - inbuff.p;

char temp [20];
memset (temp, 0, 20);

int len = d2 - d1;
memcpy (temp, inbuff.p + d1 + termlen, len - termlen - 1);

request->content_len = atol (temp);
return 1;
//printf ("[%lu]\n", request->content_len);
} //if post  get cont len

printf ("parse request end of function (error)\n");
return 0;
} // process_request

int get_edit (const struct request_data request)
{ // bm get_edit
buffer_t editor;
buffer_t filedata;

{
struct stat finfo;
char editor_path [smbuff_sz];
sprintf (editor_path, "internal/%s", settings.editor);
if (stat (editor_path, &finfo)) {send_txt2 (request.fd, "cant stat Editor"); return 0;}
editor = init_buffer (finfo.st_size);
int editor_fd = open (editor_path, O_RDONLY);
if (editor_fd < 0) {send_txt2 (request.fd, "cant open Editor"); return 0;}
editor.len = read (editor_fd, editor.p, editor.max);
editor.p[editor.len] = 0;
close (editor_fd);

if (stat (request.full_path, &finfo)) {send_txt2 (request.fd, "cant stat FILE"); return 0;}
filedata = init_buffer (finfo.st_size);
int file_fd = open (request.full_path, O_RDONLY);
if (file_fd < 0) {send_txt2 (request.fd, "cant open FILE"); return 0;}
filedata.len = read (file_fd, filedata.p, filedata.max);
filedata.p[filedata.len] = 0;
close (file_fd);
} // end file stat


// process bookmarks
buffer_t bookmarks = init_buffer (mdbuff_sz);
int linecount = 0;
char *feed = filedata.p;
buffcatf (&bookmarks, "<select class=\"button\" onchange=\"bookmark(event)\">\n");
const int bmlen = strlen (BOOKMARK);
while (1)
{
char line [smbuff_sz];
++linecount;
feed = parse_line (line, feed);
if (feed == NULL) break;

char *ret = strstr (line, BOOKMARK);
if (ret != NULL)
{
int offset = ret - line;
char bm [default_sz];
int len = strnlen (line, default_sz);
memcpy (bm, line + offset + bmlen, len - offset - bmlen);
bm [len - offset - bmlen] = 0;

buffcatf (&bookmarks, "<option value=\"%d\">%s</option>\n", linecount, bm);
} // if bookmark found

} // while
buffcatf (&bookmarks, "</select>\n");

struct far_builder builder = far_init (&editor);

int r1 = far_add (&builder, "<!--bookmarks-->", bookmarks.p, bookmarks.len);
int r2 = far_add (&builder,"RESOURCE_PATH", request.path, -1);

if (r1 == 0  || r2 == 0)
killme ("far add = 0\n");

buffer_t page = far_build (&builder);

struct string_data head;
head.len = sprintf (head.p, "%s%s%s%d\n\n", hthead, conthtml, contlen, page.len);

printf ("%d bytes: edit file served\n", page.len);

io_write (request.io, head.p, head.len);
io_write (request.io, page.p, page.len);
//sock_1writeold (request.fd, head.p, head.len);
//sock_buffwrite (request.fd, &page);

//save_buffer (page, "get_edit.txt");

free (page.p);
free (filedata.p);
free (editor.p);
//free (tempb.p);
free (bookmarks.p);
//send_txt2 (request.fd, "wow");

return 1;
} // get_edit


int post_edit (const struct request_data request)
{ // bm post edit
//save_buffer (request.mainbuff, "POST_EDIT.txt");


int progress = 0;

buffer_t encoded = init_buffer (request.content_len);
//memset (encoded.p, 0, encoded.max)

char *p1 = strchr (request.mainbuff->p, (int) '\"');
if (p1 != NULL) {
printf ("started in 1st xmission\n");
int d1 = p1 - request.mainbuff->p;
memcpy (encoded.p, request.mainbuff->p + d1, request.mainbuff->len - d1);
encoded.len = request.mainbuff->len - d1;
progress = request.mainbuff->len - d1;
}
//printf ("content en: %u, progress: %d\n", request.content_len, encoded.len);

/*
if (request.content_len == progress) {
printf ("fully contained in first xmission \"\n");
encoded.len -= 2;
}
*/


while (progress < request.content_len) {
encoded.len += sock_readold (request.fd, encoded.p + encoded.len, encoded.max);
//printf ("multi-reciever\n");
progress = encoded.len;
}
//printf ("finished cat json\n[%.*s]\n", encoded.len, encoded.p);

//save_buffer (encoded, "JSONencoded.txt");

char backup [string_sz];
strcpy (backup, "old/");
strcat (backup, "%H:%M-");
strcat (backup, request.filename);

//strcat (backup, request.ext);

time_t t;
struct tm *tmp;
t = time(NULL); 
tmp = localtime(&t);
if (tmp == NULL) { perror("localtime"); exit(EXIT_FAILURE); } 
char outstr [string_sz];
if (strftime(outstr, sizeof(outstr), backup, tmp) == 0) 
{ fprintf(stderr, "strftime returnd 0"); exit(EXIT_FAILURE); } 
printf ("backup name [%s]\n", outstr);

if (rename (request.full_path, outstr) == -1)
	printf ("error moving backup file\n");

buffer_t decoded = JSON_decode (encoded);

int localfd = open (request.full_path, O_WRONLY | O_TRUNC| O_CREAT, S_IRUSR | S_IWUSR);
if (localfd < 0) {
printf ("errno %d\n", errno);
killme ("cannot open file to save");
} // if error


write (localfd, decoded.p, decoded.len);
close (localfd);

send_txt2 (request.fd, "it worked");
free (encoded.p);
free (decoded.p);


return 1;

} //post edit


int servico (const io_t io)
{ // bm servico
  struct string_data outbuff;
   
struct stat finfo;
stat ("favicon.ico", &finfo);

outbuff.len = sprintf (outbuff.p, "%s%s%s%ld\n\n", hthead,  conticon, contlen, finfo.st_size);
io_write (io, outbuff.p, outbuff.len);

send_file (io, "favicon.ico");
//send_file2 ("favicon.ico", fd);
return 1;
} // servico

/*
void safe_fname (const struct request_data request, const char *fname, char *rtn)
{
  sprintf (rtn, "%s/%s", request.full_path, fname);
  struct stat finfo;
  int check = stat (rtn, &finfo);
  if (check != 0) return;

 int copynum = 1;
 while (check == 0)
 {
    sprintf (rtn, "%s/%s-%d",request.full_path, fname, copynum);  
      check = stat (rtn, &finfo);

      ++copynum;
 }// while
}// safe_fname
*/

int send_err (const int fd, const int code)
{ // bm send err

if (code == 410)
	sock_1writeold (fd, "HTTP/1.1 410 GONE", 0);


if (code == 500)
	sock_1writeold (fd, "HTTP/1.1 500 ERROR", 0);

return 1;
} // send_err

int send_txt (const io_t io, const char *txt)
{ // bm send_txt

int len = strnlen (txt, maxbuffer);

char outbuffer [maxbuffer];
struct buffer_data outbuff;
outbuff.p = outbuffer;
outbuff.len = 0;
outbuff.max = maxbuffer;

outbuff.len = snprintf (outbuff.p, maxbuffer, "%s%s%s%s%d\n\n%.*s", hthead, conttxt, connclose, contlen, len, len, txt);

io_write (io, outbuff.p, outbuff.len);

return 1;
} // send_txt2

int send_txt2 (const int fd, const char *txt)
{

int len = strlen (txt);

char outbuffer [maxbuffer];
struct buffer_data outbuff;
outbuff.p = outbuffer;
outbuff.len = 0;
outbuff.max = maxbuffer;

outbuff.len = sprintf (outbuff.p, "%s%s%s%s%d\n\n%.*s", hthead, conttxt, connclose, contlen, len, len, txt);

sock_buffwrite (fd, &outbuff);

return 1;
} // send_txt2


int sock_buffwrite (const int connfd, struct buffer_data *out)
{ // bm sock buffwrite
int wlen = 0;
int progress = 0;
int offset = 0;
int i;

int written = sock_1writeold (connfd, out->p, out->len);
if (written == -1)
	return -1;

if (written == out->len)
	return written;

progress = written;

while (progress < out->len)
{
wlen = 0;
for (i = offset; i < 500 - offset; ++i)
{
++wlen;
out->p [i] = out->p [wlen + progress];
if (i + offset == 500)
	break;

if (progress + wlen == out->len)
       break;

}// for reset op 
int totalwrite = wlen + offset;

written = sock_1writeold (connfd, out->p, totalwrite);
if (written == -1)
	return -1;

if (written == totalwrite)
	offset = 0;

if (written < totalwrite)
{
offset = wlen - written;

for (i = 0; i < offset; ++i)
{
out->p[i] = out->p[i + written];


} // for copy offset

} // if incomplete writer
progress += wlen;


} // while progress <
return 1; 
} // sock_buffwrite



int sock_send_file (const io_t io, const char *path)
{ // bm sock sendfile

//works!
//return send_file2 (path, io.connfd);
struct pollfd pfd;
pfd.fd = io.connfd;
pfd.events = POLLOUT;

int locfd = open (path, O_RDONLY);
if (locfd < -1)
    return -1;
//const int connfd = io.connfd;

struct stat finfo;
fstat (locfd, &finfo);

size_t read_progress = 0;

size_t write_progress = 0;

size_t fsize = finfo.st_size;
//printf (200, "file size: %d\n", fsize);

char *c_fbuffer = (char *) malloc (sendfileunit);
struct buffer_data fbuff;
fbuff.p = c_fbuffer;
fbuff.max = sendfileunit;


while (read_progress < fsize)
{  
fbuff.len = read (locfd, fbuff.p, fbuff.max);
read_progress += fbuff.len;


int rtn = poll (&pfd, 1, timeout * 1000);
if (rtn < 0) return -1;

int interim_progress = write (io.connfd, fbuff.p, fbuff.len);
if (interim_progress == -1)
	return -1;

int cpylen = interim_progress;
while (cpylen < fbuff.len)
{

int rtn = poll (&pfd, 1, timeout * 1000);
if (rtn < 0) return -1;
interim_progress = write (io.connfd, fbuff.p + cpylen, fbuff.len - cpylen);
if (interim_progress == -1)
	return -1;
cpylen += interim_progress;
} // while

} // while loop
close (locfd);
free (c_fbuffer);
return 1;

} // sock_send_file


int tls_send_file (const io_t io, const char *path)
{ // bm tls sendfile

//return send_file2 (path, io.connfd);
struct pollfd pfd;
pfd.fd = io.connfd;
pfd.events = POLLOUT;

int locfd = open (path, O_RDONLY);
if (locfd < -1)
    return -1;
//const int connfd = io.connfd;

struct stat finfo;
fstat (locfd, &finfo);

size_t read_progress = 0;

size_t write_progress = 0;

size_t fsize = finfo.st_size;
//printf (200, "file size: %d\n", fsize);

char *c_fbuffer = (char *) malloc (sendfileunit);
struct buffer_data fbuff;
fbuff.p = c_fbuffer;
fbuff.max = sendfileunit;


while (read_progress < fsize)
{  
fbuff.len = read (locfd, fbuff.p, fbuff.max);
read_progress += fbuff.len;


int rtn = poll (&pfd, 1, timeout * 1000);
if (rtn < 0) return -1;

int interim_progress = SSL_write (io.ssl, fbuff.p, fbuff.len);
if (interim_progress == -1)
	return -1;

int cpylen = interim_progress;
while (cpylen < fbuff.len)
{

int rtn = poll (&pfd, 1, timeout * 1000);
if (rtn < 0) return -1;
interim_progress = SSL_write (io.ssl, fbuff.p + cpylen, fbuff.len - cpylen);
if (interim_progress == -1)
	return -1;
cpylen += interim_progress;
} // while

} // while loop
close (locfd);
free (c_fbuffer);
return 1;



} // tls_send_file

int send_file2 (const char *path, const int fd)
{ // bm sendfile2
int locfd = open (path, O_RDONLY);
if (locfd < -1)
    return -1;

struct stat finfo;
fstat (locfd, &finfo);

size_t read_progress = 0;

size_t write_progress = 0;

size_t fsize = finfo.st_size;
//printf (200, "file size: %d\n", fsize);

char *c_fbuffer = (char *) malloc (sendfileunit);
struct buffer_data fbuff;
fbuff.p = c_fbuffer;
fbuff.max = sendfileunit;


while (read_progress < fsize)
{  
fbuff.len = read (locfd, fbuff.p, fbuff.max);
read_progress += fbuff.len;

int interim_progress = sock_1writeold (fd, fbuff.p, fbuff.len);
if (interim_progress == -1)
	return -1;

int cpylen = interim_progress;
while (cpylen < fbuff.len)
{

interim_progress = sock_1writeold (fd, fbuff.p + cpylen, fbuff.len - cpylen);
if (interim_progress == -1)
	return -1;
cpylen += interim_progress;
} // while

} // while loop
close (locfd);
free (c_fbuffer);
return 1;
} // sendfile2

int sock_setnonblock (const int fd)
{ // bm set nom block
if (fcntl(fd, F_SETFL, fcntl(fd, F_GETFL, 0) | O_NONBLOCK) == -1)
{
printf ("bad FD, calling fcntl: %d\n", fd);
perror("calling fcntl");

    return 0;
} // if
    return 1;
} // sock_setnonblock

int prepsocket (const int PORT)
{
int result = 0;
int optval = 1;
struct linger ling;
ling.l_onoff=1;
ling.l_linger=4;

int server_fd = socket(AF_INET, SOCK_STREAM, 0);

struct sockaddr_in address;
int addrlen = sizeof(address);

memset(&address, 0, sizeof (address));
address.sin_family = AF_INET;

address.sin_addr.s_addr = INADDR_ANY;

address.sin_port = htons( PORT );
//memset(address.sin_zero, 0, sizeof address.sin_zero);

result = setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &optval , sizeof(int));
if (result == -1)
	perror("error, reuse addr");

//result = setsockopt(server_fd, SOL_SOCKET, TCP_CORK,&optval , sizeof(int));
//if (result == -1)
//	logging("error, TCP-CORK", 100, 0);


//result = setsockopt(server_fd, SOL_SOCKET, SO_LINGER, &ling, sizeof(ling));
//if (result == -1)
//	logging("error, linger", 100, 0);

result = bind(server_fd, (struct sockaddr *)&address,(socklen_t) sizeof(address));
if (result == -1)
	return -1; 
	//perror("error, bind");

result = listen(server_fd, 10);
if (result == -1)
	perror("error, reuse listen");

return (server_fd);
}// end prep socket

void softclose (const int fd, struct buffer_data *inbuff)
{

shutdown (fd,  SHUT_WR);

//wait for client to close connection
int a = -1;

while (a) 
{
a=read(fd, inbuff->p, inbuff->max);

//if (a > 0)
//printf ("reading \"success\" in kill mode\n");

if(a < 0)
{
usleep (1000);
//logging ("closing, remote, not ready",1,0);
}
if(!a) 
{
printf ("Connection closed by client\n");
} // if socket non blocked
} // for wait for close bit.

close(fd);
} // softclose


int sock_1writeold (const int connfd, const char *buffer, int size)
{ // bm sock 1writeold

if (size == 0)
    size = strlen (buffer);

struct pollfd ev;
ev.fd = connfd;
ev.events = POLLOUT;

int prtn = poll (&ev, 1, timeout * 1000);
if (prtn < 1) return -1;

return write (connfd, buffer, size);

} // sock_writeold_old

int sock_readold (const int connfd, void *buffer, const int size)
{ // bm sock readold
struct pollfd ev;
ev.fd = connfd;
ev.events = POLLIN;

int r = poll (&ev, 1, timeout * 1000);
if (r == 0) return -1;

int len = read (connfd, buffer, size);
	
return len;
} // sock_readold


int sock_writeold (const int connfd, char *out, const int len)
{ // bm sock writeold
int wlen = 0;
int progress = 0;
int offset = 0;
int i;

int written = sock_1writeold (connfd, out, len);
if (written == -1)
	return -1;

if (written == len)
	return written;

progress = written;

while (progress < len)
{
wlen = 0;
for (i = offset; i < 500 - offset; ++i)
{
++wlen;
out [i] = out [wlen + progress];
if (i + offset == 500)
	break;

if (progress + wlen == len)
       break;

}// for reset op 
int totalwrite = wlen + offset;

written = sock_1writeold (connfd, out, totalwrite);
if (written == -1)
	return -1;

if (written == totalwrite)
	offset = 0;

if (written < totalwrite)
{
offset = wlen - written;

for (i = 0; i < offset; ++i)
{
out[i] = out[i + written];


} // for copy offset

} // if incomplete writee
progress += wlen;


} // while progress <
return 1; 
} // sock_buffwrite

int tls_read1 (const io_t io, char *buffer, const int max)
{ // bm tls_read1
  struct pollfd ev;
ev.fd = io.connfd;
ev.events = POLLIN;

int r = poll (&ev, 1, timeout * 1000);
if (r == 0) return -1;

int len = SSL_read (io.ssl, buffer, max);
	
return len;  
}

int sock_read1 (const io_t io, char *buffer, const int max)
{ // bm sock_read1
  struct pollfd ev;
ev.fd = io.connfd;
ev.events = POLLIN;

int r = poll (&ev, 1, timeout * 1000);
if (r == 0) return -1;

int len = read (io.connfd, buffer, max);
	
return len;  
}

int tls_read (const io_t io, char *buffer, const int max)
{ // bm tls_read
  struct pollfd ev;
ev.fd = io.connfd;
ev.events = POLLIN;

int r = poll (&ev, 1, timeout * 1000);
if (r == 0) return -1;

int len = SSL_read (io.ssl, buffer, max);
	
return len;  
}

int sock_read (const io_t io, char *buffer, const int max)
{ // bm sock_read
  struct pollfd ev;
ev.fd = io.connfd;
ev.events = POLLIN;

int r = poll (&ev, 1, timeout * 1000);
if (r == 0) return -1;

int len = read (io.connfd, buffer, max);
	
return len;  
}

int tls_write (const io_t io, const char *buffer, const int size)
{// tls_write
printf ("tls writing\n");
struct pollfd ev;
ev.fd = io.connfd;
ev.events = POLLOUT;

int prtn = poll (&ev, 1, timeout * 1000);
if (prtn < 1) return -1;

int rtn = SSL_write (io.ssl, buffer, size);
int accrue = rtn;
while (accrue < size)
{

prtn = poll (&ev, 1, timeout * 1000);
if (prtn < 1) return -1;
   
rtn = SSL_write (io.ssl, buffer + accrue, size - accrue);
if (rtn <= 0) return 1;
accrue += rtn;
}
return accrue;

} // tls write


int sock_write (const io_t io, const char *buffer, const int size)
{// bm sock write
//printf ("sock writing\n");
 struct pollfd ev;
ev.fd = io.connfd;
ev.events = POLLOUT;

int prtn = poll (&ev, 1, timeout * 1000);
if (prtn < 1) return -1;

int rtn = write (io.connfd, buffer, size);
int accrue = rtn;
while (accrue < size)
{

prtn = poll (&ev, 1, timeout * 1000);
if (prtn < 1) return -1;
   
rtn = write (io.connfd, buffer + accrue, size - accrue);
if (rtn <= 0) return 1;
accrue += rtn;
}
return accrue;
} // sock write


int sock_write1 (const io_t io, const char *buffer, const int size)
{// bm sock write1
 struct pollfd ev;
ev.fd = io.connfd;
ev.events = POLLOUT;

int prtn = poll (&ev, 1, timeout * 1000);
if (prtn < 1) return -1;

return write (io.connfd, buffer, size);
return -1;
}

int tls_write1 (const io_t io, const char *buffer, const int size)
{ // bm tls_write1
struct pollfd ev;
ev.fd = io.connfd;
ev.events = POLLOUT;

int prtn = poll (&ev, 1, timeout * 1000);
if (prtn < 1) return -1;

return SSL_write (io.ssl, buffer, size);

}

struct far_builder far_init (const buffer_t *in)
{ // bm far init
struct far_builder rtn;
rtn.base = in->p;
rtn.base_len = in->len;
rtn.req_len = in->len;
rtn.start = NULL;

return rtn;
}

int far_add (struct far_builder *b, const char *delim, const char *rep, int rep_len)
{ // bm far add
if (rep_len == -1) rep_len = strlen (rep);
//printf ("far add delim: %s, rep: %s\n", delim, rep);

int delim_len = strlen (delim);
char *p1 = memmem (b->base, b->base_len, delim, delim_len);
if (p1 == NULL) return 0;
int delim_pos = p1 - b->base;

//printf ("delimpos: %d\n", delim_pos);

struct far_entry *entry = malloc (sizeof (struct far_entry));
if (entry == NULL) return 0;
entry->rep = rep;
entry->rep_len = rep_len;
entry->delim_pos = delim_pos;
entry->next = NULL;
entry->delim = delim;
entry->delim_len = delim_len;
b->req_len -= delim_len;
b->req_len += rep_len;

if (b->start == NULL)
{
//printf ("list started\n");
b->start = entry;
return 1;
}

struct far_entry *current = b->start;
struct far_entry *last = NULL;
struct far_entry *next = NULL;
while (current != NULL)
//for (int i = 0; i < 20; ++i)
{
if (current == NULL) break;
next = current->next;
//printf ("entry delim %d  current: %d, %s\n", delim_pos, current->delim_pos, current->rep);


if (current->delim_pos > delim_pos)
{
//printf ("%d insert here %p\n", i, last);
if (last != NULL)
{
last->next = entry;
entry->next = current;
return 1;
}else{
b->start = entry;
entry->next = current;
return 1;
} // if / else NULL
} // ins here


if (next == NULL) 
{
//printf ("add to end\n");
current->next = entry;
return 1;
}
last = current;
current = current->next;
}// while

return 0;
} // far add

buffer_t far_build (struct far_builder *b)
{
//printf ("far build\n");

buffer_t rtn = init_buffer (b->req_len + 1);
memset (rtn.p, 0, rtn.max);



char *base = b->base;




struct far_entry *current = b->start;
struct far_entry *last = NULL;

int len = b->req_len;
int offset = 0;
int boffset = 0;
while (current != NULL)    
//for (int i = 0; i < 10; ++i)
{
//printf ("delim: %s rep: %s\n", current->delim, current->rep);

int copylen = current->delim_pos - boffset;

//printf ("%d, dpos: %d boffset: %d copylen: %d\n", i, current->delim_pos, boffset, copylen);
memcpy (rtn.p + offset, base + boffset, copylen);

offset += copylen;
printf ("delim lrn: %d \n", current->delim_len);
boffset += copylen + current->delim_len;
//len += copylen;

memcpy (rtn.p + offset, current->rep, current->rep_len);
offset += current->rep_len;

last = current;
current = current->next;
free (last);
if (current == NULL) break;
} // loop

int copylen = b->base_len - boffset;
memcpy (rtn.p + offset, base + boffset, copylen);
//len += copylen;
rtn.p [len] = 0;
rtn.len = len;
return rtn;
} // far builder

void far_clear (struct far_builder *b)
{
//printf ("far clear\n");
struct far_entry *ent = b->start;
//for (int i = 0; i < 10; ++i)
while (ent != NULL)
{
if (ent == NULL) break;
//printf ("%d, %s\n", ent->delim_pos, ent->rep);
//printf ("(%d) %s, %s\n", ent->delim_pos, ent->delim, ent->rep);
free (ent);
ent = ent->next;

} // while

}// far clear

void test ()
{
buffer_t temp = init_buffer (1000);
temp.len = sprintf (temp.p, "the big fat fox ran up the road!");
//printf ("%s\n", temp.p);

struct far_builder builder = far_init (&temp);
printf ("%s\n", builder.base);

far_add (&builder, "big", "small", 5);

far_add (&builder, "fox", "dog", 3);

far_add (&builder, "fat", "skinny", 6);

far_add (&builder, "up", "down", 4);

far_add (&builder, "road", "street", 6);

buffer_t page = far_build (&builder);
printf ("[%s]\n", page.p);

free (temp.p);
exit (0);
}


buffer_t JSON_decode (const buffer_t in)
{
int tail = 0;
int req_len = in.len - 2;

const int increment = 200;
int inst_max = increment;
int inst_count = 0;

struct codeco *inst = (struct codeco *)  malloc (inst_max * sizeof (struct codeco));
if (inst == NULL) killme ("no malloc");

//for (int i = 0; i < 100; ++i)
while (1)
{
char *p1 = (char *) memchr (in.p + tail,  92, in.len - tail);
if (p1 == NULL) break;

if (inst_count == inst_max)
{
inst_max += increment;
printf ("resizing instruction array\n");
inst = realloc (inst, sizeof (struct codeco) * inst_max);
if (inst == NULL) killme ("no realloc");
} //if

int d1 = p1 - in.p + 1;
char dchar = in.p[d1];


//printf ("%d pos:%d dchar: %c\n", inst_count ,d1 , dchar);

switch (dchar) {
case 'n':
inst[inst_count].dchar = dchar;
inst[inst_count].pos = d1;
req_len -= 1;

break;
case '\"':
inst[inst_count].dchar = dchar;
inst[inst_count].pos = d1;
req_len -= 1;

break;
case 92:
inst[inst_count].dchar = dchar;
inst[inst_count].pos = d1;
req_len -= 1;

break;
case 't':
inst[inst_count].dchar = dchar;
inst[inst_count].pos = d1;
req_len -= 1;

break;
case '\'':
inst[inst_count].dchar = dchar;
inst[inst_count].pos = d1;
req_len -= 1;

break;
default:
printf ("unhandled escape delimeter char: [%d]%c\n", d1, dchar);
char temp [smbuff_sz];
memset (temp, 0, default_sz);
memcpy (temp, in.p + d1 - 80,160);
printf ("samp [%s]\n", temp);
exit (0);
} // end switch

tail = d1 + 1;
++inst_count;
} //loop

//printf ("in.len: %d| req_len: %d\n", in.len, req_len);

buffer_t out = init_buffer (req_len);
memset (out.p, 0, req_len);
int i = 0;
tail = 0;
for (i = 0; i < inst_count; ++i)
{

int len = inst[i].pos - tail - 2;
memcpy (out.p + out.len, in.p + tail + 1, len);
out.len += len;
tail = inst[i].pos;

switch (inst[i].dchar) {

case 'n':
	out.p[out.len] = 10;

break;
case 92:
	out.p[out.len] = 92;

break;
case  '\"':
	out.p[out.len] = '\"';

break;
case '\'':
	out.p[out.len] = '\'';

break;
case  't':
	out.p[out.len] = 9;

break;
default: 
killme ("unhandled instruction");

}// switch

++out.len;
} // loop

///printf ("req_len: %d, ou.len: %d\n", req_len, out.len);

int len = req_len - out.len;
if (len > 0) {
memcpy (out.p + out.len, in.p + tail +1, len);
out.len += len;
printf ("added!!\n");
}else{

switch (inst[inst_count -1].dchar) {

case 'n':
	out.p[out.len] = 10;

break;
case 92:
	out.p[out.len] = 92;

break;
case  '\"':
	out.p[out.len] = '\"';

break;
case '\'':
	out.p[out.len] = '\'';

break;
case  't':
	out.p[out.len] = 9;
}
printf ("len is 0last dchar %d[%c]\n", inst[inst_count -1].dchar, inst[inst_count -1].dchar);

++out.len;
}




//printf ("len: %d ...last char: %d[%c]\n", out.len, out.p[out.len], out.p[out.len]);

/*
char temp [default_sz];
memset (temp, 0, default_sz);
memcpy (temp, in.p + tail +1, len);
printf ("adding [%s]\n", temp);
exit (0);
*/

free (inst);
return out;
} // JSON decode


buffer_t HTML_encode (const buffer_t in, const int level)
{
//const buffer_t in = *((buffer_t *) buff);

int d1 = 0;
int d2 = 0;
char lastc = 0;

char searchstr[10];
if (level == 1)
{
lastc = 0;
strcpy (searchstr, "\'\"&<>");

}else if  (level == 2) {
lastc = '\n';
strcpy (searchstr, "\"\'&<>\n");
} // if


//printf ("%s\n\n", in.p);
const int increment = 500;
int inst_max = increment;
int inst_count = 0;

struct codeco *inst = (struct codeco *)  malloc (inst_max * sizeof (struct codeco));
if (inst == NULL) killme ("no malloc");
int req_len = in.len;
printf ("in.len: %d\n", in.len);
while (1)
//for (int q = 0; q < 10; ++q)
{

char *p1 = strpbrk (in.p + d1, searchstr);
//char *p1 = memchr (in.p + d1, (int) '<', in.len);
if (p1 == NULL) break ;

if (inst_count == inst_max)
{
inst_max += increment;
printf ("resizing instruction array\n");
inst = realloc (inst, sizeof (struct codeco) * inst_max);
if (inst == NULL) killme ("no realloc");
} //if

d2 = p1 - in.p;
char dchar = in.p[d2];
switch (dchar) {
// 34 = " 39 = '
case 34:
inst [inst_count].pos = d2;
inst [inst_count].dchar = in.p[d2];
++inst_count;
req_len += 5;

break;
case 39:
inst [inst_count].pos = d2;
inst [inst_count].dchar = in.p[d2];
++inst_count;
req_len += 5;

break;
case '<':
inst [inst_count].pos = d2;
inst [inst_count].dchar = in.p[d2];
++inst_count;
req_len += 4;
break;

case '>':
inst [inst_count].pos = d2;
inst [inst_count].dchar = in.p[d2];
++inst_count;
req_len += 4;

break;
case '&':
inst [inst_count].pos = d2;
inst [inst_count].dchar = in.p[d2];
++inst_count;
req_len += 5;

break;
default :

if (lastc == dchar) {
inst [inst_count].pos = d2;
inst [inst_count].dchar = in.p[d2];
++inst_count;
req_len += 4;
}
}// switch

d1 = d2 + 1;
} // first loop

//printf ("inst_cout: %d, in_len: %d, req_len: %d\n", inst_count, in.len, req_len);
buffer_t out;
out.p = malloc (req_len);
if (out.p == NULL) killme ("malloc");
out.len = 0;
out.max = req_len;
/*
if (out->max == 0)
{
//printf ("buffer not allocated\n");
out->p = malloc (req_len);
if (out->p == NULL) killme ("no malloc");
}else if (out->max < req_len) {

//printf ("buffer too small\n");
out->p = realloc (out->p, req_len);
if (out->p == NULL) killme ("no realloc");
}
*/
int pos = -1;
for (int i = 0; i < inst_count; ++i)
{
//printf ("#d %d: pos %d: dchar: %c\n", i, inst[i].pos, inst[i].dchar);


int len = inst[i].pos - pos - 1;
//memcpy (temp, in.p + pos + 1, len);
memcpy (out.p + out.len, in.p + pos + 1, len);
out.len += len;

char dchar = inst[i].dchar;
switch (dchar) {
case 34: // = "
//&#34;
out.p[out.len] = '&';
out.p[out.len + 1] = '#';
out.p[out.len + 2] = '3';
out.p[out.len + 3] = '4';
out.p[out.len + 4] = ';';
out.len += 5;

break;
case 39: // = '
//&#39;
out.p[out.len] = '&';
out.p[out.len + 1] = '#';
out.p[out.len + 2] = '3';
out.p[out.len + 3] = '9';
out.p[out.len + 4] = ';';
out.len += 5;

break;
case '&':
out.p[out.len] = '&';
out.p[out.len + 1] = 'a';
out.p[out.len + 2] = 'm';
out.p[out.len + 3] = 'p';
out.p[out.len + 4] = ';';
out.len += 5;

break;
case '<':
out.p[out.len] = '&';
out.p[out.len + 1] = 'l';
out.p[out.len + 2] = 't';
out.p[out.len + 3] = ';';
out.len += 4;
break;

case '>':
out.p[out.len] = '&';
out.p[out.len + 1] = 'g';
out.p[out.len + 2] = 't';
out.p[out.len + 3] = ';';
out.len += 4;
break;
default:


if (lastc == dchar) {
out.p[out.len] = '<';
out.p[out.len + 1] = 'b';
out.p[out.len + 2] = 'r';
out.p[out.len + 3] = '>';
out.len += 4;

}

} // switch
pos = inst[i].pos;

} //for


int len = in.len - pos - 1;
if (len > 0) {
memcpy (out.p + out.len, in.p + pos + 1, len);
out.len += len;
}
//printf ("out-> [%.*s]\n", out->len, out->p);
free (inst);
//free (in.p);

//const buffer_t rtn = *((buffer_t *) out);
return out;

} // end HTML encode



int URL_decode (const char *in, char *out)
{ // bm URL_decode
int outlen = 0;
int tail = 0;
while (1)
{
char *p1 = strchr (in + tail, (int) '%');
if (p1 == NULL) break;
int offset = p1 - in;

//copy everything up to offset
int len = offset - tail;
memcpy (out + outlen, in + tail, len);
outlen += len;
//printf ("progress: %.*s\n", outlen, out); 

char code [4]; 
memcpy (code, in + offset + 1, 2);
code [2] = 0;
 
if (!strcmp (code, "20"))
	out [outlen] = ' ';
	++outlen;

tail = offset + 3;
} //loop

int in_len = strlen (in);
int remainder = in_len - tail;
memcpy (out + outlen, in + tail, remainder);
outlen += remainder;

out [outlen] = 0;



return outlen;
} // URL decode

void buffer_sanity (buffer_t *buff, const int req, const int inc)
{
const int diff = buff->max - buff->len;
//printf ("bufflen: %d, buffmax: %d\n", buff->len, buff->max);

if (diff < req) {
//printf ("resizing buffer\n");
buff->p = (char *) realloc (buff->p, buff->max + inc +1);
	if (buff->p == NULL) killme ("no realloc");
buff->max += inc;

} // if resize needed

} // buffer_sanity

int FAR (buffer_t *base, const char *delim, const buffer_t rep)
{
char *p1 = (char *) memmem (base->p, base->len, delim, strlen (delim));
if (p1 == NULL) return 0;

int d1 = p1 - base->p;
int d2 = d1 + strlen (delim);

int templen = base->len - d2;
int bparse = base->len - templen;

char *temp = (char *) malloc (templen);
if (temp == NULL) killme ("no malloc");
memcpy (temp, base->p + bparse, templen);

/*printf ("base: %d:  [%.*s]\n", base->len, base->len,  base->p);
printf ("rep %s\n", rep.p);
printf ("temp %d [%.*s]\n", templen, templen, temp);
*/
// do overflow protection
int req_len = d1 + templen + rep.len;
if (req_len > base->max) {
//printf ("req len: %d, base->max: %d-resizing buffer\n", req_len, base->max);
int diff = req_len - base->max;
int needed = base->max + req_len;

base->p = (char *) realloc (base->p, needed);
if (base->p == NULL) killme ("no realloc");
base->max = needed;
} // if base resize needed

memcpy (base->p + d1, rep.p, rep.len);
base->len = rep.len + d1;


//printf ("base: %d:  [%.*s]\n", base->len, base->len,  base->p);
memcpy (base->p + base->len, temp, templen);
base->len += templen;
free (temp);
//printf ("base: %d:  [%.*s]\n", base->len, base->len,  base->p);
return 1;
} // FAR

// no overflow protection
void buffcatf (struct buffer_data *buff, const char *format, ...)
{
va_list ap;
va_start (ap, format);

int formatlen = strlen (format);
//char type;
//int specify_len = 0;
//char entry [lgbuff_sz] = "";

int len = 0;
int d;
char c;
char *s;

char *delim;
int offset = 0;
int pos = 0;

while (1)
{
delim = strchr (format + offset, '%');
if (delim == NULL) break;

offset = delim - format + 1;
char dchar = format [offset];
int len = offset - pos - 1;
// do realloc check here
//add formatted string here before dchar
buffer_sanity (buff, len, len);
memcpy (buff->p + buff->len, format + pos, len);
buff->len += len;

switch (dchar)
//if (dchar == 's')
{
case 's':
s = va_arg(ap, char *);
int slen = strlen (s);
buffer_sanity (buff, slen, slen);
buff->len += sprintf (buff->p + buff->len, "%s", s);
break;

case 'd':
d = va_arg(ap, int);
char temp [100];
int tlen = sprintf (temp, "%d", d);
buffer_sanity (buff, tlen, tlen);

buff->len += sprintf (buff->p + buff->len, "%d", d);

//strcat (buff->p, temp);
//buff->len += tlen;

//buff->len +=break;
} // switch
pos = offset + 1;

} // while

len = formatlen - pos;
buffer_sanity (buff, len, len);
memcpy (buff->p + buff->len, format + pos, len);
buff->len += len;
buff->p[buff->len] = 0;


/*

if (type == 'c')
{
c = (char) va_arg(ap, int);
entry [len] = c;
++len;
entry [len] = 0;
++fplace;    
} // if c
*/

} // buffcatf


int strsearch (const char *main, const char *minor, const int start)
{
char *p = strstr (main + start, minor);
if (p == NULL) return -1;

return (p - main + strlen (minor));

} // end search

int midstr(const char *major, char *minor, int start, const int end)
{
int len = end - start;

memcpy (minor, major + start, len);
minor [len] = 0;
return len;

} // end midstr

int getlast (const char *str, const int next, int end)
{
char *p = (char *) memrchr (str, next, end);
if (p == NULL) return -1;

return (p - str);
}

int getnext (const char *str, const int next, const int start, int end)
{

char *p = (char *) memchr (str + start, next, end - start);
if (p == NULL) return -1;

return (p - str);
} // getnext

// unfinished and untested

int extract_SC (const buffer_t *src, char *ex, const int exmax, const char *d1, const char d2)
{ // bm buffer_data extract string-char

int d1len = strlen (d1);

char *p1 = (char *) memmem (src->p, src->len, d1, d1len);
if (p1 == NULL) return 0;

int o1 = p1 - src->p + d1len;

char *p2 = (char *) memchr (src->p + o1 + 1, (int) d2, src->len);
if (p2 == NULL) return 0;
int o2 = p2 - src->p;

int exlen = o2 - o1;
if (exlen > exmax) return 0;

memcpy (ex, src->p + o1, exlen);

ex[exlen] = 0;

return (exlen);
} // bm extract char char


int extract_CC (const buffer_t src, char *ex, const int exmax, const char d1, const char d2)
{ // bm buffer_data extract char-char
char *p1 = (char *) memchr (src.p, (int) d1, src.len);
if (p1 == NULL) return 0;

int o1 = p1 - src.p;


char *p2 = (char *) memchr (src.p + o1 + 1, (int) d2, src.len);
if (p2 == NULL) return 0;
int o2 = p2 - src.p;

int exlen = o2 - o1 - 1;
if (exlen > exmax) return 0;

memcpy (ex, src.p + o1 + 1, exlen);

ex[exlen] = 0;

return (exlen);
} // bm extract char char

struct buffer_data init_buffer (const int sz)
{ // bm init_buffer
struct buffer_data buffer; 
if (sz == 0)
{memset (&buffer, 0, sizeof (struct buffer_data)); return buffer;}

buffer.p = (char *) malloc (sz + 1);
if (buffer.p == NULL) killme ("error malloc");

buffer.max = sz;
buffer.len = 0;
buffer.procint = 0;

return buffer;
   
} // init buffer



/*
timer_c::timer_c ()
{ // bm timer constructor
clock_gettime (CLOCK_MONOTONIC, &start);
}// timer constructor

void timer_c::reset ()
{ // bm timer reset
clock_gettime (CLOCK_MONOTONIC, &start);
} // timer

void timer_c::stop () 
{ //bm timer end
clock_gettime (CLOCK_MONOTONIC, &end);
} //timer end

void timer_c::result (int *sec, int *ms)
{ // bm timer get diff
struct timespec temp;
//if ((end.tv_nsec-start.tv_nsec)<0)
if (end.tv_nsec < start.tv_nsec)
{ 
temp.tv_sec = end.tv_sec-start.tv_sec-1;
temp.tv_nsec = 1000000000+end.tv_nsec-start.tv_nsec; 
} else { 
temp.tv_sec = end.tv_sec-start.tv_sec;
temp.tv_nsec = end.tv_nsec-start.tv_nsec;
} // if

int millisec = temp.tv_nsec / 1000000;

*sec = temp.tv_sec;
*ms = millisec;

}// timer get diff
*/

void err_ctl (const int rslt, const char *msg) {
if (rslt < 0) {
printf ("%s\n", msg);
exit(0);
}
} //err_ctl


void killme (const char *msg) {
printf ("%s\n", msg);
exit(0);
} //killme


char *parse_line (char *dest, const char *src)
{ // bm parse_line 
char *rtn = (char *) strchr (src, 10);
if (rtn == NULL) return NULL;
int len = rtn - src;
memcpy (dest, src, len);
dest [len] = 0;
return rtn +1;
 
} // parse_line

int split_value (char *src, const char d, char *value)
{ // bm split_value
char *p1 = strchr (src, (int) d);
if (p1 == NULL) return 0;

int len = strlen (src);


int d1 = p1 - src;
//memset (value, 0, valsz);
memcpy (value, src + d1 +1, len - d1 -1);
value [len - d1 -1] = 0;
src[d1] = 0;



return 1;
} // parse value


int trim (char *totrim)
{ // bm void trim
int len = strlen (totrim);

//trim beginning
int count = 0;
for (int i = 0; i !=len; ++i)
{
int trigger = 0;
switch (totrim[i])
{
case 10: 
++count;
break;
case 13:
++count;
break;
case 32:
++count;
break;
  
default:
goto fin1;
  
} // switch
} // for
    
fin1:;
if (count)
{
memmove (totrim, totrim + count, len - count);
totrim [len - count] = 0;  // works partially
//memset (totrim + len - count, 0, count);
len -= count;
///printf ("trimming %d\n", count);
}

int endtrim = 0;
// trim end
for (int i = len -1; i > 0; --i)
{
// printf ("i: %d (%c - <%d>)\n", i, totrim[i], totrim[i]);
switch (totrim[i])
{
case 10: 
totrim[i] = 0;
++endtrim;
break;
case 13:
totrim[i] = 0;
++endtrim;
break;
case 32:
totrim[i] = 0;
++endtrim;
break;

default:
i = 0;
} // switch
} // for

len -= endtrim;
return len;
} // trim

void save_buffer (const struct buffer_data b, const char *path)
{ // bm save_buffer
int localfd = open (path, O_WRONLY | O_TRUNC| O_CREAT, S_IRUSR | S_IWUSR);
if (localfd < 0)
	return ;

write (localfd, b.p, b.len);


close (localfd);
}// save_page









