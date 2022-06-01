#include "admin.h"

#include <signal.h>
#include <openssl/sha.h>

#include <poll.h>
const int timeout = 3;





int get_action (const struct settings_data settings, const struct request_data request)
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
	buffcatf (&out, "<img src=\"/file%s\" width=\"600px\" height=\"auto\"></img>", request.path);

if (viewtype == txt)
{
    printf ("generating text preview\n");
int localfd = open (request.fullpath, O_RDONLY);
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
sock_write (request.fd, head.p, head.len);
save_buffer (out, "actionpage.htm");
sock_write (request.fd, out.p, out.len);
return 1;
    

} // get_action

int post_action (const struct request_data request, const struct buffer_data inbuff)
{
// bm post action
printf ("%s\n", inbuff.p);
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
strcpy (settings.editor_path, argv[i+1]);

} // for settings

struct sockaddr_in address;
socklen_t addrlen = sizeof(address);

int servfd = prepsocket (settings.port);

printf ("admin load\nPort: %d\nPath: %s\nEditor: %s\n", settings.port, settings.base_path, settings.editor_path);

char inbuffer [string_sz];
struct buffer_data inbuff;
inbuff.p = inbuffer;
inbuff.max = (string_sz);

// main loop
while (1)
{
printf ("waiting\n");

int connfd = 0;
connfd = accept(servfd, (struct sockaddr *)&address, (socklen_t*)&addrlen);
sock_setnonblock (connfd);

char str[INET_ADDRSTRLEN];
  inet_ntop(address.sin_family, &address.sin_addr, str, INET_ADDRSTRLEN);
printf("new connection from %s:%d\n", str, ntohs(address.sin_port));

// keep alive loop
int kacount = 1;
int procint = 1;

// keep alive loop
while (1)
{
inbuff.len = sock_read (connfd, inbuff.p, inbuff.max);
if (inbuff.len == -1)
{ printf ("client timed out (main-loop)\n");  close (connfd); break; }
inbuff.p[inbuff.len] = 0;

struct request_data request;
request = process_request (connfd, inbuff);
request.mainbuff = &inbuff;


(request.method == 'G')?
printf ("GET: %s\n", request.uri):
printf ("POST: %s\n", request.uri);

if (request.mode == websock)
	{getwebsock (request); exit (0);}

if (request.mode == root)
	send_redirect (connfd, "/file");

if (request.method == 'G' && request.mode == edit)
    procint = get_edit_file (request);

if (request.method == 'G' && request.mode == config)
    procint = get_config (settings, request);

if (request.method == 'G' && request.mode == file)
             procint = get_file (settings, request);
             
if (request.method == 'G' && request.mode == action)
    procint = get_action (settings, request);


//if (request.method == 'O' && request.mode == action)
//    procint = post_action (request, inbuff);

if (request.method == 'P' && request.mode == edit)
	{procint = put_edit (request); printf ("put file return: %d\n", procint);}


//if (request.method == 'U' && request.mode == edit)
//	procint = put_edit (request);

//if (request.method == 'O' && request.mode == upload)
//	procint = put_file (request);
//if (request.method == 'U' && request.mode == upload)
//	procint = put_file (request);

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

int serv_dir (const struct settings_data settings, const struct request_data request)
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
//printf ("dir: %s\n", ep->d_name);

if (ep->d_type == 4)

(request.uri[strlen (request.uri) - 1] == '/')?
buffcatf (&out, "<a href=\"%s%s\">%s/</a><br>\n", request.uri, ep->d_name, ep->d_name):
buffcatf (&out, "<a href=\"%s/%s\">%s/</a><br>\n", request.uri, ep->d_name, ep->d_name);

//printf ("char: %c, dirname is: %s\n", request.uri[strlen (request.uri) - 1], request.uri.p);
} // while

closedir (dp);
dp = opendir (request.fullpath);

while ((ep = readdir (dp)))
{
if (ep->d_name[0] == '.')
	continue;
	
if (ep->d_type == 8)

if (settings.showaction > 0)	
(request.uri[strlen (request.uri) - 1] == '/')?
buffcatf (&out, "<a href=\"/action%s%s\">%s</a><br>\n", request.path, ep->d_name, ep->d_name):
buffcatf (&out, "<a href=\"/action%s/%s\">%s</a><br>\n", request.path, ep->d_name, ep->d_name);


if (settings.showaction == 0)	
(request.uri[strlen (request.uri) - 1] == '/')?
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
sock_writeold (request.fd, head.p, head.len);
sock_buffwrite (request.fd, &out);

closedir (dp);

return 1;
} // serv_dir

int serv_file (const struct settings_data settings, const struct request_data request, const unsigned long size)
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

//printf ("%s\n", mime_txt);

//struct stat finfo;
//stat (request.fullpath, &finfo);

outbuff.len = sprintf (outbuff.p, "%s%s%s%ld\n\n", hthead, mime_txt, contlen, size);

printf ("%ld bytes: %s", size, mime_txt);

sock_writeold (request.fd, outbuff.p, outbuff.len);

return send_file (request.fullpath, request.fd);
//return 1;

} // serv_file`

int get_file (const struct settings_data settings, const struct request_data request)
{

struct stat finfo;
if (stat(request.fullpath, &finfo) == -1)
    {send_txt (request.fd, "bad resource"); return (-1);}

if (S_ISDIR(finfo.st_mode))
    return (serv_dir (settings, request));

if (S_ISREG(finfo.st_mode)) // is file
    return (serv_file (settings, request, finfo.st_size));

return 0;

} //get_file

int URL_decode (const char *in, const int start, const int end, char *out, const int out_max)
{ // bm URL_decode


return 1;
} // URL decode

struct request_data process_request (const int fd, const struct buffer_data inbuff)
{ // bm process_request
struct request_data request;
memset (&request, 0, sizeof (request));

request.method = inbuff.p [0];
request.fd = fd;

int d1 = getnext (inbuff.p, 32, 0, inbuff.len);
++d1;
int d2 = getnext (inbuff.p, 32, d1, inbuff.len);
int rear = d2;

int procint = d2 - d1;
if (procint ==-1) exit (0); // catch potential seg fault when kill signal sent

strncpy (request.uri, inbuff.p + d1, procint);
// get URI old way



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

d1 = strsearch (request.uri, "/action", 0);
if (d1 > 0)
{
request.mode = action;
memcpy (request.path, request.uri + d1, strlen (request.uri) - d1);
} // if edit

d1 = strsearch (request.uri, "/edit", 0);
if (d1 > 0)
{
request.mode = edit;
memcpy (request.path, request.uri + d1, strlen (request.uri) - d1);
} // if edit

d1 = strsearch (request.uri, "/file", 0);
if (d1 > 0)
{
request.mode = file;
strncpy (request.path, request.uri + d1, strlen (request.uri) - d1);
} // if file

d1 = strsearch (request.uri, "/upload", 0);
if (d1 > 0)
{
request.mode = upload;
strncpy (request.path, request.uri + d1, strlen (request.uri) - d1);
} // if file

//d1 = strsearch (request.uri, "/websock", 0, 8);
//if (d1 > 0)
//{
//request.mode = websock;
//d1 = strsearch (inbuff.p, "Sec-WebSocket-Accept: ", d1, inbuff.len);
//} // if websock

sprintf (request.fullpath, "%s%s", settings.base_path, request.path);

if (request.method == 'G')
	return (request);

char temp [100] = "";

d1 = strsearch (inbuff.p, "ength: ", rear);
if (d1 ==-1) {send_txt (fd, "error ength: "); request.procint = -1; return request;}
d2 = getnext (inbuff.p, 10, d1, inbuff.len);
if (d2 ==-1) {send_txt (fd, "error end (669)"); request.procint = -1; return request;}
midstr (inbuff.p, temp, d1, d2);
trim (temp);
request.content_len = atoi (temp);

d1 = strsearch (inbuff.p, "gent: ", rear);
if (d1 ==-1) {send_txt (fd, "error gent: "); request.procint = -1; return request;}
d2 = getnext (inbuff.p, 10, d1, inbuff.len);
if (d2 ==-1) {send_txt (fd, "error end (677)"); request.procint = -1; return request;}
midstr (inbuff.p, request.user_agent, d1, d2);

if (request.mode == edit)
{
request.procint = getnext (inbuff.p, (int) '\"', rear, inbuff.len);
return request;
} // if edit

if (request.mode == upload)
{
printf ("put file processed in proc\n");
d1 = strsearch (inbuff.p, "boundary=", rear);
if (d1 ==-1) {printf ("error boundary: "); request.procint = -1; return request;}
d2 = getnext (inbuff.p, 10, d1, inbuff.len);
if (d2 ==-1) {printf ("error end (694)"); request.procint = -1; return request;}
midstr (inbuff.p, request.code, d1, d2);
trim (request.code);

request.codelen = strlen (request.code);

request.procint = strsearch (inbuff.p, request.code, d2);
//request.procpnt = strstr (inbuff.p + d2, request.code);
//request.procint = request.procpnt - inbuff.p;
//printf ("code: %s, procint: %d\n", request.code, request.procint);
return request;
} // if file

// content len
// boundary
// startdata or actually begin data
//
return request;	
} // process_request

int get_edit_file (const struct request_data request)
{ // bm get_edit_file
struct stat finfo;

buffer_t editor;
editor.p = NULL;
if (stat (settings.editor_path, &finfo)) {send_txt (request.fd, "Bad Editor"); return 0;}
if (!init_buffer (&editor, finfo.st_size)) return 0;
int editor_fd = open (settings.editor_path, O_RDONLY);
if (editor_fd < 0) {send_txt (request.fd, "Bad Editor"); return 0;}
editor.len = read (editor_fd, editor.p, editor.max);
editor.p[editor.len] = 0;


buffer_t filedata;
filedata.p = NULL;
if (stat (request.fullpath, &finfo)) {send_txt (request.fd, "Bad FILE"); return 0;}
if (!init_buffer (&filedata, finfo.st_size)) return 0;
int file_fd = open (request.fullpath, O_RDONLY);
if (file_fd < 0) {send_txt (request.fd, "Bad FILE"); return 0;}
filedata.len = read (file_fd, editor.p, editor.max);
filedata.p[filedata.len] = 0;


buffer_t outbuff;
outbuff.p = NULL;
if (!init_buffer (&outbuff, lgbuff_sz)) return 0;

buffer_t bm;
bm.p = NULL;
if (!init_buffer (&bm, mdbuff_sz)) return 0;



cleanup:;
free (outbuff.p);
free (filedata.p);
free (editor.p);
free (bm.p);
return 1;
} // get_edit_file

/*
int get_edit_file (const struct settings_data settings, const struct request_data request)
{ // bm get_edit_file

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

char bm [maxbuffer];
struct buffer_data bmlist;
bmlist.p = bm;
bmlist.max = maxbuffer;
bmlist.len = 0;

int editorfd = open (settings.editor_path, O_RDONLY);
if (editorfd < 0)
    {send_txt (request.fd, "bad editor"); return -1;}

int filefd = open (request.fullpath, O_RDONLY);
if (filefd < 0)
    {send_txt (request.fd, "bad file name"); return -1;}

editorbuffer.len = read (editorfd, editorbuffer.p, editorbuffer.max);
editorbuffer.p [editorbuffer.len] = 0;
filebuffer.len = read (filefd, filebuffer.p, filebuffer.max);
filebuffer.p [filebuffer.len] = 0;
//int assets = countassets (editorbuffer);

close (editorfd);
close (filefd);

int linecount = 0;
int d1 = 0, d2 = 0;
char line [string_sz];

buffcatf (&bmlist, "<select class=\"button\" onchange=\"bookmark(event)\">\n");
//strcpy (bmlist.p, "<select onchange=bookmark (event)>\n");
//bmlist.len = strlen ("<select onchange=bookmark (event)>\n");

while (1)
{
d2 = getnext (filebuffer.p, (int) '\n', d1 + 1, filebuffer.len);
if (d2 ==-1) { printf ("lc: %d, break!!!\n", linecount); break;}
++linecount;
midstr (filebuffer.p, line, d1, d2);
//printf ("line: %s\n", line);
int check = strsearch (line, "// bm-", 0);
if (check > 0)
{
 printf ("match!!!\n");
    char bm [nameholder]; // duplicate name, but encapsulation should be ok
    midstr (line, bm, check, strlen (line));
    printf ("bookmark: %s\n", bm);
    buffcatf (&bmlist, "<option value=\"%d\">%s</option>\n", linecount, bm);
}// if bookmark

d1 = d2;    
} // loop 
buffcatf (&bmlist, "</select>\n");
// bm- new_code

//<!--bookmarks-->

editorbuffer.procint = strsearch (editorbuffer.p, "<!--bookmarks-->", 0);
if (editorbuffer.procint < 0)
    {send_txt (request.fd, "failed to find !--bookmarks"); return -1;}

memcpy (outbuff.p, editorbuffer.p, editorbuffer.procint);
outbuff.len = editorbuffer.procint;

memcpy (outbuff.p + outbuff.len, bmlist.p, bmlist.len);
outbuff.len += bmlist.len;

int oldprocint = editorbuffer.procint;

editorbuffer.procint = strsearch (editorbuffer.p, "DELIMETER", 0);
if (editorbuffer.procint < 0)
    {send_txt (request.fd, "failed to find DELIMETER"); return -1;}

memcpy (outbuff.p + outbuff.len, editorbuffer.p + oldprocint, editorbuffer.procint - oldprocint - 9);
outbuff.len += editorbuffer.procint - oldprocint - 9;

//for (int i = 0; i < (editorbuffer.procint - 9); ++i)
//{
//outbuff.p [outbuff.len] = editorbuffer.p [i];
//++outbuff.len;
//} // for

// commence file parsing

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

printf ("%d bytes: edit file served\n", outbuff.len);

sock_writeold (request.fd, head.p, head.len);
sock_buffwrite (request.fd, &outbuff);
//return assets + 1;
return 4;
} // get_edit_file
*/

int put_edit (const struct request_data request)
{
printf ("put edit commence\n");


struct buffer_data inbuff;
char inb [string_sz];
inbuff.max = string_sz;
inbuff.p = inb;

struct buffer_data filedata;
char fd [maxbuffer];
filedata.max = maxbuffer;
filedata.p = fd;
filedata.len = 0;

struct buffer_data json;char js [maxbuffer];
json.max = maxbuffer;
json.p = js;
json.len = 0;

int progress = 0;

if (request.procint > 0)
{
printf ("data started in first xmission\n");
memcpy (json.p, request.mainbuff->p + request.procint, request.mainbuff->len - request.procint);
json.len = request.mainbuff->len - request.procint;
progress = json.len;
}else{
printf ("data not started in first xmission\n");
} //if

while (progress < request.content_len)
{
inbuff.len = sock_read (request.fd, inbuff.p, inbuff.max);
if (inbuff.len ==-1)
{printf ("read timeout, 839\n"); break;}

memcpy (json.p + json.len, inbuff.p, inbuff.len);
progress += inbuff.len;
json.len += inbuff.len;
} // while cat json

printf ("finished cat json\n");


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
printf ("missing escape sequence\n");
	//{printf ("the missing escape is: %d (%c)\n", src->p[i+1], src->p[i+1]);
//dest->p[dest->len] = '^';}

++i;
++filedata.len;
continue;
} // if
filedata.p[filedata.len] = json.p[i];
++filedata.len;
} // for

char backup [string_sz];
strcpy (backup, "old/");
strcat (backup, request.resourcename);
strcat (backup, "-%m%d%H%M");
strcat (backup, request.ext);

time_t t;
struct tm *tmp;
t = time(NULL); 
tmp = localtime(&t);
if (tmp == NULL) { perror("localtime"); exit(EXIT_FAILURE); } 
char outstr [string_sz];
if (strftime(outstr, sizeof(outstr), backup, tmp) == 0) 
{ fprintf(stderr, "strftime returnd 0"); exit(EXIT_FAILURE); } 

rename (request.fullpath, outstr);

int localfd = open (request.fullpath, O_WRONLY | O_TRUNC| O_CREAT, S_IRUSR | S_IWUSR);
write (localfd, filedata.p + 1, filedata.len - 2);
close (localfd);

send_txt (request.fd, "it worked");
return 1;
} //put edit

int get_config (const struct settings_data settings, const struct request_data request)
{
char outb [maxbuffer];
struct buffer_data out;
out.p = outb;
out.len = 0;
out.max = maxbuffer;

printf ("get config\n");

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

send_file ("favicon.ico", fd);
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
/*
int put_file (const struct request_data request)
{
printf ("put_file commence\n");

const int safename = 1;

string fdata;
fdata.len =-1;
fdata.procint = 0;

buffer audit;
char ab[maxbuffer];
audit.p = ab;
audit.len = 0;
audit.max = maxbuffer;


int filecount = 0, filenum = 0;
int fsize = 0, read_progress = 0;
int file_progress = 0;
int localfd = -1;

if (request.procint > 0)
{
int startdata = 0, enddata = 0;
char fname [nameholder];
char fullpath [string_sz];
int d1 = 0, d2 = 0;
read_progress = request.mainbuff->len - request.procint + request.codelen + 2;	
printf ("data started in first xmission: %d / %l\n", read_progress, request.content_len);
//find second boundary, place into d1

  // find start of fdata place into d1
d1 = getnext(request.mainbuff->p, '\"', request.procint, request.mainbuff->len);
d1 = getnext(request.mainbuff->p, '\"', d1 +1, request.mainbuff->len);
// attempt to find end of data
d2 = getnext(request.mainbuff->p, 10, d1 +5, request.mainbuff->len);

// for diagnostic purposes
buffcatf (&audit, "mainbuff->\n", request.mainbuff->p);

// diagnostics
if (d2==-1)
{
//printf ("mainbuff proc fdata started and spans 1114\n");
fdata.len=0;
d2 = request.mainbuff->len;

buffcatf (&audit, "fdata spans from mainbuff:\n");
}
else buffcatf (&audit, "fdata fully containedin mainbuff:\n");


midstr (request.mainbuff->p, fdata.p, d1 +1, d2);
enddata = d2;
buffcatf (&audit, "1264 initial fdata: %s\n", fdata.p);

if (fdata.len ==-1) // not started at all - fully constained in mainbuff
{
buffcatf (&audit, "complete fdata in first xmission\n");
char sfsize [nameholder];

ftrim (fdata.p);
fdata.len = rtrim (fdata.p);
buffcatf (&audit, "fdata trim: %s len: %d\n", fdata.p, fdata.len);
fdata.procint = getnext (fdata.p, ':', 0, fdata.len);
midstr (fdata.p, sfsize, 0, fdata.procint);
buffcatf (&audit, "1275 (mainbuff): str_filecount: %s\n", sfsize);
filecount = atoi (sfsize);

buffcatf (&audit, "1278 (mainbuff) int_file count %d\n", filecount);
} // if complete fdata in first xmission
else if (fdata.len == 0) {buffcatf (&audit, "fdata spans buffer reads trigger 1132\n");goto multi;}

buffcatf (&audit, "endata: %d bufflen: %d\n", enddata, request.mainbuff->len);
if (enddata < request.mainbuff->len)
{
buffcatf (&audit, "additional data may be started in first xmission\n");
//d1 = strsearch (request.mainbuff->p, request.code, enddata, request.mainbuff->len);
//	if (d1==-1) {printf ("strsearch error 1118\n"); exit (0);}

d1 = strsearch (request.mainbuff->p, "filename=\"", enddata, request.mainbuff->len);
	if (d1==-1) {buffcatf (&audit, "1290, no files in first xmission, data within next xmission\n"); goto multi;}
d2 = getnext (request.mainbuff->p, '\"', d1 + 1, request.mainbuff->len);
	if (d2==-1) {printf ("strsearch error 1292\n"); exit (0);}

midstr (request.mainbuff->p, fname, d1, d2);
buffcatf (&audit, "fname: %s\n", fname);

startdata = getnext (request.mainbuff->p, '\n', d2 + 4, request.mainbuff->len);
	if (startdata ==-1) {printf ("error 1298\n"); exit (0);}
startdata += 3;

if (safename) safe_fname (request, fname, fullpath);
else sprintf (fullpath, "%s/%s",request.fullpath, fname);

localfd = open (fullpath, O_WRONLY | O_TRUNC| O_CREAT, S_IRUSR | S_IWUSR);
	if (localfd ==-1) {printf ("error 1160\n"); exit(0);}
filenum = 1;
fsize = get_nextsize (&fdata);
buffcatf (&audit, "file opened: %s, %d: bytes\n", fullpath, fsize);


// if fsize smaller than remaining buffer
d1 = request.mainbuff->len - startdata;
if (fsize < d1)
{
write (localfd, request.mainbuff->p + startdata, fsize);
close (localfd);
localfd = -1;
printf ("first file recieved, within first xmission\n");

if (filecount == 1)
{printf ("only one file to recieved, done, check\n");
send_txt (request.fd, "only one small file to recieve, done");
return 1;} // if no more files and all data transmitted in single xmission

}else{ // else if fsizebigger than mainbuff
// if first file spans first and multiple xmission
enddata = request.mainbuff->len - startdata;
write (localfd, request.mainbuff->p + startdata, enddata);
file_progress = enddata;
buffcatf (&audit, "first file started in main, spans frames, %d\n", file_progress);

    
} // if file single / multi reciever 

} // if file(s) started aftr fdata


//long fsize = get_nextsize (fdata)
} // if procint > 0

multi:printf("ahead to subsequent reads\n");

struct ubuffer_data inbuff;
unsigned char in [string_sz * 2];
inbuff.p = in;
inbuff.len = 0;
inbuff.max = string_sz * 2;
inbuff.procint = 0;

// first check that fdata is completed
if (fdata.len == 0 || fdata.len == -1)
{
buffcatf (&audit, "fdata incomplete\n");
int startdata = 0, enddata = 0;
char temp1 [nameholder];

inbuff.len = usock_read (request.fd, inbuff.p, inbuff.max);
if (inbuff.len ==-1){printf ("client timed out\n"); return -1;}
read_progress += inbuff.len;

if (fdata.len == -1)
{
buffcatf (&audit, "fdata not started at all\n");
startdata = ustrsearch (inbuff.p, "name=\"fsize\"", 0, inbuff.len);
if (startdata==-1) {printf ("error locating fdata 1240"); exit (0);}
startdata += 3;
} // if fdata not started at all

enddata = ugetnext (inbuff.p, '\n', startdata + 1, inbuff.len);
if (enddata ==-1) {printf ("1227 error locating fdata end\n"); exit (0);}
inbuff.procint = enddata;

umidstr (inbuff.p, fdata.p, startdata, enddata);

ftrim (fdata.p);
fdata.len = rtrim (fdata.p);
buffcatf (&audit, "1235 fdata: %s, len: %d\n", fdata.p, fdata.len);

fdata.procint = getnext (fdata.p, ':', 0, fdata.len);

midstr (fdata.p, temp1, 0, fdata.procint);
buffcatf (&audit, "str filecount: %s\n", temp1);
filecount = atoi (temp1);
buffcatf (&audit, "last iteration filecount: %d\n", filecount);
} // if fdata incomplete

save_buffer (audit, "auditbuffer.txt");

// file reads now
// do subsequent reads here

while (read_progress < request.content_len || inbuff.procint > 0)
{ // if reading done, but file data remains

if (inbuff.procint > 0)
{ // ifdata remains in buff
int startdata = ustrsearch (inbuff.p, "filename=\"", inbuff.procint, inbuff.len);
if (startdata ==-1) {inbuff.procint = 0; continue;}

int enddata = ugetnext (inbuff.p, '\"', startdata +1, inbuff.len);
if (enddata ==1)
{printf ("not sure how this happened:\n"); exit (0);}

char fname [nameholder];
umidstr (inbuff.p, fname, startdata, enddata);

char fullpath [string_sz];
//if (safename) safe_fname (request, fname, fullpath);
//else 
sprintf (fullpath, "%s/%s", request.fullpath, fname);
printf ("fullpath: %s\n", fullpath);

localfd = open (fullpath, O_WRONLY | O_TRUNC| O_CREAT, S_IRUSR | S_IWUSR);
if (localfd ==-1) {printf ("error 1269\n"); exit(0);}

fsize = get_nextsize (&fdata);
filenum += 1;


startdata = ugetnext (inbuff.p, '\n', enddata + 3, inbuff.len);
startdata += 3;

int leftover = inbuff.len - startdata;
// check to write fsize or remaining buffer, set procint and continue;
if (leftover > fsize)
{
write (localfd, inbuff.p + startdata, fsize);
inbuff.procint = startdata;
close (localfd);
localfd = -1;
continue;
}else{
write (localfd, inbuff.p + startdata, inbuff.len - startdata);
file_progress = inbuff.len - startdata;
//inbuff.len = 0;
inbuff.procint = 0;
} // fsize bigger, data spans
} // if data remains in buff

inbuff.len = usock_read (request.fd, inbuff.p, inbuff.max);
if (inbuff.len ==-1){printf ("client timed out 1441\n"); return -1;}
read_progress += inbuff.len;



// if file already open
if (localfd > 0)
{
write (localfd, inbuff.p, inbuff.len);
file_progress += inbuff.len;

if (file_progress > fsize)
{
printf ("closing: %d\n", filenum);
ftruncate (localfd, fsize);
close (localfd);
localfd =-1;

int leftover = file_progress - fsize;
int offset = inbuff.len - leftover;

memmove (inbuff.p, inbuff.p + offset, inbuff.len - offset);

int newin = usock_read (request.fd, inbuff.p + offset, inbuff.max - offset);
if (newin == -1) {printf ("newin read timeout\n"); return -1;}
// a bit sloppy here....
// needs future reivision
inbuff.len += newin;

inbuff.procint = 1;

//inbuff.procint = inbuff.len - leftover;

continue;
} // if file done
} // if file already open

if (localfd == -1)
{ inbuff.procint = 1;}

} //w


hile data remains

send_txt (request.fd, "done");
printf ("done\n");
return 1;
} // put_file
*/

int send_err (const int fd, const int code)
{

if (code == 410)
	sock_writeold (fd, "HTTP/1.1 410 GONE", 0);


if (code == 500)
	sock_writeold (fd, "HTTP/1.1 500 ERROR", 0);

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




int sock_buffwrite (const int connfd, struct buffer_data *out)
{
int wlen = 0;
int progress = 0;
int offset = 0;
int i;

int written = sock_writeold (connfd, out->p, out->len);
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

written = sock_writeold (connfd, out->p, totalwrite);
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




void buffcatf (struct buffer_data *buff, const char *format, ...)
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

for (int i = 0; i < len; ++i)
{
buff->p [buff->len] = entry [i];
++buff->len;

} // for



//return 1;
} // buffcatf



int send_file (const char *path, const int fd)
{
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

int interim_progress = sock_writeold (fd, fbuff.p, fbuff.len);
if (interim_progress == -1)
	return -1;

int cpylen = interim_progress;
while (cpylen < fbuff.len)
{

interim_progress = sock_writeold (fd, fbuff.p + cpylen, fbuff.len - cpylen);
if (interim_progress == -1)
	return -1;
cpylen += interim_progress;
} // while

} // while loop
close (locfd);
free (c_fbuffer);
return 1;
} // sendfile

int sock_setnonblock (const int fd)
{
if (fcntl(fd, F_SETFL, fcntl(fd, F_GETFL, 0) | O_NONBLOCK) == -1)
{
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


int sock_writeold (const int connfd, const char *buffer, int size)
{ // bm sock writeold

if (size == 0)
    size = strlen (buffer);

struct pollfd ev;
ev.fd = connfd;
ev.events = POLLOUT;

int prtn = poll (&ev, 1, timeout * 1000);
if (prtn < 1) return -1;

return write (connfd, buffer, size);

} // sock_write_old

int sock_read (const int connfd, char *buffer, const int size)
{
struct pollfd ev;
ev.fd = connfd;
ev.events = POLLIN;

int r = poll (&ev, 1, timeout * 1000);
if (r == 0) return -1;

int len = read (connfd, buffer, size);
	
return len;
} // sock_read


int sock_write (const int connfd, char *out, const int len)
{
int wlen = 0;
int progress = 0;
int offset = 0;
int i;

int written = sock_writeold (connfd, out, len);
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

written = sock_writeold (connfd, out, totalwrite);
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


void save_buffer (const struct buffer_data b, const char *path)
{
int localfd = open (path, O_WRONLY | O_TRUNC| O_CREAT, S_IRUSR | S_IWUSR);
if (localfd < 0)
	return ;

write (localfd, b.p, b.len);


close (localfd);
}// save_page

