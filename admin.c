#include "admin.h"
#include <signal.h>
#include <openssl/sha.h>

//Encodes Base64
#include <openssl/bio.h>
#include <openssl/evp.h>
#include <openssl/buffer.h>
#include <stdint.h>
int get_action (const struct args_data args, const struct request_data request)
{
    char outb [maxbuffer];
struct buffer_data out;
out.p = outb;
out.len = 0;
out.max = maxbuffer;

char fname [200];
char ext [10];

int d1 = getlast (request.path.p, (int) '.', request.path.len);
strncpy (ext, request.path.p + d1, request.path.len);

d1 = getlast (request.path.p, (int) '/', request.path.len);
strncpy (fname, request.path.p + d1 + 1, request.path.len - d1);

int showedit = 0;
enum eviewtype
{none, txt, img} viewtype;

loggingf ("the extracted extension is %s\n", ext);

if (strcmp(ext, ".c") == 0)
		viewtype = txt;

if (strcmp(ext, ".h") == 0)
		viewtype = txt;

if (strcmp(ext, ".htm") == 0)
		viewtype = txt;

if (strcmp(ext, ".html") == 0)
		viewtype = txt;

if (strcmp(ext, ".jpg") == 0)
		viewtype = img;

if (strcmp(ext, ".jpeg") == 0)
		viewtype = img;

if (strcmp(ext, ".png") == 0)
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
buffcatf (&out, "let fname = \"%s\";\n", fname);

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
buffcatf(&out, "var newfname = prompt (\"New File Name:\", \"%s\");\n", fname);

buffcatf (&out, "if (newfname == null) return;\n");


buffcatf (&out, "window.alert (newfname); \npostdata (\'rename\', newfname);\n}\n");

buffcatf (&out, "</script>\n");

buffcatf (&out, "%s<br>\n", fname);

buffcatf (&out, "<a href=\"/file%s\">View File</a><br>\n", request.path.p);
if (showedit)
	buffcatf (&out, "<a href=\"/edit%s\">Edit File</a><br>\n", request.path.p);

buffcatf (&out, "<input type=\"button\" class=\"button\" value=\"Rename\" onclick=\"frename()\">");
buffcatf (&out, "<input type=\"button\" class=\"button\" value=\"Copy\" onclick=\"fcopy()\">");
buffcatf (&out, "<input type=\"button\" class=\"button\" value=\"Move\" onclick=\"fmove()\">");
buffcatf (&out, "<input type=\"button\" class=\"button\" value=\"Delete\" onclick=\"fdelete()\"><br>\n");

if (viewtype == img)
	buffcatf (&out, "<img src=\"/file%s\"></img>", request.path.p);

if (viewtype == txt)
{
int localfd = open (request.fullpath.p, O_RDONLY);
if (localfd == -1)
	goto skip;

struct string_data txtdata;

txtdata.len = read (localfd, txtdata.p, string_sz);
close (localfd);

buffcatf (&out, "%.*s", txtdata.len, txtdata.p);

skip:;
} // if text preview

buffcatf (&out, "</body></html>");
struct string_data head;



head.len = sprintf (head.p, "%s%s%s%s%d\n\n", hthead, connka, conthtml, contlen, out.len);
sock_write (request.fd, head.p, head.len);

sock_write (request.fd, out.p, out.len);
return 1;
    

} // get_action

int post_action (const struct request_data request, const struct buffer_data inbuff)
{
loggingf ("%s\n", inbuff.p);
send_txt (request.fd, "recieved", 0);
exit (0);
return 1;

} // post action

int send_mredirect (const int fd, const char *msg, const char *uri)
{
char outbuff [string_sz];
char head [string_sz];

int doclen = sprintf (outbuff, "<html><body><script>window.alert(\"%s\"); window.location=\"%s\";</script></body></html>", msg, uri);



int headlen = sprintf (head, "%s%s%s%s%d\n\n", hthead, conthtml, connclose, contlen, doclen);

sock_write (fd, head, headlen);
sock_write (fd, outbuff, doclen);

return 1;
} //send_mredirect

int send_redirect (const int fd, const char *uri)
{
char outbuff [string_sz];
char head [string_sz];

int doclen = sprintf (outbuff, "<html><body><script>window.location=\"%s\";</script></body></html>", uri);



int headlen = sprintf (head, "%s%s%s%s%d\n\n", hthead, conthtml, connclose, contlen, doclen);

sock_write (fd, head, headlen);
sock_write (fd, outbuff, doclen);

return 1;
} //send_redirect

void save_buffer (const struct buffer_data b, const char *path)
{
int localfd = open (path, O_WRONLY | O_TRUNC| O_CREAT, S_IRUSR | S_IWUSR);
if (localfd < 0)
	return ;

write (localfd, b.p, b.len);


close (localfd);
}// save_page

int get_multifile (const struct request_data request)
{
char cnum [10];
char procpath [string_sz];

int d1 = search (request.uri.p, "count=", 0, request.uri.len);

strncpy (cnum, request.uri.p + d1, request.uri.len - d1);

d1 = getnext (request.path.p, '?', 0, request.path.len);
//send_ftxt (request.fd, "path is; %s\n d1 is: %d\n", request.path.p, d1);

//exit (0);



strncpy (procpath, request.path.p, d1);
int count = atoi (cnum);
//loggingf ("number of requests: %d\nprocpath path: %s\n", count, procpath);

//exit (0);

struct buffer_data out;
char outd [maxbuffer];
out.p = outd;
out.max = maxbuffer;
out.len = 0;

buffcatf (&out, "<!DOCTYPE html>\n<html>\n<head>\n");

buffcatf (&out,"<style>\n");
buffcatf (&out,"body\n{\ntext-align:left;\nmargin-left:70px;\nbackground-color:aqua;\nfont-size:24px;\n}\n");
buffcatf (&out, "a:link\n{\ncolor:midnightblue;\ntext-decoration:none;\n}\n");

buffcatf (&out, ".button {\npadding-left:20px;\npadding-right:20px;\nfont-size:18px;\n}");


buffcatf (&out, "</style>\n</head>\n<body>\n");

buffcatf (&out, "<form enctype=\"multipart/form-data\" action=\"/upload%s\" method=\"post\">\n", procpath);

buffcatf (&out, "<input type=\"hidden\" name=\"count\" value=\"%d\"><br><br>\n", count);

for (int i = 0; i < count; ++i)
buffcatf (&out, "<input type=\"file\" class=\"button\" name=\"myFile\"><br><br>\n");


buffcatf (&out, "<br><input type=\"submit\" class=\"button\"  value=\"upload\">\n</form>\n");

buffcatf (&out, "</body></html>");
loggingf ("%d: bytes multi-upload page (internal)\n", out.len);
struct string_data head;

head.len = sprintf (head.p, "%s%s%s%d\n\n", hthead, conthtml, contlen, out.len);
sock_writeold (request.fd, head.p, head.len);

//save_page (out, "saved.htm");
sock_buffwrite (request.fd, &out);


return 1;
} // get multifile

long multipart_reciever (const char *path, const struct buffer_data mainbuff, const struct request_data request)
{

int localfd = open (path, O_RDWR | O_TRUNC| O_CREAT, S_IRUSR | S_IWUSR);
if (localfd < 0)
	return -1;

write (localfd, mainbuff.p + request.procint, mainbuff.len - request.procint);

long progress = mainbuff.len - request.procint;
if (mainbuff.p [mainbuff.len - 3] == '-' && mainbuff.p [mainbuff.len - 4] == '-')
	return 1;

struct string_data inbuff;
while (1)
{
inbuff.len = sock_read (request.fd, inbuff.p, string_sz);
if (inbuff.len == -1) {send_txt (request.fd, "client timed out", 0); return (-1);}

write (localfd, inbuff.p, inbuff.len);
progress += inbuff.len;


if (inbuff.p [inbuff.len - 3] == '-' && inbuff.p [inbuff.len - 4] == '-')
return localfd;

} // while
return -1;

}// multipart_reciever

void parse_multifiles (const char *path, const int localfd, const struct request_data request)
{
    

} // parse_multifiles


int post_multifile (const struct buffer_data mainbuff, const struct request_data request)
{
int multifd = multipart_reciever ("tempfile.txt", mainbuff, request);

parse_multifiles ("tempfile.txt", multifd, request);

// run multifile_parser OR multi_parse with config/file/action POST callback  handlers 
return 1;
} // post_multifile

/*
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

inbuff.len = sock_read (request.fd, inbuff.p, inbuff.max);

loggingf ("%s\n\nbreak\n\n%s", mainbuff.p, inbuff.p);

exit (0);

// search for first boundary in initial xmission
//fbound = strsearch (mainbuff.p, request.boundary, request.procint, mainbuff.len);
fbound = search (mainbuff.p, request.boundary, request.procint, mainbuff.len);
if (fbound > - 1)
{
startdata = getnext (mainbuff.p, 10, fbound, mainbuff.len);
startdata = getnext (mainbuff.p, 10, startdata + 1, mainbuff.len);
startdata = getnext (mainbuff.p, 10, startdata + 1, mainbuff.len);
startdata = getnext (mainbuff.p, 10, startdata + 1, mainbuff.len);

while (mainbuff.p [startdata] == 10 || mainbuff.p [startdata] == 13 || mainbuff.p [startdata] == '.')
    ++startdata;

//int d1 = strsearch (mainbuff.p, "filename=\"", fbound, mainbuff.len);
int d1 = search (mainbuff.p, "filename=\"", fbound, mainbuff.len);
int d2 = getnext (mainbuff.p, (int) '\"', d1, mainbuff.len);
memcpy (fname, mainbuff.p + d1, d2 - d1);
sprintf (fullpath, "%s/%s", request.fullpath.p, fname);
localfd = open (fullpath, O_WRONLY | O_TRUNC| O_CREAT, S_IRUSR | S_IWUSR);
if (localfd < 0)  {send_txt(request.fd,"1, error opening newfile",0); return -1; }

//rbound = strsearch (mainbuff.p, request.boundary, fbound + 1, mainbuff.len);
rbound = search (mainbuff.p, request.boundary, fbound + 1, mainbuff.len);
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
//fbound = strsearch (inbuff.p, request.boundary, 0, inbuff.len);
fbound = search (inbuff.p, request.boundary, 0, inbuff.len);
++cnt;
if (cnt == 5) {send_txt(request.fd,"1, 1st boundary not found",0); return -1; }
} // while fbound - 1

startdata = getnext (inbuff.p, 10, fbound, inbuff.len);
startdata = getnext (inbuff.p, 10, startdata + 1, inbuff.len);
startdata = getnext (inbuff.p, 10, startdata + 1, inbuff.len);
startdata = getnext (inbuff.p, 10, startdata + 1, inbuff.len);

while (inbuff.p [startdata] == 10 || inbuff.p [startdata] == 13 || inbuff.p [startdata] == '.')
    ++startdata;

//int d1 = strsearch (inbuff.p, "filename=\"", fbound, inbuff.len);
int d1 = search (inbuff.p, "filename=\"", fbound, inbuff.len);
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
//rbound = strsearch (inbuff.p, request.boundary, inbuff.len - request.boundlen - 10, inbuff.len);
rbound = search (inbuff.p, request.boundary, inbuff.len - request.boundlen - 10, inbuff.len);

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
close (localfd);
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

ftruncate (localfd, progress - request.boundlen - 8);

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


return 1;	
} // post multifile
*/


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


void getwebsock (const struct request_data request)
{
printf ("getwebsock\n");

struct string_data inbuff;

const char *response = "HTTP/1.1 101 Switching Protocols\r\nUpgrade: websocket\r\nConnection: Upgrade\r\nSec-WebSocket-Accept: ";

char *append = "258EAFA5-E914-47DA-95CA-C5AB0DC85B11";

char tosha [string_sz];
size_t len = sprintf (tosha, "%s%s", request.boundary, append);

//strcpy (tosha, request.boundary);
//strcat (tosha, append);
//size_t len = strlen (tosha);

unsigned char *conv = (unsigned char*)tosha;
//char data[] = "Hello, world!";
//size_t length = strlen(data);
unsigned char hash[SHA_DIGEST_LENGTH];
SHA1(conv, len, hash); // hash now contains the 20-byte SHA-1 hash

//printf ("hash: %s\n", hash);

char *b64rtn;


  Base64Encode(hash, SHA_DIGEST_LENGTH, &b64rtn);


char handshake [200];
len = sprintf (handshake, "%s%s\r\n\r\n", response, b64rtn);

int a = sock_writeold (request.fd, handshake, len);

if (a != len)
	printf ("sorry\n");

printf ("[ %s ] done\n", handshake);
} // websock

int main (int argc, char **argv)
{
signal(SIGPIPE, SIG_IGN);
struct args_data args;
args.port = 9999;
args.showaction = 2;
// 0 for none, 1 show action page 2 load previewcon action page
strcpy (args.base_path.p, ".");
strcpy (args.editor_path.p, "aceeditor.htm");
	

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
while (1)
{
struct request_data request;
loggingf ("waiting\n");

int connfd = accept(servfd, (struct sockaddr *)&address, (socklen_t*)&addrlen);
sock_setnonblock (connfd);
//int set = 1; setsockopt(connfd, SOL_SOCKET, SO_NOSIGPIPE, (void *)&set, sizeof(int));
// keep alive loop
int kacount = 1;
int procint = 1;

// keep alive loop
while (1)
{
inbuff.len = sock_read (connfd, inbuff.p, inbuff.max);
if (inbuff.len == -1)
{ loggingf ("client timed out\n");  close (connfd); break; }

request = process_request (connfd, args, inbuff);
request.mainbuff = &inbuff;

(request.method == 'G')?
loggingf ("GET: %s\n", request.uri.p):
loggingf ("POST: %s\n", request.uri.p);

if (request.mode == websock)
	{getwebsock (request); exit (0);}

if (request.mode == root)
	send_redirect (connfd, "/file");

if (request.method == 'G' && request.mode == edit)
    procint = get_edit_file (args, request);

if (request.method == 'G' && request.mode == config)
    procint = get_config (args, request);

if (request.method == 'G' && request.mode == file)
    procint = get_file (args, request);

if (request.method == 'G' && request.mode == action)
    procint = get_action (args, request);

if (request.method == 'G' && request.mode == upload)
    procint = get_multifile (request);


if (request.method == 'P' && request.mode == upload)
    procint = post_multifile (inbuff, request);

if (request.method == 'P' && request.mode == action)
    procint = post_action (request, inbuff);

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
} // main

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
 buffcatf (&out,"body\n{\ntext-align:left;\nmargin-left:70px;\nbackground-color:aqua;\nfont-size:24px;\n}\n");
 buffcatf (&out, "a:link\n{\ncolor:midnightblue;\ntext-decoration:none;\n}\n");


buffcatf (&out, ".button {\npadding-left:20px;\npadding-right:20px;\nfont-size:18px;\n}");
 buffcatf (&out, "</style>\n</head>\n<body>\n");

buffcatf (&out, "<script>");
buffcatf (&out, "function get_multi () { \n");

//buffcatf (&out, "window.alert (\"wow\");\n");

buffcatf (&out, "var count = window.prompt (\"How Many\", 2);\n");
buffcatf (&out, "form = document.createElement(\'form\');\n");
buffcatf (&out, "form.setAttribute(\'method\', \'GET\');\n");
buffcatf (&out, "form.setAttribute(\'action\', \'/upload%s\');\n", request.path.p);

buffcatf (&out, "myvar = document.createElement(\'input\');\n");
buffcatf (&out, "myvar.setAttribute(\'name\', \'count\');\n");
buffcatf (&out, "myvar.setAttribute(\'type\', \'hidden\');\n");
buffcatf (&out, "myvar.setAttribute(\'value\', count);\n");

//buffcatf (&out, "myvar2 = document.createElement(\'input\');\n");
//buffcatf (&out, "myvar2.setAttribute(\'name\', \'path\');\n");
//buffcatf (&out, "myvar2.setAttribute(\'type\', \'hidden\');\n");
//buffcatf (&out, "myvar2.setAttribute(\'value\', \'/upload%s\');\n", request.path.p);


buffcatf (&out, "form.appendChild(myvar);\n");

//buffcatf (&out, "form.appendChild(myvar2);\n");
buffcatf (&out, "document.body.appendChild(form);\n");
buffcatf (&out, "form.submit();\n}\n");  
// end function

buffcatf (&out, "</script>\n");


dp = opendir (request.fullpath.p);
if (dp == NULL)
	{send_txt (request.fd, "OOPS", 0); return -1;}

buffcatf (&out, "%s<br>\n", request.path.p);

buffcatf (&out, "<input type=\"button\" class=\"button\" value=\"make\">\n");

buffcatf (&out, "<input type=\"button\" class=\"button\" value=\"mkdir\">\n");
buffcatf (&out, "<input type=\"button\" class=\"button\" value=\"new\"><br>\n");

buffcatf (&out, "<input type=\"button\" class=\"button\" value=\"search\">\n");
// addonclick here
buffcatf(&out, "<input type=\"button\" class=\"button\" value=\"Multiple Upload\" onclick=\"get_multi ()\">\n");
//
buffcatf (&out, "<form enctype=\"multipart/form-data\" action=\"%s\" method=\"post\">\n", request.uri.p);
buffcatf (&out, "<input type=\"file\" class=\"button\" name=\"myFile\">\n");
buffcatf (&out, "<input type=\"submit\" class=\"button\"  value=\"upload\">\n</form>\n");

while ((ep = readdir (dp)))
{
if (ep->d_name[0] == '.')
	continue;
	
// 4 is dir 8 is file
//loggingf ("dir: %s\n", ep->d_name);

if (ep->d_type == 4)

(request.uri.p[request.uri.len - 1] == '/')?
buffcatf (&out, "<a href=\"%s%s\">%s/</a><br>\n", request.uri.p, ep->d_name, ep->d_name):
buffcatf (&out, "<a href=\"%s/%s\">%s/</a><br>\n", request.uri.p, ep->d_name, ep->d_name);

//loggingf ("char: %c, dirname is: %s\n", request.uri.p[request.uri.len - 1], request.uri.p);
} // while

closedir (dp);
dp = opendir (request.fullpath.p);

while ((ep = readdir (dp)))
{
if (ep->d_name[0] == '.')
	continue;
	
if (ep->d_type == 8)

if (args.showaction > 0)	
(request.uri.p[request.uri.len - 1] == '/')?
buffcatf (&out, "<a href=\"/action%s%s\">%s</a><br>\n", request.path.p, ep->d_name, ep->d_name):
buffcatf (&out, "<a href=\"/action%s/%s\">%s</a><br>\n", request.path.p, ep->d_name, ep->d_name);


if (args.showaction == 0)	
(request.uri.p[request.uri.len - 1] == '/')?
buffcatf (&out, "<a href=\"/file%s%s\">%s</a><br>\n", request.path.p, ep->d_name, ep->d_name):
buffcatf (&out, "<a href=\"/file%s/%s\">%s</a><br>\n", request.path.p, ep->d_name, ep->d_name);


//stradd (outbuff.p, ep->d_name, &outbuff.len);

} // while



buffcatf (&out, "</body>\n</html>");
// save page
//save_buffer (out, "dir.htm");
//savepage
loggingf ("%d: bytes directory info\n", out.len);
struct string_data head;

head.len = sprintf (head.p, "%s%s%s%d\n\n", hthead, conthtml, contlen, out.len);
sock_writeold (request.fd, head.p, head.len);
sock_buffwrite (request.fd, &out);

closedir (dp);

return 1;
} // serv_dir

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

if (strcmp(mime_ext, ".png") == 0)
{i=1;  mime_txt = contpng;}

if (strcmp(mime_ext, ".mp4") == 0)
{i=1;  mime_txt = contmp4;}

if (i == 0)
{mime_txt = conttxt;}

//loggingf ("%s\n", mime_txt);

//struct stat finfo;
//stat (request.fullpath.p, &finfo);

outbuff.len = sprintf (outbuff.p, "%s%s%s%d\n\n", hthead, mime_txt, contlen, size);

loggingf ("%d bytes: %s", size, mime_txt);

sock_writeold (request.fd, outbuff.p, outbuff.len);

sendfileold (request.fullpath.p, request.fd);
return 1;

} // serv_file`

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
//request.mainbuff = &inbuff;


int d1 = getnext (inbuff.p, 32, 0, inbuff.len);
++d1;
int d2 = getnext (inbuff.p, 32, d1, inbuff.len);

//request.uri.len = midstr (inbuff.p, request.uri.p, d1, d2);

strncpy (request.uri.p, inbuff.p + d1, d2 - d1);
request.uri.len = d2 - d1;

if (request.uri.p[0] == '/' && request.uri.p[1] == 0)
{request.mode = root; return request;}

if (!strcmp(request.uri.p, "/favicon.ico"))
		{request.mode = favicon; return request; }

if (!strcmp(request.uri.p, "/config"))
		{request.mode = config; return request; }

//d1 = strsearch (request.uri.p, "/action", 0, 8);
d1 = search (request.uri.p, "/action", 0, 8);
if (d1 > 0)
{
request.mode = action;
memcpy (request.path.p, request.uri.p + d1, request.uri.len - d1);
request.path.len = request.uri.len - d1 - 1;
} // if edit

//d1 = strsearch (request.uri.p, "/edit", 0, 6);
d1 = search (request.uri.p, "/edit", 0, 6);
if (d1 > 0)
{
request.mode = edit;
memcpy (request.path.p, request.uri.p + d1, request.uri.len - d1);
request.path.len = request.uri.len - d1 - 1;
} // if edit

//d1 = strsearch (request.uri.p, "/file", 0, 6);
d1 = search (request.uri.p, "/file", 0, 6);
if (d1 > 0)
{
request.mode = file;
strncpy (request.path.p, request.uri.p + d1, request.uri.len - d1);
request.path.len = request.uri.len - d1;
} // if file

//d1 = strsearch (request.uri.p, "/upload", 0, 6);
d1 = search (request.uri.p, "/upload", 0, 8);
if (d1 > 0)
{
request.mode = upload;
strncpy (request.path.p, request.uri.p + d1, request.uri.len - d1);
request.path.len = request.uri.len - d1;
} // if file

d1 = search (request.uri.p, "/websock", 0, 8);
if (d1 > 0)
{
request.mode = websock;
//d1 = search (inbuff.p, "Sec-WebSocket-Accept: ", d1, inbuff.len);
d1 = search (inbuff.p, "Sec-WebSocket-Key: ", d1, inbuff.len);
d2 = getnext (inbuff.p, 13, d1, inbuff.len);
strncpy (request.boundary, inbuff.p + d1, d2 - d1);
//request.boundlen = midstr (inbuff.p, request.boundary, d1, d2);
request.boundlen = d2 - d1;
return request;
} // if websock

request.fullpath.len = sprintf (request.fullpath.p, "%s%s", args.base_path.p, request.path.p);

if (request.method == 'P')
{
//d1 = strsearch (inbuff.p, "boundary=", 0, inbuff.len);
d1 = search (inbuff.p, "boundary=", 0, inbuff.len);
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

//int assets = countassets (editorbuffer);

close (editorfd);
close (filefd);

//editorbuffer.procint = strsearch (editorbuffer.p, "DELIMETER", 0, editorbuffer.len);
editorbuffer.procint = search (editorbuffer.p, "DELIMETER", 0, editorbuffer.len);
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

loggingf ("%d bytes: edit file served\n", outbuff.len);

sock_writeold (request.fd, head.p, head.len);
sock_buffwrite (request.fd, &outbuff);
//return assets + 1;
return 4;
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
//fbound = strsearch (mainbuff.p, request.boundary, request.procint, mainbuff.len);
fbound = search (mainbuff.p, request.boundary, request.procint, mainbuff.len);
if (fbound > - 1)
{
// get past name data, since only one object is sent
startdata = getnext (mainbuff.p, 10, fbound, mainbuff.len);
startdata = getnext (mainbuff.p, 10, startdata + 1, mainbuff.len);

while (mainbuff.p [startdata] == 10 || mainbuff.p [startdata] == 13 || mainbuff.p [startdata] == '.')
    ++startdata;

//rbound = strsearch (mainbuff.p, request.boundary, fbound + 1, mainbuff.len);
rbound = search (mainbuff.p, request.boundary, fbound + 1, mainbuff.len);
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
//fbound = strsearch (inbuff.p, request.boundary, 0, inbuff.len);
fbound = search (inbuff.p, request.boundary, 0, inbuff.len);
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
//rbound = strsearch (inbuff.p, request.boundary, inbuff.len - request.boundlen - 10, inbuff.len);
rbound = search (inbuff.p, request.boundary, inbuff.len - request.boundlen - 10, inbuff.len);
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

ftruncate (localfd, progress - request.boundlen - 8);

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

send_txt (request.fd, "file recieved, multi-part reciever",0);
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
//fbound = strsearch (mainbuff.p, request.boundary, request.procint, mainbuff.len);
fbound = search (mainbuff.p, request.boundary, request.procint, mainbuff.len);
if (fbound > - 1)
{
startdata = getnext (mainbuff.p, 10, fbound, mainbuff.len);
startdata = getnext (mainbuff.p, 10, startdata + 1, mainbuff.len);
startdata = getnext (mainbuff.p, 10, startdata + 1, mainbuff.len);
startdata = getnext (mainbuff.p, 10, startdata + 1, mainbuff.len);

while (mainbuff.p [startdata] == 10 || mainbuff.p [startdata] == 13 || mainbuff.p [startdata] == '.')
    ++startdata;

//int d1 = strsearch (mainbuff.p, "filename=\"", fbound, mainbuff.len);
int d1 = search (mainbuff.p, "filename=\"", fbound, mainbuff.len);
int d2 = getnext (mainbuff.p, (int) '\"', d1, mainbuff.len);
memcpy (fname, mainbuff.p + d1, d2 - d1);
sprintf (fullpath, "%s/%s", request.fullpath.p, fname);
localfd = open (fullpath, O_WRONLY | O_TRUNC| O_CREAT, S_IRUSR | S_IWUSR);
if (localfd < 0)  {send_txt(request.fd,"1, error opening newfile",0); return -1; }

//rbound = strsearch (mainbuff.p, request.boundary, fbound + 1, mainbuff.len);
rbound = search (mainbuff.p, request.boundary, fbound + 1, mainbuff.len);
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
//fbound = strsearch (inbuff.p, request.boundary, 0, inbuff.len);
fbound = search (inbuff.p, request.boundary, 0, inbuff.len);
++cnt;
if (cnt == 5) {send_txt(request.fd,"1, 1st boundary not found",0); return -1; }
} // while fbound - 1

startdata = getnext (inbuff.p, 10, fbound, inbuff.len);
startdata = getnext (inbuff.p, 10, startdata + 1, inbuff.len);
startdata = getnext (inbuff.p, 10, startdata + 1, inbuff.len);
startdata = getnext (inbuff.p, 10, startdata + 1, inbuff.len);

while (inbuff.p [startdata] == 10 || inbuff.p [startdata] == 13 || inbuff.p [startdata] == '.')
    ++startdata;

//int d1 = strsearch (inbuff.p, "filename=\"", fbound, inbuff.len);
int d1 = search (inbuff.p, "filename=\"", fbound, inbuff.len);
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
//rbound = strsearch (inbuff.p, request.boundary, inbuff.len - request.boundlen - 10, inbuff.len);
rbound = search (inbuff.p, request.boundary, inbuff.len - request.boundlen - 10, inbuff.len);

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
close (localfd);
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

ftruncate (localfd, progress - request.boundlen - 8);

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


int send_ftxt (const int fd, const char *format, ...)
{
	
va_list ap;

va_start (ap, format);

int formatlen = strlen (format);
char type;
int specify_len = 0;
char entry [maxbuffer] = "";

int len = 0;
int d;
char c;
char *s;


for (int fplace = 0; fplace < formatlen; ++fplace)
{
if (format [fplace] == '%')
{
type = format [fplace + 1];
// if length is specified as %.*s
if (type == '.') { specify_len = 1; type = format [fplace + 3];}

if (type == 's')
{
if (specify_len)
{
d = va_arg(ap, int);
s = va_arg(ap, char *);
len += sprintf (entry + len, "%.*s", d, s);
fplace += 3;
} // if spec_len	

if (!specify_len)
{
s = va_arg(ap, char *);
len += sprintf (entry + len, "%s", s);
++fplace;

} // if ! specify_len
} // if s
    
if (type == 'd')
{
d = va_arg(ap, int);
len+= sprintf (entry + len, "%d", d);
++fplace;
} // if d
    

if (type == 'c')
{
c = (char) va_arg(ap, int);
entry [len] = c;
++len;
entry [len] = 0;
++fplace;    
} // if c
    
    
}else{ // else if    
entry [len] = format [fplace];
++len;

} // if    
} // for

//printf ("%s\n", entry);

va_end(ap);

char outbuffer [maxbuffer];
struct buffer_data outbuff;
outbuff.p = outbuffer;
outbuff.len = 0;
outbuff.max = maxbuffer;

outbuff.len = sprintf (outbuff.p, "%s%s%s%s%d\n\n%.*s", hthead, conttxt, connclose, contlen, len, len, entry);

sock_buffwrite (fd, &outbuff);

//if (printfon)
  //  printf ("%.*s", len, entry);


//write (fd, entry, len);

return 1;
} // send_ftxt
