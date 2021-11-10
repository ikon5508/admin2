#include "admin.h"

#include <openssl/bio.h>
#include <openssl/err.h>
#include <openssl/pem.h>
#include <openssl/ssl.h>

struct ssl_client
{
  int fd;

  SSL *ssl;

  BIO *rbio; /* SSL reads from, we write to. */
  BIO *wbio; /* SSL writes to, we read from. */

  /* Bytes waiting to be written to socket. This is data that has been generated
   * by the SSL object, either due to encryption of user input, or, writes
   * requires due to peer-requested SSL renegotiation. */
  char* write_buf;
  size_t write_len;

  /* Bytes waiting to be encrypted by the SSL object. */
  char* encrypt_buf;
  size_t encrypt_len;

  /* Method to invoke when unencrypted bytes are available. */
  void (*io_on_read)(char *buf, size_t len);
} client;

/* This enum contols whether the SSL connection needs to initiate the SSL
 * handshake. */
enum ssl_mode { SSLMODE_SERVER, SSLMODE_CLIENT };

enum sslstatus { SSLSTATUS_OK, SSLSTATUS_WANT_IO, SSLSTATUS_FAIL};

SSL_CTX *ctx;

void print_unencrypted_data(char *buf, size_t len) {
  printf("%.*s", (int)len, buf);
} // print unecrypted data

void queue_encrypted_bytes(const char *buf, size_t len)
{
  client.write_buf = (char*)realloc(client.write_buf, client.write_len + len);
  memcpy(client.write_buf+client.write_len, buf, len);
  client.write_len += len;
} // queue encrypted bytes

void softclose (const int fd, struct buffer_data *inbuff)
{
shutdown (fd,  SHUT_WR);

//wait for client to close connection
int a = -1;

while (a) 
{
a=read(fd, inbuff->p, inbuff->max);

//if (a > 0)
//loggingf ("reading \"success\" in kill mode\n");

if(a < 0)
{
usleep (1000);
//logging ("closing, remote, not ready",1,0);
}
if(!a) 
{
loggingf ("Connection closed by client\n");
} // if socket non blocked
} // for wait for close bit.

close(fd);
} // softclose

int send_err (const int fd, const int code)
{

if (code == 410)
	sock_write (fd, "HTTP/1.1 410 GONE", 0);


if (code == 500)
	sock_write (fd, "HTTP/1.1 500 ERROR", 0);

return 1;
} // send_err

int send_txt (const int fd, const char *txt, int len)
{
if (!len)
    len = strlen (txt);

char outbuffer [maxbuffer];
struct buffer_data outbuff;
outbuff.p = outbuffer;
outbuff.len = 0;
outbuff.max = maxbuffer;

outbuff.len = sprintf (outbuff.p, "%s%s%s%s%d\n\n%.*s", hthead, conttxt, connclose, contlen, len, len, txt);

sock_buffwrite (fd, &outbuff);

return 1;
} // send_txt

int serv_file (const struct args_data args, const struct request_data request, const int size)
{
int dot = getlast (request.path.p, (int) '.', request.path.len);
char outbuffer [string_sz];
struct buffer_data outbuff;
outbuff.p = outbuffer;
outbuff.len = 0;
outbuff.max = string_sz;

char mime_ext[10] = "";
const char *mime_txt;

strncpy (mime_ext, request.path.p + dot, request.path.len - dot);

int i = 0;

if (strcmp(mime_ext, ".txt") == 0)
{i=1;  mime_txt = conttxt;}


if (strcmp(mime_ext, ".htm") == 0)
{i=1;  mime_txt = conthtml;}


if (strcmp(mime_ext, ".html") == 0)
{i=1;  mime_txt = conthtml;}

if (strcmp(mime_ext, ".js") == 0)
{i=1;  mime_txt = contjava;}

if (strcmp(mime_ext, ".jpg") == 0)
{i=1;  mime_txt = contjpg;}

if (strcmp(mime_ext, ".jpeg") == 0)
{i=1;  mime_txt = contjpg;}

if (strcmp(mime_ext, ".css") == 0)
{i=1;  mime_txt = contcss;}

if (strcmp(mime_ext, ".ico") == 0)
{i=1;  mime_txt = conticon;}

if (i == 0)
{mime_txt = conttxt;}

//loggingf ("%s\n", mime_txt);

//struct stat finfo;
//stat (request.fullpath.p, &finfo);

outbuff.len = sprintf (outbuff.p, "%s%s%s%d\n\n", hthead, mime_txt, contlen, size);

loggingf ("%d bytes: %s", size, mime_txt);

sock_writeold (request.fd, outbuff.p, outbuff.len);

sendfile (request.fullpath.p, request.fd);
return 1;

} // serv_file`

int serv_dir (const struct args_data args, const struct request_data request)
{
DIR *dp;
struct dirent *ep;

char outb [maxbuffer];
struct buffer_data out;
out.p = outb;
out.len = 0;
out.max = maxbuffer;


 buffcatf (&out, "<!DOCTYPE html>\n<html>\n<head>\n");

 buffcatf (&out,"<style>\n");
 buffcatf (&out,"body\n{\ntext-align:left;\nmargin-left:70px;\nbackground-color:aqua;\nfont-size:52px;\n}\n");
 buffcatf (&out, "a:link\n{\ncolor:midnightblue;\ntext-decoration:none;\n}\n");
 buffcatf (&out, "</style>\n</head>\n<body>\n");

dp = opendir (request.fullpath.p);
if (dp == NULL)
	{send_txt (request.fd, "OOPS", 0); return -1;}

buffcatf (&out, "uri: %s<br>\n path: %s<br>\n fpath: %s<br>\n", request.uri.p, request.path.p, request.fullpath.p);

buffcatf (&out, "<form enctype=\"multipart/form-data\" action=\"%s\" method=\"post\">\n", request.uri.p);
buffcatf (&out, "<input type=\"file\" name=\"myFile\">\n");
buffcatf (&out, "<input type=\"submit\" value=\"upload\">\n</form>\n");

while ((ep = readdir (dp)))
{

if (ep->d_name[0] == '.')
	continue;
	
// 4 is dir 8 is file
//loggingf ("dir: %s\n", ep->d_name);

if (ep->d_type == 4)
buffcatf (&out, "<a href=\"%s/%s\">%s/</a><br>\n", request.uri.p, ep->d_name, ep->d_name);



//stradd (outbuff.p, ep->d_name, &outbuff.len);

} // while


 closedir (dp);


dp = opendir (request.fullpath.p);
while ((ep = readdir (dp)))
{

if (ep->d_name[0] == '.')
	continue;
	
if (ep->d_type == 8)
buffcatf (&out, "<a href=\"/file%s/%s\">%s</a><br>\n", request.path.p, ep->d_name, ep->d_name);



//stradd (outbuff.p, ep->d_name, &outbuff.len);

} // while



buffcatf (&out, "</body>\n</html>");


loggingf ("%d: bytes directory info\n", out.len);
struct string_data head;

head.len = sprintf (head.p, "%s%s%s%d\n\n", hthead, conthtml, contlen, out.len);
sock_writeold (request.fd, head.p, head.len);
sock_buffwrite (request.fd, &out);

closedir (dp);

return 1;
} // serv_dir


int get_file (const struct args_data args, const struct request_data request)
{

struct stat finfo;
if (stat(request.fullpath.p, &finfo) == -1)
    {send_txt (request.fd, "bad resource", 0); return (-1);}

if (S_ISDIR(finfo.st_mode))
    return (serv_dir (args, request));

if (S_ISREG(finfo.st_mode)) // is file
    return (serv_file (args, request, finfo.st_size));

return 0;

} //get_file

struct request_data process_request (const int fd, const struct args_data args, const struct buffer_data inbuff)
{
struct request_data request;
memset (&request, 0, sizeof (request));

request.method = inbuff.p [0];
request.fd = fd;

int d1 = getnext (inbuff.p, 32, 0, inbuff.len);
++d1;
int d2 = getnext (inbuff.p, 32, d1, inbuff.len);

//request.uri.len = midstr (inbuff.p, request.uri.p, d1, d2);

strncpy (request.uri.p, inbuff.p + d1, d2 - d1);
request.uri.len = d2 - d1;

if (!strcmp(request.uri.p, "/favicon.ico"))
		{request.mode = favicon; return request; }

if (!strcmp(request.uri.p, "/config"))
		{request.mode = config; return request; }

//d1 = strsearch (inbuff.p, "Connection: keep-alive", d2, inbuff.len);
//if (d1 > 0)
//request.keepalive = 1;

d1 = strsearch (request.uri.p, "/edit", 0, 6);
if (d1 > 0)
{
request.mode = edit;
memcpy (request.path.p, request.uri.p + d1, request.uri.len - d1);
request.path.len = request.uri.len - d1 - 1;
} // if edit

d1 = strsearch (request.uri.p, "/file", 0, 6);
if (d1 > 0)
{
request.mode = file;
strncpy (request.path.p, request.uri.p + d1, request.uri.len - d1);
request.path.len = request.uri.len - d1;
} // if file

request.fullpath.len = sprintf (request.fullpath.p, "%s%s", args.base_path.p, request.path.p);

if (request.method == 'P')
{
d1 = strsearch (inbuff.p, "boundary=", 0, inbuff.len);
d2 = getnext (inbuff.p, 10, d1, d1 + 100);

strncpy (request.boundary, inbuff.p + d1, d2 - d1 - 1);
request.boundlen = d2 - d1 - 1; //strlen (request.boundary);
request.procint = d2;
} // if post get boundary

return (request);
} // process_request

int get_edit_file (const struct args_data args, const struct request_data request)
{

char outbuffer [maxbuffer];
struct buffer_data outbuff;
outbuff.p = outbuffer;
outbuff.max = (maxbuffer);
outbuff.len = 0;

char ebuffer [maxbuffer];
struct buffer_data editorbuffer;
editorbuffer.p = ebuffer;
editorbuffer.max = maxbuffer;

char fbuffer [maxbuffer];
struct buffer_data filebuffer;
filebuffer.p = fbuffer;
filebuffer.max = maxbuffer;


int editorfd = open (args.editor_path.p, O_RDONLY);
if (editorfd < 0)
    {send_txt (request.fd, "bad editor", 0); return -1;}

int filefd = open (request.fullpath.p, O_RDONLY);
if (filefd < 0)
    {send_txt (request.fd, "bad file name", 0); return -1;}

editorbuffer.len = read (editorfd, editorbuffer.p, editorbuffer.max);
filebuffer.len = read (filefd, filebuffer.p, filebuffer.max);

int assets = countassets (editorbuffer);

close (editorfd);
close (filefd);

editorbuffer.procint = strsearch (editorbuffer.p, "DELIMETER", 0, editorbuffer.len);
if (editorbuffer.procint < 0)
    {send_txt (request.fd, "failed to find DELIMETER", 0); return -1;}

for (int i = 0; i < (editorbuffer.procint - 9); ++i)
{
outbuff.p [outbuff.len] = editorbuffer.p [i];
++outbuff.len;
} // for

for (int i = 0; i < filebuffer.len; ++i)
{
if (filebuffer.p [i] == '<')
{
outbuff.p [outbuff.len] = '&';
outbuff.p [outbuff.len + 1] = 'l';
outbuff.p [outbuff.len + 2] = 't';
outbuff.p [outbuff.len + 3] = ';';
outbuff.len += 4;
continue;
} // if

if (filebuffer.p [i] == '>')
{
outbuff.p [outbuff.len] = '&';
outbuff.p [outbuff.len + 1] = 'g';
outbuff.p [outbuff.len + 2] = 't';
outbuff.p [outbuff.len + 3] = ';';
outbuff.len += 4;
continue;
} // if

outbuff.p [outbuff.len] = filebuffer.p [i];
++outbuff.len;
} // for

for (int i = (editorbuffer.procint); i < editorbuffer.len; ++i)
{
outbuff.p [outbuff.len] = editorbuffer.p [i];
++outbuff.len;
}

struct string_data head;
head.len = sprintf (head.p, "%s%s%s%d\n\n", hthead, conthtml, contlen, outbuff.len);

loggingf ("edit file served: %d bytes\n", outbuff.len);

sock_writeold (request.fd, head.p, head.len);
sock_buffwrite (request.fd, &outbuff);
return assets + 1;

} // get_edit_file

int post_edit (const struct buffer_data mainbuff, const struct request_data request)
{
int startdata = -1;
int enddata = -1;
int fbound =-1, rbound=-1;
const int saveold = 0;

long progress = 0;

char fdata [string_sz];
struct buffer_data filedata;
filedata.p = fdata;
filedata.max = string_sz;
filedata.len = 0;

char bd [string_sz];
struct buffer_data inbuff;
inbuff.p = bd;
inbuff.max = string_sz;

char newfname [string_sz];
if (saveold) 
{	
strcpy (newfname, request.fullpath.p);
strcat (newfname, ".old");
rename (request.fullpath.p, newfname);
} // save old file

int localfd = open (request.fullpath.p, O_WRONLY | O_TRUNC| O_CREAT, S_IRUSR | S_IWUSR);
if (localfd < 0)  {send_txt(request.fd,"1, error opening newfile",0); return -1; }

// search for first boundary in initial xmission
fbound = strsearch (mainbuff.p, request.boundary, request.procint, mainbuff.len);
if (fbound > - 1)
{
// get past name data, since only one object is sent
startdata = getnext (mainbuff.p, 10, fbound, mainbuff.len);
startdata = getnext (mainbuff.p, 10, startdata + 1, mainbuff.len);

while (mainbuff.p [startdata] == 10 || mainbuff.p [startdata] == 13 || mainbuff.p [startdata] == '.')
    ++startdata;

rbound = strsearch (mainbuff.p, request.boundary, fbound + 1, mainbuff.len);
if (rbound > - 1)
{
enddata = rbound - request.boundlen;
while (mainbuff.p [enddata] == 10 || mainbuff.p [enddata] == 13 || mainbuff.p [enddata] == '-')
    --enddata;
++enddata;

//for (int i = startdata; i < enddata; ++i)
//{
//filedata.p [filedata.len] = mainbuff.p [i];
//++filedata.len;
//} // for

memcpy (filedata.p, mainbuff.p + startdata, enddata - startdata);
filedata.len = enddata - startdata;

filedata.procint = write (localfd, filedata.p, filedata.len);
if (filedata.procint != filedata.len)  {send_txt(request.fd,"error writing single reciever",0); return -1; }
close (localfd);
rename (newfname, request.fullpath.p);
send_txt (request.fd, "File recieved, single reciever", 0);
return 1;
} // if rbound

enddata = mainbuff.len;
//for (int i = startdata; i < enddata; ++i)
//{
//filedata.p [filedata.len] = mainbuff.p [i];
//++filedata.len;
//} // for

memcpy (filedata.p, mainbuff.p + startdata, enddata - startdata);
filedata.len = enddata - startdata;
filedata.procint = write (localfd, filedata.p, filedata.len);
if (filedata.procint != filedata.len)  {send_txt(request.fd,"1 error writing multipart reciever",0); return -1; }

inbuff.len = sock_read (request.fd, inbuff.p, inbuff.max);
if (inbuff.len == -1) {send_txt(request.fd,"1, reading timout",0); return -1; }
}// if fbound
// done inumerating start and enddata in mainbuffer
// written to file accoring to bound vars

// if filedata not started yet, get new data, cycle through favicon if necessary
if (startdata == -1)
{
int cnt = 0;
while (fbound == -1)
{
inbuff.len = sock_read (request.fd, inbuff.p, inbuff.max);
if (inbuff.len == -1) {send_txt(request.fd,"1, reading timout",0); return -1; }
fbound = strsearch (inbuff.p, request.boundary, 0, inbuff.len);
++cnt;
if (cnt == 5) {send_txt(request.fd,"1, 1st boundary not found",0); return -1; }
} // while fbound - 1

startdata = getnext (inbuff.p, 10, fbound, inbuff.len);
startdata = getnext (inbuff.p, 10, startdata + 1, inbuff.len);
while (inbuff.p [startdata] == 10 || inbuff.p [startdata] == 13 || inbuff.p [startdata] == '.')
    ++startdata;
}else{
startdata = 0;
}//if filedata not started yet, start it,otherwise set start to 0;

// while loop until rbound > 0
// read until boundary is found, or socket times out.
// place all new contents in file - boundary

while (rbound == -1)
{
rbound = strsearch (inbuff.p, request.boundary, inbuff.len - request.boundlen - 10, inbuff.len);

if (rbound > 0)
{
enddata = inbuff.len - request.boundlen - 2;
while (inbuff.p [enddata] == 10 || inbuff.p [enddata] == 13 || inbuff.p [enddata] == '-')
    --enddata;
++enddata;

memcpy (filedata.p, inbuff.p + startdata, enddata - startdata);
filedata.len = enddata - startdata;

filedata.procint = write (localfd, filedata.p, filedata.len);
if (filedata.procint != filedata.len)  {send_txt(request.fd,"error writing multipart-reciever",0); return -1; }

send_txt (request.fd, "file recieved, multipart reciever", 0);

progress += filedata.len;
loggingf ("%d bytes: file recieved and allocated properly\n", progress);
return 1;

} // if < rbound

// read here after memcpy / direct write

if (startdata == 0)
{
inbuff.procint = write (localfd, inbuff.p, inbuff.len);
if (inbuff.procint != inbuff.len) {send_txt(request.fd,"error writing multipart-reciever",0); return -1; }

progress += inbuff.len;
}else{
memcpy (filedata.p, inbuff.p + startdata, inbuff.len - startdata);
filedata.len = inbuff.len - startdata;
progress += filedata.len;


filedata.procint = write (localfd, filedata.p, filedata.len);
if (filedata.procint != filedata.len)  {send_txt(request.fd,"error writing multipart-reciever",0); return -1; }
}/// if


inbuff.len = sock_read (request.fd, inbuff.p, inbuff.max);
if (inbuff.len == -1)
{
loggingf ("split boundary!!!\n");

//struct stat finfo;
//fstat (localfd, &finfo);

//ftruncate (localfd, finfo.st_size - request.boundlen - 5);

// added additional 5 here, to accomodate the -----...seems imprecise
// if fails again e.g. on chrome based browsers
// use lseek reset fpos, read last 100 
//
// redermine and calculate truncate len
close (localfd);
send_txt (request.fd, "CHECK DATA-split boundary file recieved, multi-part reciever CHECK DATA",0);

return 1;
} // if timeout finish

startdata = 0;
} // while rbound

close (localfd);

//send_txt (request.fd, "file recieved, multi-part reciever",0);
return 1;

} //post edit

int get_config (const struct args_data args, const struct request_data request)
{
char outb [maxbuffer];
struct buffer_data out;
out.p = outb;
out.len = 0;
out.max = maxbuffer;

loggingf ("get config\n");

buffcatf (&out, "<!DOCTYPE html>\n<html>\n<head>\n");

buffcatf (&out,"<style>\n");
buffcatf (&out,"body\n{\ntext-align:left;\nmargin-left:70px;\nbackground-color:aqua;\nfont-size:52px;\n}\n");
buffcatf (&out, "a:link\n{\ncolor:midnightblue;\ntext-decoration:none;\n}\n");
buffcatf (&out, "</style>\n</head>\n<body>\n");


buffcatf (&out, "uri: %s<br>\n path: %s<br>\n fpath: %s<br>\n", request.uri.p, request.path.p, request.fullpath.p);
buffcatf (&out, "<img src=\"/file/pic.jpg\">");
struct string_data head;


head.len = sprintf (head.p, "%s%s%s%s%d\n\n", hthead, connka, conthtml, contlen, out.len);
sock_write (request.fd, head.p, head.len);

sock_write (request.fd, out.p, out.len);
return 2;
} // get config

int servico (const int fd)
{
   struct string_data outbuff;
   
struct stat finfo;
stat ("favicon.ico", &finfo);

outbuff.len = sprintf (outbuff.p, "%s%s%s%ld\n\n", hthead,  conticon, contlen, finfo.st_size);
sock_writeold (fd, outbuff.p, outbuff.len);

sendfile ("favicon.ico", fd);
return 1;
} // servico

int post_file (const struct buffer_data mainbuff, const struct request_data request)
{
int startdata = -1;
int enddata = -1;
int fbound =-1, rbound=-1;
const int saveold = 0;
long progress = 0;

char fdata [string_sz];
struct buffer_data filedata;
filedata.p = fdata;
filedata.max = string_sz;
filedata.len = 0;

char bd [string_sz];
struct buffer_data inbuff;
inbuff.p = bd;
inbuff.max = string_sz;

char fname [100];
char fullpath [string_sz];
memset (fname, 0, 100);


int localfd;

// search for first boundary in initial xmission
fbound = strsearch (mainbuff.p, request.boundary, request.procint, mainbuff.len);
if (fbound > - 1)
{
startdata = getnext (mainbuff.p, 10, fbound, mainbuff.len);
startdata = getnext (mainbuff.p, 10, startdata + 1, mainbuff.len);
startdata = getnext (mainbuff.p, 10, startdata + 1, mainbuff.len);
startdata = getnext (mainbuff.p, 10, startdata + 1, mainbuff.len);

while (mainbuff.p [startdata] == 10 || mainbuff.p [startdata] == 13 || mainbuff.p [startdata] == '.')
    ++startdata;

int d1 = strsearch (mainbuff.p, "filename=\"", fbound, mainbuff.len);
int d2 = getnext (mainbuff.p, (int) '\"', d1, mainbuff.len);
memcpy (fname, mainbuff.p + d1, d2 - d1);
sprintf (fullpath, "%s/%s", request.fullpath.p, fname);
localfd = open (fullpath, O_WRONLY | O_TRUNC| O_CREAT, S_IRUSR | S_IWUSR);
if (localfd < 0)  {send_txt(request.fd,"1, error opening newfile",0); return -1; }

rbound = strsearch (mainbuff.p, request.boundary, fbound + 1, mainbuff.len);
if (rbound > - 1)
{
enddata = rbound - request.boundlen;
while (mainbuff.p [enddata] == 10 || mainbuff.p [enddata] == 13 || mainbuff.p [enddata] == '-')
    --enddata;
++enddata;

memcpy (filedata.p, mainbuff.p + startdata, enddata - startdata);
filedata.len = enddata - startdata;

filedata.procint = write (localfd, filedata.p, filedata.len);
if (filedata.procint != filedata.len)  {send_txt(request.fd,"error writing single reciever",0); return -1; }
close (localfd);
send_txt (request.fd, "File recieved, single reciever", 0);
return 1;
} // if rbound

enddata = mainbuff.len;

memcpy (filedata.p, mainbuff.p + startdata, enddata - startdata);
filedata.len = enddata - startdata;
filedata.procint = write (localfd, filedata.p, filedata.len);
if (filedata.procint != filedata.len)  {send_txt(request.fd,"1 error writing multipart reciever",0); return -1; }

inbuff.len = sock_read (request.fd, inbuff.p, inbuff.max);
if (inbuff.len == -1) {send_txt(request.fd,"1, reading timout",0); return -1; }
}// if fbound
// done inumerating start and enddata in mainbuffer
// written to file accoring to bound vars

// if filedata not started yet, get new data, cycle through favicon if necessary
if (startdata == -1)
{
int cnt = 0;
while (fbound == -1)
{
inbuff.len = sock_read (request.fd, inbuff.p, inbuff.max);
if (inbuff.len == -1) {send_txt(request.fd,"1, reading timout",0); return -1; }
fbound = strsearch (inbuff.p, request.boundary, 0, inbuff.len);
++cnt;
if (cnt == 5) {send_txt(request.fd,"1, 1st boundary not found",0); return -1; }
} // while fbound - 1

startdata = getnext (inbuff.p, 10, fbound, inbuff.len);
startdata = getnext (inbuff.p, 10, startdata + 1, inbuff.len);
startdata = getnext (inbuff.p, 10, startdata + 1, inbuff.len);
startdata = getnext (inbuff.p, 10, startdata + 1, inbuff.len);

while (inbuff.p [startdata] == 10 || inbuff.p [startdata] == 13 || inbuff.p [startdata] == '.')
    ++startdata;

int d1 = strsearch (inbuff.p, "filename=\"", fbound, inbuff.len);
int d2 = getnext (inbuff.p, (int) '\"', d1, inbuff.len);
memcpy (fname, inbuff.p + d1, d2 - d1);
sprintf (fullpath, "%s/%s", request.fullpath.p, fname);
localfd = open (fullpath, O_WRONLY | O_TRUNC| O_CREAT, S_IRUSR | S_IWUSR);
if (localfd < 0)  {send_txt(request.fd,"1, error opening newfile",0); return -1; }
    
}else{
startdata = 0;
}//if filedata not started yet, start it,otherwise set start to 0;

// while loop until rbound > 0
// read until boundary is found, or socket times out.
// place all new contents in file - boundary
while (rbound == -1)
{
rbound = strsearch (inbuff.p, request.boundary, inbuff.len - request.boundlen - 10, inbuff.len);

if (rbound > 0)
{
enddata = inbuff.len - request.boundlen - 2;
while (inbuff.p [enddata] == 10 || inbuff.p [enddata] == 13 || inbuff.p [enddata] == '-')
    --enddata;
++enddata;

memcpy (filedata.p, inbuff.p + startdata, enddata - startdata);
filedata.len = enddata - startdata;

filedata.procint = write (localfd, filedata.p, filedata.len);
if (filedata.procint != filedata.len)  {send_txt(request.fd,"error writing multipart-reciever",0); return -1; }

send_txt (request.fd, "file recieved, multipart reciever", 0);

progress += filedata.len;
loggingf ("%d bytes: file recieved and allocated properly\n", progress);
return 1;

} // if < rbound

// read here after memcpy / direct write

if (startdata == 0)
{
inbuff.procint = write (localfd, inbuff.p, inbuff.len);
if (inbuff.procint != inbuff.len) {send_txt(request.fd,"error writing multipart-reciever",0); return -1; }

progress += inbuff.len;
}else{
memcpy (filedata.p, inbuff.p + startdata, inbuff.len - startdata);
filedata.len = inbuff.len - startdata;
progress += filedata.len;


filedata.procint = write (localfd, filedata.p, filedata.len);
if (filedata.procint != filedata.len)  {send_txt(request.fd,"error writing multipart-reciever",0); return -1; }
}/// if

inbuff.len = sock_read (request.fd, inbuff.p, inbuff.max);
if (inbuff.len == -1)
{
loggingf ("split boundary!!!\n");

//struct stat finfo;
//fstat (localfd, &finfo);

//ftruncate (localfd, finfo.st_size - request.boundlen - 5);

// added additional 5 here, to accomodate the -----...seems imprecise
// if fails again e.g. on chrome based browsers
// use lseek reset fpos, read last 100 
//
// redermine and calculate truncate len
close (localfd);
send_txt (request.fd, "CHECK DATA-split boundary file recieved, multi-part reciever CHECK DATA",0);

return 1;
} // if timeout finish



startdata = 0;
} // while rbound

close (localfd);

//send_txt (request.fd, "file recieved, multi-part reciever",0);
return 1;

} //post file


void send_unencrypted_bytes(const char *buf, size_t len)
{
  client.encrypt_buf = (char*)realloc(client.encrypt_buf, client.encrypt_len + len);
  memcpy(client.encrypt_buf+client.encrypt_len, buf, len);
  client.encrypt_len += len;
} // send unencrypted bytes

void ssl_init(const char * certfile, const char* keyfile)
{
 
      	printf("initialising SSL\n");

  SSL_library_init();
  OpenSSL_add_all_algorithms();
  SSL_load_error_strings();
  ERR_load_BIO_strings();
  ERR_load_crypto_strings();

  ctx = SSL_CTX_new(SSLv23_method());
  if (!ctx)
  { loggingf ("SSL_CTX_new()"); exit (0);}


  if (certfile && keyfile) {
    if (SSL_CTX_use_certificate_file(ctx, certfile,  SSL_FILETYPE_PEM) != 1)
    { loggingf ("SSL_CTX_use_certificate_file failed"); exit (0);}

    if (SSL_CTX_use_PrivateKey_file(ctx, keyfile, SSL_FILETYPE_PEM) != 1)
    {loggingf ("SSL_CTX_use_PrivateKey_file failed"); exit (0);}

    if (SSL_CTX_check_private_key(ctx) != 1)
    {loggingf ("SSL_CTX_check_private_key failed"); exit (0);}
    else
      loggingf ("certificate and private key loaded and verified\n");
  }

  SSL_CTX_set_options(ctx, SSL_OP_ALL|SSL_OP_NO_SSLv2|SSL_OP_NO_SSLv3);
  
}  /// ssl_init

void ssl_client_init(struct ssl_client *p,
                     int fd,
                     enum ssl_mode mode)
{
  memset(p, 0, sizeof(struct ssl_client));

  p->fd = fd;

  p->rbio = BIO_new(BIO_s_mem());
  p->wbio = BIO_new(BIO_s_mem());
  p->ssl = SSL_new(ctx);

  if (mode == SSLMODE_SERVER)
    SSL_set_accept_state(p->ssl);  
  else if (mode == SSLMODE_CLIENT)
    SSL_set_connect_state(p->ssl); 

  SSL_set_bio(p->ssl, p->rbio, p->wbio);

  p->io_on_read = print_unencrypted_data;

  } // ssl_client_init

static enum sslstatus get_sslstatus(SSL* ssl, int n)
{
  switch (SSL_get_error(ssl, n))
  {
    case SSL_ERROR_NONE:
      return SSLSTATUS_OK;
    case SSL_ERROR_WANT_WRITE:
    case SSL_ERROR_WANT_READ:
      return SSLSTATUS_WANT_IO;
    case SSL_ERROR_ZERO_RETURN:
    case SSL_ERROR_SYSCALL:
    default:
      return SSLSTATUS_FAIL;
  }
} // get_sslstatus

enum sslstatus do_ssl_handshake()
{
  char buf[string_sz];
  enum sslstatus status;

  int n = SSL_do_handshake(client.ssl);
  status = get_sslstatus(client.ssl, n);

  /* Did SSL request to write bytes? */
  if (status == SSLSTATUS_WANT_IO)
    do {
      n = BIO_read(client.wbio, buf, sizeof(buf));
      if (n > 0)
        queue_encrypted_bytes(buf, n);
      else if (!BIO_should_retry(client.wbio))
        return SSLSTATUS_FAIL;
    } while (n>0);

  return status;
} // do ssl handshake



int do_sock_read(const int fd)
{
struct buffer_data inbuff;
char inb [string_sz];
inbuff.p = inb;
inbuff.max = string_sz;

inbuff.len = sock_read (fd, inuff.p, inbuff.max);



}

 /* 
loggingf ("do sock read\n");

char buf[string_sz];
  ssize_t n = sock_read(client.fd, buf, sizeof(buf));

  if (n>0)
{
loggingf ("read bytes ready to decrypt\n");
loggingf ("encrypted bytes [%d]: %s\n", n, buf);

size_t len = n;
char *src = buf;

enum sslstatus status;

while (len > 0) {
n = BIO_write(client.rbio, src, len);

if (n<=0)
return -1; 

src += n;
len -= n;

if (!SSL_is_init_finished(client.ssl)) {
if (do_ssl_handshake() == SSLSTATUS_FAIL)
return -1;

if (!SSL_is_init_finished(client.ssl))
return 0;
} // end if renegotiate


do {
n = SSL_read(client.ssl, buf, sizeof(buf));
if (n > 0)
print_unencrypted_data(buf ,(size_t) n);
//client.io_on_read(buf, (size_t)n);
} while (n > 0);

status = get_sslstatus(client.ssl, n);

if (status == SSLSTATUS_WANT_IO)
do {
n = BIO_read(client.wbio, buf, sizeof(buf));
if (n > 0)
queue_encrypted_bytes(buf, n);
else if (!BIO_should_retry(client.wbio))
return -1;
} while (n>0);

if (status == SSLSTATUS_FAIL)
return -1;
} // while buf > 0


const char *hello = "HTTP/1.1 200 OK\nContent-Type: text/plain\nContent-Length: 12\n\nHello world!";
 send_unencrypted_bytes(hello, strlen(hello));
  //return 0;
//} // end if read > 0
} //end do sock read
*/

int main (int argc, char **argv)
{
struct args_data args;
args.port = 5555;
strcpy (args.base_path.p, ".");
strcpy (args.editor_path.p, "aceeditor.htm");
args.ssl = 1;	

init_log ("log.txt");

loggingf ("admin load\nPort: %d\nPath: %s\nEditor: %s\n", args.port, args.base_path.p, args.editor_path.p);

    init_sockbackdoor ("bd.txt");

struct sockaddr_in address;
socklen_t addrlen = sizeof(address);

int servfd = prepsocket (args.port);

char inbuffer [string_sz];
struct buffer_data inbuff;
inbuff.p = inbuffer;
inbuff.max = (string_sz);

// main loop
//
if (args.ssl)
	ssl_init("server.crt", "server.key"); // see README to create these files

while (1)
{
struct request_data request;
loggingf ("waiting\n");

int connfd = accept(servfd, (struct sockaddr *)&address, (socklen_t*)&addrlen);
sock_setnonblock (connfd);

if (args.ssl)
	ssl_client_init(&client, connfd, SSLMODE_SERVER);
// keep alive loop
int kacount = 1;
int procint = 1;

// keep alive loop
while (1)
{
do_sock_read (connfd); exit (0);

	
	
inbuff.len = sock_read (connfd, inbuff.p, inbuff.max);
if (inbuff.len == -1)
{ loggingf ("client timed out\n");  close (connfd); break; }

request = process_request (connfd, args, inbuff);
(request.method == 'G')?
loggingf ("GET request: %s\n", request.uri.p):
loggingf ("POST request: %s\n", request.uri.p);

if (request.method == 'G' && request.mode == edit)
    procint = get_edit_file (args, request);

if (request.method == 'G' && request.mode == config)
    procint = get_config (args, request);

if (request.method == 'G' && request.mode == file)
    procint = get_file (args, request);


if (request.method == 'P' && request.mode == edit)
	procint = post_edit (inbuff, request);

if (request.method == 'P' && request.mode == file)
	procint = post_file (inbuff, request);

if (request.mode == favicon)
	servico (connfd);

if (procint > 1)
	kacount = procint;

//if (procint == 1)
	--kacount;

if (procint == -1 || kacount == 0)
{ softclose (connfd, &inbuff); break;}

} // keep alive loop
} // main loop
} // man