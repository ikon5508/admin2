
#include "admin.h"

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
int dot = getlast (request.path.p, (int) '.');


char outbuffer [string_sz];
struct buffer_data outbuff;
outbuff.p = outbuffer;
outbuff.len = 0;
outbuff.max = string_sz;

char mime_ext[10] = "";
const char *mime_txt;

memcpy (mime_ext, request.path.p + dot, request.path.len - dot);

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
buffcatf (&out, "<a href=\"/file/%s/%s\">%s</a><br>\n", request.path.p, ep->d_name, ep->d_name);



//stradd (outbuff.p, ep->d_name, &outbuff.len);

} // while



buffcatf (&out, "</body>\n</html>");


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


int get_config (const struct args_data args, const struct request_data request)
{
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


buffcatf (&out, "uri: %s<br>\n path: %s<br>\n fpath: %s<br>\n", request.uri.p, request.path.p, request.fullpath.p);
buffcatf (&out, "<img src=\"/file/pic.jpg\">");
struct string_data head;


head.len = sprintf (head.p, "%s%s%s%s%d\n\n", hthead, connka, conthtml, contlen, out.len);
sock_writeold (request.fd, head.p, head.len);

sock_writeold (request.fd, out.p, out.len);
return 100;
} // get config

struct request_data process_request (const int fd, const struct args_data args, const struct buffer_data inbuff)
{
struct request_data request;
memset (&request, 0, sizeof (request));

struct request_data nrequest;
memset (&nrequest, 0, sizeof (request));

request.method = inbuff.p [0];
request.fd = fd;

int d1 = getnext (inbuff.p, 32, 0, inbuff.len);
++d1;
int d2 = getnext (inbuff.p, 32, d1, inbuff.len);

//request.uri.len = midstr (inbuff.p, request.uri.p, d1, d2);

memcpy (request.uri.p, inbuff.p + d1, d2 - d1);
request.uri.len = strlen (request.uri.p);

if (!strcmp(request.uri.p, "/favicon.ico"))
		{request.mode = favicon; return request; }

if (!strcmp(request.uri.p, "/config"))
		{request.mode = config; return request; }

d1 = strsearch (inbuff.p, "Connection: keep-alive", d2, inbuff.len);
if (d1 > 0)
request.keepalive = 1;

d1 = strsearch (request.uri.p, "/edit", 0, 6);
if (d1 > 0)
{
request.mode = edit;
memcpy (request.path.p, request.uri.p + d1 + 1, request.uri.len - d1 - 1);
request.path.len = strlen (request.path.p);

} // if edit

d1 = strsearch (request.uri.p, "/file", 0, 6);
if (d1 > 0)
{
request.mode = file;
memcpy (request.path.p, request.uri.p + d1 + 1, request.uri.len - d1 - 1);
request.path.len = strlen (request.path.p);
} // if file

request.fullpath.len = sprintf (request.fullpath.p, "%s/%s", args.base_path.p, request.path.p);

if (request.method == 'P')
{
d1 = strsearch (inbuff.p, "boundary=", 0, inbuff.len);
d2 = getnext (inbuff.p, 10, d1, d1 + 100);

memcpy (request.boundary, inbuff.p + d1, d2 - d1 - 1);
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

sock_writeold (request.fd, head.p, head.len);
sock_buffwrite (request.fd, &outbuff);
return 1;

} // get_edit_file

int send_err (const int fd, const int code)
{

if (code == 410)
	sock_write (fd, "HTTP/1.1 410 GONE\n", 0);


if (code == 500)
	sock_write (fd, "HTTP/1.1 500 ERROR\n", 0);

return 1;
} // send_err

int post_edit (const struct buffer_data mainbuff, const struct request_data request)
{
int startdata = -1;
int enddata = -1;
int fbound =-1, rbound=-1;
const int saveold = 0;

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

strcpy (newfname, request.fullpath.p);
strcat (newfname, ".new");
int localfd = open (newfname, O_WRONLY | O_TRUNC| O_CREAT, S_IRUSR | S_IWUSR);
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

for (int i = startdata; i < enddata; ++i)
{
filedata.p [filedata.len] = mainbuff.p [i];
++filedata.len;
} // for
filedata.procint = write (localfd, filedata.p, filedata.len);
if (filedata.procint != filedata.len)  {send_txt(request.fd,"error writing single reciever",0); return -1; }
close (localfd);
rename (newfname, request.fullpath.p);
send_txt (request.fd, "File recieved, single reciever", 0);
return 1;
} // if rbound

enddata = mainbuff.len;
for (int i = startdata; i < enddata; ++i)
{
filedata.p [filedata.len] = mainbuff.p [i];
++filedata.len;
} // for
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
rbound = strsearch (inbuff.p, request.boundary, startdata +1, inbuff.len);
if (rbound == -1) enddata = inbuff.len;

if (rbound > 0)
{
enddata = inbuff.len - request.boundlen - 2;
while (inbuff.p [enddata] == 10 || inbuff.p [enddata] == 13 || inbuff.p [enddata] == '-')
    --enddata;
++enddata;
} // if rbound

// copy filedata, away from socket buffer, read socket again
filedata.len = 0;
for (int i = startdata; i < enddata; ++i)
	{ filedata.p [filedata.len] = inbuff.p [i]; ++filedata.len; }
startdata = 0;

filedata.procint = write (localfd, filedata.p, filedata.len);
if (filedata.procint != filedata.len)  {send_txt(request.fd,"error writing multipart-reciever",0); return -1; }

if (rbound == -1)
	inbuff.len = sock_read (request.fd, inbuff.p, inbuff.max);

if (inbuff.len == -1)
{
loggingf ("split boundary!!!\n");

struct stat finfo;
fstat (localfd, &finfo);

ftruncate (localfd, finfo.st_size - request.boundlen - 5);

// added additional 5 here, to accomodate the -----...seems imprecise
// if fails again e.g. on chrome based browsers
// use lseek reset fpos, read last 100 
//
// redermine and calculate truncate len
close (localfd);
rename (newfname, request.fullpath.p);
send_txt (request.fd, "CHECK DATA-split boundary file recieved, multi-part reciever CHECK DATA",0);

return 1;
} // if timeout finish
} // while rbound

close (localfd);
rename (newfname, request.fullpath.p);

send_txt (request.fd, "file recieved, multi-part reciever",0);
return 1;

}

int main (int argc, char **argv)
{
struct args_data args;
args.port = 9999;
strcpy (args.base_path.p, ".");
strcpy (args.editor_path.p, "aceeditor.htm");
args.backdoor = 1;
	

init_log ("log.txt");

loggingf ("admin load\nPort: %d\nPath: %s\nEditor: %s\n", args.port, args.base_path.p, args.editor_path.p);

if (args.backdoor)
    init_sockbackdoor ("bd.txt");

struct sockaddr_in address;
socklen_t addrlen = sizeof(address);

int servfd = prepsocket (args.port);

char inbuffer [string_sz];
struct buffer_data inbuff;
inbuff.p = inbuffer;
inbuff.max = (string_sz);

// main loop
while (1)
{
struct request_data request;
loggingf ("waiting\n");

int connfd = accept(servfd, (struct sockaddr *)&address, (socklen_t*)&addrlen);
sock_setnonblock (connfd);

// keep alive loop
int kacount = 1;
int procint = 1;

// keep alive loop
while (1)
{
inbuff.len = sock_read (connfd, inbuff.p, inbuff.max);
if (inbuff.len == -1)
{ loggingf ("client timed out\n"); break; }

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

if (request.mode == favicon)
	send_err (connfd, 500);

if (procint > 1)
	kacount = procint;

if (procint == 1)
	--kacount;

if (procint == -1)
	break;

if (kacount == 0)
	break;
} // keep alive loop
shutdown (connfd,  SHUT_WR);

//wait for client to close connection
int a;

while (1) 
{
a=read(connfd, inbuff.p, inbuff.max);

if (a > 0)
loggingf ("reading \"success\" in kill mode");

if(a < 0)
{
{
usleep (1000);
//logging ("closing, remote, not ready",1,0);
}
//exit(1);
}
if(!a) 
{
loggingf ("Connection closed by client\n");
break;
} // if socket non blocked
} // for wait for close bit.

close(connfd);

} // main loop
} // main

//The C library function size_t strspn(const char *str1, const char *str2) calculates the length of the initial segment of str1 which consists entirely of characters in str2
//
//library function size_t strcspn(const char *str1, const char *str2) calculates the length of the initial segment of str1, which consists entirely of characters not in str2.
