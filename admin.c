#include "admin.h"

#include <signal.h>
#include <openssl/sha.h>

#include <poll.h>

#define edit_mode "/edit"
#define action_mode "/action"
#define file_mode "/file"

const int file_mode_len = strlen (file_mode);
const int action_mode_len = strlen (action_mode);
const int edit_mode_len = strlen (edit_mode);
const char  BOOKMARK [] = {'/', '/', ' ', 'b', 'm', ' '};

const int timeout = 3;
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

//int localfd = open (request.full_path, O_WRONLY | O_TRUNC| O_CREAT, S_IRUSR | S_IWUSR);
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
while (1)
{
inbuff.len = sock_read (connfd, inbuff.p, inbuff.max);
if (inbuff.len == -1)
{ printf ("client timed out (main-loop)\n");  close (connfd); break; }
inbuff.p[inbuff.len] = 0;

struct request_data request;
int ret = process_request (&request, connfd, inbuff);
if (ret == 0) {close (connfd); break;}

request.mainbuff = &inbuff;


if (request.method == 'G') {
printf ("GET: %s\n", request.url);

switch (request.mode) {

case file:

if (request.type == reg || request.type == altreg) {
	serv_file (request);
}else if (request.type == dir || request.type == altdir) {
	serv_dir (request);
}else {
	send_txt (connfd, "Bad Resource");
}// if else if type

break;
case edit:
	get_edit (request);


break;
case action:
	ret = get_action (request);
break;
case favicon:
	servico (connfd);

break;
case err:
	send_err (connfd, 410);


break;
default:
printf ("default\n");
} // END GET switch

} else if (request.method == 'P') {
printf ("POST: %s\n", request.url);

if (request.mode == edit)
	post_edit (request);


//if (request.method == 'U' && request.mode == edit)
//	procint = put_edit (request);

//if (request.method == 'O' && request.mode == upload)
//	procint = put_file (request);
//if (request.method == 'U' && request.mode == upload)
//	procint = put_file (request);

//    procint = post_action (request, inbuff);

//{ softclose (connfd, &inbuff); break;}

} // else if POST


} // keep alive loop
} // main loop
} // main

int serv_dir (const struct request_data request)
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
buffcatf (&out, "<form enctype=\"multipart/form-data\" action=\"%s\" method=\"post\">\n", request.url);
buffcatf (&out, "<input type=\"file\" class=\"button\" name=\"myFile\">\n");
buffcatf (&out, "<input type=\"submit\" class=\"button\"  value=\"upload\">\n</form>\n");
buffcatf (&out, "</div>\n");

while ((ep = readdir (dp)))
{
if (ep->d_name[0] == '.')
	continue;
	
// 4 is dir 8 is file
printf ("dir: %s\n", ep->d_name);

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
sock_writeold (request.fd, head.p, head.len);
sock_buffwrite (request.fd, &out);

closedir (dp);

return 1;
} // serv_dir

int serv_file (const struct request_data request)
{
//int dot = getlast (request.path, (int) '.', strlen (request.path));
//char outbuffer [string_sz];
struct string_data outbuff;
//outbuff.p = outbuffer;
//outbuff.len = 0;
//outbuff.max = string_sz;

//char request.ext[10] = "";
const char *mime_txt;

//strncpy (request.ext, request.path + dot, strlen (request.path) - dot);

//int i = 0;

if (strcmp(request.ext, ".txt") == 0) {
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

}else {mime_txt = conttxt;}

//printf ("%s\n", mime_txt);

//struct stat finfo;
//stat (request.full_path, &finfo);

outbuff.len = sprintf (outbuff.p, "%s%s%s%ld\n\n", hthead, mime_txt, contlen, request.content_len);

//printf ("%ld bytes: %s", request.content_len, mime_txt);

sock_writeold (request.fd, outbuff.p, outbuff.len);

return send_file (request.full_path, request.fd);
//return 1;

} // serv_file`



int process_request (struct request_data *request, const int fd, const struct buffer_data inbuff)
{ // bm process_request
memset (request, 0, sizeof (struct request_data));

request->method = inbuff.p [0];
request->fd = fd;
//request->mode = file;

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

} // if uri params

p1 = strstr (request->url, edit_mode);
if (p1 != NULL) {
path_start = edit_mode_len;
request->mode = edit;
} // if edit_mode

p1 = strstr (request->url, action_mode);
if (p1 != NULL) {
path_start = action_mode_len;
request->mode = action;
} // if action_mode

p1 = strstr (request->url, file_mode);
if (p1 != NULL) {
path_start = file_mode_len;
request->mode = file;
} // if action_mode

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

if (request->method == 'P') {
int termlen = strlen ("Content-Length: ");
char *p1 = (char *) strcasestr (inbuff.p, "Content-Length: ");
if (p1 == NULL) killme ("error locating length");

int d1 = p1 - inbuff.p;

char *p2 = strchr (inbuff.p + d1, 10);
int d2 = p2 - inbuff.p;

char temp [20];
memset (temp, 0, 20);

int len = d2 - d1;
memcpy (temp, inbuff.p + d1 + termlen, len - termlen - 1);

request->content_len = atol (temp);
//printf ("[%lu]\n", request->content_len);
} //if post  get cont len

//attempt to stat file for GET requests
sprintf (request->full_path, "%s%s", settings.base_path, request->path);
//printf ("statting regular file :[%s]\n", request->full_path);
struct stat finfo;
if (stat (request->full_path, &finfo) == 0)
{
if (request->method == 'G') request->content_len = finfo.st_size;
//printf ("(base) running stat on: %s\n", request->full_path);
	if (S_ISDIR(finfo.st_mode))
		{request->type = dir;}

	if (S_ISREG(finfo.st_mode)) // is file
		{request->type = reg;}
return 1;
} // end if 

sprintf (request->full_path, "%s%s", settings.internal, request->path);
//printf ("(alt) running stat on: %s\n", request->full_path);
if (stat (request->full_path, &finfo) == 0)
{
if (request->method == 'G') request->content_len = finfo.st_size;
	if (S_ISDIR(finfo.st_mode))
		{request->type = altdir; printf ("is altdir\n");}

	if (S_ISREG(finfo.st_mode)) // is file
		{request->type = altreg; printf ("is altreg\n");}

return 1;
}else {
if (request->method == 'G') request->mode = err; 
}
//end if

//send_txt (fd, request->path);
return 1;
} // process_request

int get_edit (const struct request_data request)
{ // bm get_edit
buffer_t editor;
buffer_t filedata;

{
struct stat finfo;
if (stat (settings.editor_path, &finfo)) {send_txt (request.fd, "cant stat Editor"); return 0;}
editor = init_buffer (finfo.st_size);
int editor_fd = open (settings.editor_path, O_RDONLY);
if (editor_fd < 0) {send_txt (request.fd, "cant open Editor"); return 0;}
editor.len = read (editor_fd, editor.p, editor.max);
editor.p[editor.len] = 0;
close (editor_fd);


if (stat (request.full_path, &finfo)) {send_txt (request.fd, "cant stat FILE"); return 0;}
filedata = init_buffer (finfo.st_size);
int file_fd = open (request.full_path, O_RDONLY);
if (file_fd < 0) {send_txt (request.fd, "cant open FILE"); return 0;}
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
save_buffer (bookmarks, "bookmarks.txt");
int req_len = editor.len + filedata.len + bookmarks.len;

//editor.p = realloc (editor.p, req_len);
//if (editor.p == NULL) killme ("no realloc");
//editor.max = req_len;
FAR (&editor, "<!--bookmarks-->", bookmarks);
//buffer_t encoded = init_buffer (0);
buffer_t encoded = HTML_encode (filedata, 1);
save_buffer (encoded, "encodedhtm.txt");
FAR (&editor, "DELIMETER", encoded);

save_buffer (editor, "editor.txt");
struct string_data head;
head.len = sprintf (head.p, "%s%s%s%d\n\n", hthead, conthtml, contlen, editor.len);

printf ("%d bytes: edit file served\n", editor.len);

sock_writeold (request.fd, head.p, head.len);
sock_buffwrite (request.fd, &editor);

save_buffer (editor, "get_edit.txt");

free (filedata.p);
free (editor.p);
free (encoded.p);
free (bookmarks.p);
//send_txt (request.fd, "wow");

return 1;
} // get_edit


int post_edit (const struct request_data request)
{
//save_buffer (request.mainbuff, "POST_EDIT.txt");

//int fd1 = open ("POST_EDIT.txt", O_WRONLY | O_TRUNC| O_CREAT, S_IRUSR | S_IWUSR);
//if (fd1 == -1) killme ("opening file");
//write (fd1, request.mainbuff->p, request.mainbuff->len);
//close (fd1);

int progress = 0;

buffer_t encoded = init_buffer (request.content_len);
//memset (encoded.p, 0, encoded.max);

char *p1 = strchr (request.mainbuff->p, (int) '\"');
if (p1 != NULL) {
printf ("started in 1st xmission\n");
int d1 = p1 - request.mainbuff->p;
memcpy (encoded.p, request.mainbuff->p + d1, request.mainbuff->len - d1);
encoded.len = request.mainbuff->len - d1;
progress = request.mainbuff->len - d1;
}
//printf ("content len: %lu, progress: %d\n", request.content_len, encoded.len);

/*
if (request.content_len == progress) {
printf ("fully contained in first xmission \"\n");
encoded.len -= 2;
}
*/


while (progress < request.content_len) {
encoded.len += sock_read (request.fd, encoded.p + encoded.len, encoded.max);
//printf ("multi-reciever\n");
progress = encoded.len;
}
//printf ("finished cat json\n[%.*s]\n", encoded.len, encoded.p);

save_buffer (encoded, "encoded.txt");

char backup [string_sz];
strcpy (backup, "old/");
strcat (backup, request.filename);
strcat (backup, "-%H-%M");
strcat (backup, request.ext);

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

send_txt (request.fd, "it worked");
free (encoded.p);
free (decoded.p);


return 1;

} //post edit


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


buffcatf (&out, "uri: %s<br>\n path: %s<br>\n fpath: %s<br>\n", request.url, request.path, request.full_path);
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
  sprintf (rtn, "%s/%s", request.full_path, fname);
  struct stat finfo;
  int check = stat (rtn, &finfo);
  if (check != 0) return;

 int copynum = 1;
 while (check == 0)
 {
    sprintf (rtn, "%s/%d-%s",request.full_path, copynum, fname);  
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
charfull_path [strig_sz];
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

if (safename) safe_fname (request, fname, full_path);
else sprintf (full_path, "%s/%s",request.full_path, fname);

localfd = open (full_path, O_WRONLY | O_TRUNC| O_CREAT, S_IRUSR | S_IWUSR);
	if (localfd ==-1) {printf ("error 1160\n"); exit(0);}
filenum = 1;
fsize = get_nextsize (&fdata);
buffcatf (&audit, "file opened: %s, %d: bytes\n", full_path, fsize);


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

char full_path [string_sz];
//if (safename) safe_fname (request, fname, full_path);
//else 
sprintf (full_path, "%s/%s", request.full_path, fname);
printf ("full_path: %s\n", full_path);

localfd = open (full_path, O_WRONLY | O_TRUNC| O_CREAT, S_IRUSR | S_IWUSR);
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
