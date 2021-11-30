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

int showedit = 0;
enum eviewtype
{none, txt, img} viewtype;

if (strcmp(request.ext, ".txt") == 0)
		viewtype = txt;
if (strcmp(request.ext, ".c") == 0)
		viewtype = txt;

if (strcmp(request.ext, ".h") == 0)
		viewtype = txt;

if (strcmp(request.ext, ".htm") == 0)
		viewtype = txt;

if (strcmp(request.ext, ".html") == 0)
		viewtype = txt;

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
buffcatf (&out, "let fname = \"%s\";\n", request.resourcename);

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
buffcatf(&out, "var newfname = prompt (\"New File Name:\", \"%s\");\n", request.resourcename);

buffcatf (&out, "if (newfname == null) return;\n");


buffcatf (&out, "window.alert (newfname); \npostdata (\'rename\', newfname);\n}\n");

buffcatf (&out, "</script>\n");

buffcatf (&out, "%s<br>\n", request.resourcename);

buffcatf (&out, "<a href=\"/file%s\">View File</a><br>\n", request.path);
if (showedit)
	buffcatf (&out, "<a href=\"/edit%s\">Edit File</a><br>\n", request.path);

buffcatf (&out, "<input type=\"button\" class=\"button\" value=\"Rename\" onclick=\"frename()\">");
buffcatf (&out, "<input type=\"button\" class=\"button\" value=\"Copy\" onclick=\"fcopy()\">");
buffcatf (&out, "<input type=\"button\" class=\"button\" value=\"Move\" onclick=\"fmove()\">");
buffcatf (&out, "<input type=\"button\" class=\"button\" value=\"Delete\" onclick=\"fdelete()\"><br>\n");

if (viewtype == img)
	buffcatf (&out, "<img src=\"/file%s\"></img>", request.path);

if (viewtype == txt)
{
int localfd = open (request.fullpath, O_RDONLY);
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
send_txt (request.fd, "recieved");
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
/*

d1 = search (inbuff.p, "Sec-WebSocket-Key: ", d1, inbuff.len);
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

int a = sock_writeold (request.fd, handshake, len);

if (a != len)
	printf ("sorry\n");

//printf ("[ %s ] done\n", handshake);
//
inbuff.len = sock_read (request.fd, inbuff.p, string_sz);
//printf ("%s\n", inbuff.p);

//int localfd = open (request.fullpath, O_WRONLY | O_TRUNC| O_CREAT, S_IRUSR | S_IWUSR);
int loc = open ("frame.bin",  O_WRONLY | O_TRUNC| O_CREAT, S_IRUSR | S_IWUSR);
if (loc ==-1)
{printf ("error"); exit (0);}

write (loc, inbuff.p, inbuff.len);

printf ("len: %d\n", inbuff.len);

*/
} // websock

int main (int argc, char **argv)
{
signal(SIGPIPE, SIG_IGN);
struct args_data args;
args.port = 9999;
args.showaction = 2;
// 0 for none, 1 show action page 2 load previewcon action page
strcpy (args.base_path, ".");
strcpy (args.editor_path, "aceeditor.htm");
	

init_log ("log.txt");

loggingf ("admin load\nPort: %d\nPath: %s\nEditor: %s\n", args.port, args.base_path, args.editor_path);

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

char str[INET_ADDRSTRLEN];
  inet_ntop(address.sin_family, &address.sin_addr, str, INET_ADDRSTRLEN);
printf("new connection from %s:%d\n", str, ntohs(address.sin_port));



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

//inbuff.p[inbuff.len + 1] = 0;
request = process_request (connfd, args, inbuff);
request.mainbuff = &inbuff;

//if (request.procint == -1)
//	exit (0);

//loggingf ("the god damned motherfucking method is: %c!!!!!!!!!!!\n", request.method);

(request.method == 'E')?
loggingf ("GET: %s\n", request.uri):
loggingf ("POST: %s\n", request.uri);

if (request.mode == websock)
	{getwebsock (request); exit (0);}

if (request.mode == root)
	send_redirect (connfd, "/file");

if (request.method == 'E' && request.mode == edit)
    procint = get_edit_file (args, request);

if (request.method == 'E' && request.mode == config)
    procint = get_config (args, request);

if (request.method == 'E' && request.mode == file)
             procint = get_file (args, request);
             
if (request.method == 'E' && request.mode == action)
    procint = get_action (args, request);


//if (request.method == 'O' && request.mode == action)
//    procint = post_action (request, inbuff);

if (request.method == 'O' && request.mode == edit)
	procint = put_edit (request);
if (request.method == 'U' && request.mode == edit)
	procint = put_edit (request);

if (request.method == 'O' && request.mode == file)
	procint = put_file (request);
if (request.method == 'U' && request.mode == file)
	procint = put_file (request);

if (request.mode == favicon)
	servico (connfd);

if (procint > 1)
	kacount = procint;

//if (procint == 1)
	--kacount;

if (procint == -1)
{close (connfd); break;}

if (kacount == 0)
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

buffcatf (&out, "var count = window.prompt (\"How Many\", 2);\n");
buffcatf (&out, "var content = \"<form enctype=\'multipart/form-data\' action=\'%s\' method=\'post\'>\";\n", request.uri);
buffcatf (&out, "var i;\n");
buffcatf (&out, "for (i= 0; i < count; ++i)\n");
buffcatf (&out, "content += \"<input type=\'file\' class=\'button\' name=\'myFile\'>\";\n");
buffcatf (&out, "content += \"<input type=\'submit\' class=\'button\'  value=\'upload\'>\";\n");
buffcatf (&out, "content += \"</form>\";\n");
buffcatf (&out, "document.getElementById(\"uploader\").innerHTML = content;\n}\n");


// end function

buffcatf (&out, "</script>\n");


dp = opendir (request.fullpath);
if (dp == NULL)
	{send_txt (request.fd, "OOPS"); return -1;}

buffcatf (&out, "%s<br>\n", request.path);

buffcatf (&out, "<input type=\"button\" class=\"button\" value=\"make\">\n");

buffcatf (&out, "<input type=\"button\" class=\"button\" value=\"mkdir\">\n");
buffcatf (&out, "<input type=\"button\" class=\"button\" value=\"new\"><br>\n");

buffcatf (&out, "<input type=\"button\" class=\"button\" value=\"search\">\n");
// addonclick here
buffcatf(&out, "<input type=\"button\" class=\"button\" value=\"Multiple Upload\" onclick=\"get_multi ()\">\n");
//

buffcatf (&out, "<div id=\"uploader\">\n");
buffcatf (&out, "<form enctype=\"multipart/form-data\" action=\"%s\" method=\"post\">\n", request.uri);
buffcatf (&out, "<input type=\"file\" class=\"button\" name=\"myFile\">\n");
buffcatf (&out, "<input type=\"submit\" class=\"button\"  value=\"upload\">\n</form>\n");
buffcatf (&out, "</div>\n");

while ((ep = readdir (dp)))
{
if (ep->d_name[0] == '.')
	continue;
	
// 4 is dir 8 is file
//loggingf ("dir: %s\n", ep->d_name);

if (ep->d_type == 4)

(request.uri[strlen (request.uri) - 1] == '/')?
buffcatf (&out, "<a href=\"%s%s\">%s/</a><br>\n", request.uri, ep->d_name, ep->d_name):
buffcatf (&out, "<a href=\"%s/%s\">%s/</a><br>\n", request.uri, ep->d_name, ep->d_name);

//loggingf ("char: %c, dirname is: %s\n", request.uri[strlen (request.uri) - 1], request.uri.p);
} // while

closedir (dp);
dp = opendir (request.fullpath);

while ((ep = readdir (dp)))
{
if (ep->d_name[0] == '.')
	continue;
	
if (ep->d_type == 8)

if (args.showaction > 0)	
(request.uri[strlen (request.uri) - 1] == '/')?
buffcatf (&out, "<a href=\"/action%s%s\">%s</a><br>\n", request.path, ep->d_name, ep->d_name):
buffcatf (&out, "<a href=\"/action%s/%s\">%s</a><br>\n", request.path, ep->d_name, ep->d_name);


if (args.showaction == 0)	
(request.uri[strlen (request.uri) - 1] == '/')?
buffcatf (&out, "<a href=\"/file%s%s\">%s</a><br>\n", request.path, ep->d_name, ep->d_name):
buffcatf (&out, "<a href=\"/file%s/%s\">%s</a><br>\n", request.path, ep->d_name, ep->d_name);


//stradd (outbuff.p, ep->d_name, &outbuff.len);

} // while



buffcatf (&out, "</body>\n</html>");
// save page
save_buffer (out, "dir.htm");
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
int dot = getlast (request.path, (int) '.', strlen (request.path));
char outbuffer [string_sz];
struct buffer_data outbuff;
outbuff.p = outbuffer;
outbuff.len = 0;
outbuff.max = string_sz;

char mime_ext[10] = "";
const char *mime_txt;

strncpy (mime_ext, request.path + dot, strlen (request.path) - dot);

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
//stat (request.fullpath, &finfo);

outbuff.len = sprintf (outbuff.p, "%s%s%s%d\n\n", hthead, mime_txt, contlen, size);

loggingf ("%d bytes: %s", size, mime_txt);

sock_writeold (request.fd, outbuff.p, outbuff.len);

return sendfile (request.fullpath, request.fd);
//return 1;

} // serv_file`

int get_file (const struct args_data args, const struct request_data request)
{

struct stat finfo;
if (stat(request.fullpath, &finfo) == -1)
    {send_txt (request.fd, "bad resource"); return (-1);}

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

request.method = inbuff.p [1];
request.fd = fd;
//request.mainbuff = &inbuff;


int d1 = getnext (inbuff.p, 32, 0, inbuff.len);
++d1;
int d2 = getnext (inbuff.p, 32, d1, inbuff.len);
int rear = d2;

//strlen (request.uri) = midstr (inbuff.p, request.uri, d1, d2);
int procint = d2 - d1;
strncpy (request.uri, inbuff.p + d1, d2 - d1);

d1 = getlast (request.uri, '/', procint); 
strncpy (request.resourcename, request.uri + d1 +1, procint - d1 -1);

d1 = getlast (request.uri, '.', procint); 
strncpy (request.ext, request.uri + d1, procint - d1);

if (request.uri[0] == '/' && request.uri[1] == 0)
{request.mode = root; return request;}

if (!strcmp(request.uri, "/favicon.ico"))
		{request.mode = favicon; return request; }

if (!strcmp(request.uri, "/config"))
		{request.mode = config; return request; }

//d1 = strsearch (request.uri, "/action", 0, 8);
d1 = search (request.uri, "/action", 0, 8);
if (d1 > 0)
{
request.mode = action;
memcpy (request.path, request.uri + d1, strlen (request.uri) - d1);
} // if edit

//d1 = strsearch (request.uri, "/edit", 0, 6);
d1 = search (request.uri, "/edit", 0, 6);
if (d1 > 0)
{
request.mode = edit;
memcpy (request.path, request.uri + d1, strlen (request.uri) - d1);
} // if edit

//d1 = strsearch (request.uri, "/file", 0, 6);
d1 = search (request.uri, "/file", 0, 6);
if (d1 > 0)
{
request.mode = file;
strncpy (request.path, request.uri + d1, strlen (request.uri) - d1);
} // if file

//d1 = strsearch (request.uri, "/upload", 0, 6);
d1 = search (request.uri, "/upload", 0, 8);
if (d1 > 0)
{
request.mode = upload;
strncpy (request.path, request.uri + d1, strlen (request.uri) - d1);
} // if file

d1 = search (request.uri, "/websock", 0, 8);
if (d1 > 0)
{
request.mode = websock;
//d1 = search (inbuff.p, "Sec-WebSocket-Accept: ", d1, inbuff.len);
} // if websock

sprintf (request.fullpath, "%s%s", args.base_path, request.path);

if (request.method == 'E')
	return (request);

char temp [100] = "";

d1 = search (inbuff.p, "ength: ", rear, inbuff.len);
if (d1 ==-1) {send_txt (fd, "error ength: "); request.procint = -1; return request;}
d2 = getnext (inbuff.p, 10, d1, inbuff.len);
if (d2 ==-1) {send_txt (fd, "error end (669)"); request.procint = -1; return request;}
midstr (inbuff.p, temp, d1, d2);
rtrim (temp);
request.content_len = atol (temp);

d1 = search (inbuff.p, "gent: ", rear, inbuff.len);
if (d1 ==-1) {send_txt (fd, "error gent: "); request.procint = -1; return request;}
d2 = getnext (inbuff.p, 10, d1, inbuff.len);
if (d2 ==-1) {send_txt (fd, "error end (677)"); request.procint = -1; return request;}
midstr (inbuff.p, request.user_agent, d1, d2);

if (request.mode == edit)
{
request.procint = getnext (inbuff.p, (int) '\"', rear, inbuff.len);
return request;
} // if edit

if (request.mode == file)
{
loggingf ("put file processed in proc\n");
d1 = search (inbuff.p, "boundary=", rear, inbuff.len);
if (d1 ==-1) {loggingf ("error boundary: "); request.procint = -1; return request;}
d2 = getnext (inbuff.p, 10, d1, inbuff.len);
if (d2 ==-1) {loggingf ("error end (694)"); request.procint = -1; return request;}
midstr (inbuff.p, request.code, d1, d2);
request.codelen = rtrim (request.code);

//request.procint = search (inbuff.p, request.code, d2, inbuff.len);
request.procpnt = strstr (inbuff.p + d2, request.code);

//printf ("code: %s, procint: %d\n", request.code, request.procint);
return request;
} // if file

// content len
// boundary
// startdata or actually begin data
//
return request;	
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


int editorfd = open (args.editor_path, O_RDONLY);
if (editorfd < 0)
    {send_txt (request.fd, "bad editor"); return -1;}

int filefd = open (request.fullpath, O_RDONLY);
if (filefd < 0)
    {send_txt (request.fd, "bad file name"); return -1;}

editorbuffer.len = read (editorfd, editorbuffer.p, editorbuffer.max);
filebuffer.len = read (filefd, filebuffer.p, filebuffer.max);

//int assets = countassets (editorbuffer);

close (editorfd);
close (filefd);

//editorbuffer.procint = strsearch (editorbuffer.p, "DELIMETER", 0, editorbuffer.len);
editorbuffer.procint = search (editorbuffer.p, "DELIMETER", 0, editorbuffer.len);
if (editorbuffer.procint < 0)
    {send_txt (request.fd, "failed to find DELIMETER"); return -1;}

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

int put_edit (const struct request_data request)
{
loggingf ("put edit commence\n");


struct buffer_data inbuff;
char inb [string_sz];
inbuff.max = string_sz;
inbuff.p = inb;

struct buffer_data filedata;
char fd [maxbuffer];
filedata.max = maxbuffer;
filedata.p = fd;
filedata.len = 0;

struct buffer_data json;
char js [maxbuffer];
json.max = maxbuffer;
json.p = js;
json.len = 0;

int progress = 0;

if (request.procint > 0)
{
loggingf ("data started in first xmission\n");
memcpy (json.p, request.mainbuff->p + request.procint, request.mainbuff->len - request.procint);
json.len = request.mainbuff->len - request.procint;
progress = json.len;
}else{
loggingf ("data not started in first xmission\n");
} //if

while (progress < request.content_len)
{
inbuff.len = sock_read (request.fd, inbuff.p, inbuff.max);
if (inbuff.len ==-1)
{loggingf ("read timeout, 839\n"); break;}

memcpy (json.p + json.len, inbuff.p, inbuff.len);
progress += inbuff.len;
json.len += inbuff.len;
} // while cat json

loggingf ("finished cat json\n");


for (int i = 0; i < json.len; ++i)
{
if (json.p[i] == '\\')
{
if (json.p[i+1] == 'n')
	filedata.p[filedata.len] = 10;

else if (json.p[i+1] == '\\')
	filedata.p[filedata.len] = '\\';

else if (json.p[i+1] == '\"')
	filedata.p[filedata.len] = '\"';

else if (json.p[i+1] == '\'')
	filedata.p[filedata.len] = '\'';

else if (json.p[i+1] == 't')
	filedata.p[filedata.len] = 9;
else
loggingf ("missing escape sequence\n");
	//{loggingf ("the missing escape is: %d (%c)\n", src->p[i+1], src->p[i+1]);
//dest->p[dest->len] = '^';}

++i;
++filedata.len;
continue;
} // if
filedata.p[filedata.len] = json.p[i];
++filedata.len;
} // for



int localfd = open (request.fullpath, O_WRONLY | O_TRUNC| O_CREAT, S_IRUSR | S_IWUSR);
write (localfd, filedata.p + 1, filedata.len - 2);
close (localfd);

send_txt (request.fd, "it worked");
return 1;
} //put edit

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


buffcatf (&out, "uri: %s<br>\n path: %s<br>\n fpath: %s<br>\n", request.uri, request.path, request.fullpath);
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

void safe_fname (const struct request_data request, const char *fname, char *rtn)
{
  sprintf (rtn, "%s/%s", request.fullpath, fname);
  struct stat finfo;
  int check = stat (rtn, &finfo);
  if (check != 0) return;

 int copynum = 1;
 while (check == 0)
 {
    sprintf (rtn, "%s/%d-%s",request.fullpath, copynum, fname);  
      check = stat (rtn, &finfo);

      ++copynum;
 }// while
}// safe_fname

void process_multifile (int *localfd, const long read_progress, const struct request_data request, struct buffer_data *filedata, const struct buffer_data inbuff)
{
const int safename = 1;

int d1 = 0, d2 = 0;
char fname [nameholder];
char fullpath [string_sz] = "";
int startdata, enddata, writelen;
int noend = 0, offset = 0;

char *p1, *p2;
while (1)
{
if (*localfd == -1) 
{
p1 = strstr (filedata->p + offset, "filename=\"");
if (p1 == NULL) {loggingf ("no starting delim found 969\n"); break;}
d1 = p1 - filedata->p + 10;

p2 = strchr (filedata->p + d1 + 2, (int) '\"');
d2 = p2 - filedata->p;

int cpylen = d2 - d1;
memset (fname, 0, nameholder);
memcpy (fname, filedata->p + d1, cpylen);

//printf ("copylen: %d fname: %s\n",cpylen, fname);
    
if (safename) safe_fname (request, fname, fullpath);
else sprintf (fullpath, "%s/%s", request.fullpath, fname);

*localfd = open (fullpath, O_WRONLY | O_TRUNC| O_CREAT, S_IRUSR | S_IWUSR);
if (*localfd == -1)
    {loggingf ("error opening local file\n"); exit (0);}

}// if no file open
p1 = strchr (filedata->p + d2 + 4, 10);
startdata = p1 - filedata->p;

while (filedata->p[startdata] == 13 || filedata->p[startdata] == 10)
++startdata;

p1 = strstr (filedata->p + d2, request.code);
if (p1 == NULL)
{

enddata = filedata->len - 200; //noend = 1;
filedata->len = 200;
writelen = enddata - startdata +1;

write (*localfd, filedata->p + startdata, writelen);

loggingf ("no rboundary found, returning");

memmove (filedata->p, filedata->p + enddata, filedata->len);

return ;

} // more logic here for long xmission no rbound found
else
{
enddata = p1 - filedata->p -3;
while (filedata->p[enddata] == 13 || filedata->p[enddata] == 10)
--enddata;
 ++enddata;   
writelen = enddata - startdata +1;

write (*localfd, filedata->p + startdata, writelen);
close (*localfd);
*localfd = -1;

offset = enddata + request.codelen;
    
} /// if enddata adjustment (found rboundary)

} // grand loop

} // process_multifile

int put_file (const struct request_data request)
{
loggingf ("put_file commence\n");
const int safename = 1;

struct buffer_data filedata;
char fd [maxbuffer];
filedata.p = fd;
filedata.len = 0;
filedata.max = maxbuffer;

struct buffer_data inbuff;
char in [string_sz];
inbuff.p = in;
inbuff.len = 0;
inbuff.max = string_sz;



long read_progress = 0;
int localfd = -1;

if (request.procpnt != NULL)
{
loggingf ("data started in first xmission\n");

//filedata.len = midstr (request.mainbuff->p, filedata.p, request.procint, request.mainbuff->len);
int d1 = request.procpnt - request.mainbuff->p;
filedata.len = request.mainbuff->len - d1;
memcpy (filedata.p, request.procpnt, filedata.len);

read_progress = filedata.len + 2;

//printf ("save data\read progress: %ld, content_len: %ld\n", read_progress, request.content_len);

//    exit (0);
} // if data started in first xmission

while (read_progress < request.content_len)
{
inbuff.len = sock_read (request.fd, inbuff.p, inbuff.max);
read_progress += inbuff.len;

//printf ("buff len: %d read progress: %ld, content_len: %ld\n", filedata.len, read_progress, request.content_len);

// buffer overflow protection
if ((filedata.len + inbuff.len) > filedata.max)
    process_multifile (&localfd, read_progress, request, &filedata, inbuff);
    
memcpy (filedata.p + filedata.len, inbuff.p, inbuff.len);
filedata.len += inbuff.len;

} // while read loop

//process data

process_multifile (&localfd, read_progress, request, &filedata, inbuff);

send_txt (request.fd, "check");
//exit (0);
    return 1;
} 


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

int send_txt (const int fd, const char *txt)
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
