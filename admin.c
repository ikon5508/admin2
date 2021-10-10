#include "socket.h"
#include "admin.h"
#include "logging.h"
#include "libmemory.h"

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

sock_write (fd, outbuff.p, outbuff.len);


} // send_txt

int serv_file (const struct args_data args, const struct request_data request, const size_t size)
{
int dot = getlast (request.path.p, '.', request.path.len);


char outbuffer [string_sz];
struct buffer_data outbuff;
outbuff.p = outbuffer;
outbuff.len = 0;
outbuff.max = string_sz;

char mime_ext[10] = "";
const char *mime_txt;

midstr (request.path.p, mime_ext, dot, request.path.len);

int i = 0;

if (!strcmp(mime_ext, ".txt"))
{i=1;  mime_txt = conttxt;}


if (!strcmp(mime_ext, ".htm"))
{i=1;  mime_txt = conthtml;}


if (!strcmp(mime_ext, ".html"))
{i=1;  mime_txt = conthtml;}

if (!strcmp(mime_ext, ".js"))
{i=1;  mime_txt = contjava;}

if (!strcmp(mime_ext, ".jpg"))
{i=1;  mime_txt = contjpg;}

if (!strcmp(mime_ext, ".jpeg"))
{i=1;  mime_txt = contjpg;}

if (!strcmp(mime_ext, ".css"))
{i=1;  mime_txt = contcss;}

if (!i)
{mime_txt = conttxt;}

//struct stat finfo;
//stat (request.fullpath.p, &finfo);
loggingf ("%d bytes: %s", size, mime_txt);

outbuff.len = sprintf (outbuff.p, "%s%s%s%d\n\n", hthead, mime_txt, contlen, size);

sock_write (request.fd, outbuff.p, outbuff.len);

sendfileold (request.fullpath.p, request.fd);

}

int serv_dir (const struct args_data args, const struct request_data request)
{
DIR *dp;
struct dirent *ep;
/*
 init_page (&outbuff, 0, 1);

dp = opendir (request.path.p);
if (dp != NULL)
{

stradd (outbuff.p, "<a href=\"", &outbuff.len);

if (request.mode == dir)
{
// if dir mode, add full path
stradd (outbuff.p, request.uri.p, &outbuff.len);
outbuff.p[outbuff.len] = '/'; ++outbuff.len;
} // if dir mode add full path
stradd (outbuff.p, settings.upuri, &outbuff.len);
stradd (outbuff.p, "\">", &outbuff.len);
stradd (outbuff.p, settings.uphandle, &outbuff.len);
stradd (outbuff.p, "</a><br>\n", &outbuff.len);
// end add upload handle

while (ep = readdir (dp))
{
//if (ep->d_type == 4)
// 4 is dir 8 is file
//ep->d_name
if (ep->d_name[0] == '.')
	continue;

stradd (outbuff.p, "<a href=\"", &outbuff.len);
if (request.mode == dir)
{
// if dir mode, add full path
stradd (outbuff.p, request.uri.p, &outbuff.len);
outbuff.p[outbuff.len] = '/'; ++outbuff.len;
} // if dir mode add full path
stradd (outbuff.p, ep->d_name, &outbuff.len);
stradd (outbuff.p, "\">", &outbuff.len);
stradd (outbuff.p, ep->d_name, &outbuff.len);

if (ep -> d_type == 4)
{
// if directory add following /
outbuff.p[outbuff.len] = '/';
++outbuff.len;
} // if dir
stradd (outbuff.p, "</a><br>\n", &outbuff.len);

} // while ep ! NULL
} // if != dp null

stradd (outbuff.p, "</body>\n</html>", &outbuff.len);
 closedir (dp);

 fix_len(&outbuff);

*/

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

} //get_file


struct request_data process_request (const int fd, const struct args_data args, const struct buffer_data inbuff);
int get_edit_file (const struct args_data args, const struct request_data request);

struct args_data process_args (int argc, char **argv)
{
struct args_data args;

//set default values
args.port = 9999;
strcpy (args.base_path.p, ".");
strcpy (args.editor_path.p, "aceeditor.htm");
args.backdoor = 1;
// set defaults

// cycle through command line args
for (int i = 1; i < argc; ++i)
{
int argvlen = strlen(argv[i + 1]);

if (!strcmp (argv[i], "-backdoor"))
{ args.backdoor = 1; continue; }

if (!strcmp (argv[i], "-port"))
{ args.port = atoi (argv[i + 1]); ++i; continue; }

if (!strcmp (argv[i], "-path"))
{ strcpy (args.base_path.p, argv[i + 1]); ++i; continue; }

if (!strcmp (argv[i], "-editor"))
{ strcpy (args.editor_path.p, argv[i + 1]); ++i; continue; }
} // for

return args;
} //process_args




int post_edit (const struct buffer_data mainbuff, const struct request_data request)
{
int d1, d2;
int startdata = - 1;
int enddata = - 1;
int fbound =-1, rbound=-1;
int localfd = - 1;
const int saveold = 1;

char bd [string_sz];
struct buffer_data inbuff;
inbuff.p = bd;
inbuff.max = string_sz;

char fdata [string_sz];
struct buffer_data filedata;
filedata.p = fdata;
filedata.max = string_sz;
filedata.len = 0;

char newfname [string_sz];
strcpy (newfname, request.fullpath.p);
strcat (newfname, ".new");

char oldfname [string_sz];
strcpy (oldfname, request.fullpath.p);
strcat (oldfname, ".old");

fbound = search (mainbuff.p, request.boundary, request.procint, mainbuff.len);
if (fbound > - 1)
{
// get past name data, since only one object is sent
d1 = getnext (mainbuff.p, (char) 10, fbound, mainbuff.len);
d2 = getnext (mainbuff.p, (char) 10, d1 + 1, mainbuff.len);

startdata = d2;
while (mainbuff.p [startdata] == 10 || mainbuff.p [startdata] == 13 || mainbuff.p [startdata] == '.')
    ++startdata;

rbound = search (mainbuff.p, request.boundary, fbound + 1, mainbuff.len);
if (rbound > - 1)
{
enddata = rbound - request.boundlen;

while (mainbuff.p [enddata] == 10 || mainbuff.p [enddata] == 13 || mainbuff.p [enddata] == '-')
    --enddata;

++enddata;

} // if rbound
}// if fbound
// done inumerating start and enddata in mainbuffer

if (startdata > - 1)
{
if (enddata == - 1)
    enddata = mainbuff.len;

for (int i = startdata; i < enddata; ++i)
{
filedata.p [filedata.len] = mainbuff.p [i];
++filedata.len;
} // for

//send_txt (request.fd, filebuffer.p, filebuffer.len);
if (saveold) rename (request.fullpath.p, oldfname);

localfd = open (newfname, O_WRONLY | O_TRUNC| O_CREAT, S_IRUSR | S_IWUSR);
if (localfd < 0)  {send_txt(request.fd,"1, error opening newfile",0); return -1; }

filedata.procint = write (localfd, filedata.p, filedata.len);
if (filedata.procint != filedata.len)  {send_txt(request.fd,"error writing single reciever",0); return -1; }

if (rbound > - 1)
{
close (localfd);
rename (newfname, request.fullpath.p);
send_txt (request.fd, "File recieved, single reciever", 0);
return 1;
} // if
} // if startdata
// if single reciever

// if fbound null then get it.
// cycle through favico if necessary - carry on
//     
int cnt = 0;

if (startdata == -1)
{
while (fbound == -1)
{
inbuff.len = sock_read (request.fd, inbuff.p, inbuff.max);
if (inbuff.len == -1) {send_txt(request.fd,"1, reading timout",0); return -1; }

fbound = search (inbuff.p, request.boundary, 0, inbuff.len);
++cnt;
if (cnt == 50) {send_txt(request.fd,"1, 1st boundary not found",0); return -1; }


} // while fbound - 1

d1 = getnext (inbuff.p, (char) 10, fbound, inbuff.len);
d2 = getnext (inbuff.p, (char) 10, d1 + 1, inbuff.len);

startdata = d2;
while (inbuff.p [startdata] == 10 || inbuff.p [startdata] == 13 || inbuff.p [startdata] == '.')
    ++startdata;
}else{
startdata = 0;
} // if

if (localfd == -1)
{
if (saveold) rename (request.fullpath.p, oldfname);
localfd = open (newfname, O_WRONLY | O_TRUNC| O_CREAT, S_IRUSR | S_IWUSR);
if (localfd < 0)  {send_txt(request.fd,"2, error opening newfile",0); return -1; }
} // if

// while loop until rbound > 0
while (rbound == -1)
{
rbound = search (inbuff.p, request.boundary, startdata +1, inbuff.len);
if (rbound == -1) enddata = inbuff.len;

if (rbound > 0)
{
enddata = inbuff.len - request.boundlen;

while (inbuff.p [enddata] == 10 || inbuff.p [enddata] == 13 || inbuff.p [enddata] == '-')
    --enddata;

++enddata;
} // if rbound

filedata.len = 0;
for (int i = startdata; i < enddata; ++i)
	{ filedata.p [filedata.len] = inbuff.p [i]; ++filedata.len; }

filedata.procint = write (localfd, filedata.p, filedata.len);
if (filedata.procint != filedata.len)  {send_txt(request.fd,"error writing multipart-reciever",0); return -1; }

if (rbound == -1)
	inbuff.len = sock_read (request.fd, inbuff.p, inbuff.max);

	// work here
} // while rbound

close (localfd);
rename (newfname, request.fullpath.p);
send_txt (request.fd, "file recieved, multi-part reciever",0);
return 1;
} // post edit


int main (int argc, char **argv)
{
struct args_data args = process_args (argc, argv);

init_log ("log.txt");

loggingf ("admin load\nPort: %d\nPath: %s\nEditor: %s\n", args.port, args.base_path.p, args.editor_path.p);

if (args.backdoor)
    init_sockbackdoor ("bd.txt");

struct sockaddr_in address;
socklen_t addrlen = sizeof(address);

int servfd = prepsocket (args.port);
int loop = 1;

char inbuffer [string_sz];
struct buffer_data inbuff;
inbuff.p = inbuffer;
inbuff.max = (string_sz);

struct request_data request;

while (loop)
{
loggingf ("waiting\n");

int connfd = accept(servfd, (struct sockaddr *)&address, (socklen_t*)&addrlen);
sock_setnonblock (connfd);

inbuff.len = sock_read (connfd, inbuff.p, inbuff.max);

request = process_request (connfd, args, inbuff);
loggingf ("URI request: %s\n", request.uri.p);

int procint;

if (request.method == 'G' && request.mode == edit)
    procint = get_edit_file (args, request);


if (request.method == 'G' && request.mode == file)
    procint = get_file (args, request);


if (request.method == 'P' && request.mode == edit)
    procint = post_edit (inbuff, request);

shutdown (connfd,  SHUT_WR);

//wait for client to close connection
char tbuff [100];
int a;

while (1) 
{
a=read(connfd, tbuff, 100);

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

editorbuffer.procint = search (editorbuffer.p, "DELIMETER", 0, editorbuffer.len);
if (editorbuffer.procint < 0)
    {send_txt (request.fd, "failed to find DELIMETER", 0); return -1;}

for (int i = 0; i < (editorbuffer.procint - 8); ++i)
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

for (int i = (editorbuffer.procint + 1); i < editorbuffer.len; ++i)
{
outbuff.p [outbuff.len] = editorbuffer.p [i];
++outbuff.len;
}

struct string_data head;
head.len = sprintf (head.p, "%s%s%s%d\n\n", hthead, conthtml, contlen, outbuff.len);



//char diagnostic [maxbuffer];
//sprintf (diagnostic, "URI(%d): %s\n Path: %s Fullpath: %s", request.uri.len, request.uri.p, request.path.p, request.fullpath.p);
//send_txt (fd, outbuff.p, outbuff.len);
sock_write (request.fd, head.p, head.len);
sock_write (request.fd, outbuff.p, outbuff.len);
return 1;

} // get_edit_file

struct request_data process_request (const int fd, const struct args_data args, const struct buffer_data inbuff)
{
struct request_data request;
request.method = inbuff.p [0];
request.fd = fd;

int d1 = getnext (inbuff.p, (char) 32, 0, inbuff.len);
++d1;
int d2 = getnext (inbuff.p, (char) 32, d1, inbuff.len);

request.uri.len = midstr (inbuff.p, request.uri.p, d1, d2);

d1 = search (request.uri.p, "/edit", 0, 6);
if (d1 > 0)
{
request.mode = edit;

request.path.len = midstr (request.uri.p, request.path.p, (d1 + 1), request.uri.len);

} // if edit

d1 = search (request.uri.p, "/file", 0, 6);
if (d1 > 0)
{
request.mode = file;

request.path.len = midstr (request.uri.p, request.path.p, (d1 + 1), request.uri.len);

} // if file

request.fullpath.len = sprintf (request.fullpath.p, "%s%s", args.base_path.p, request.path.p);

if (request.method == 'P')
{
d1 = search (inbuff.p, "boundary=", 0, inbuff.len);
d2 = getnext (inbuff.p, (char) 10, d1, inbuff.len);
request.boundlen = midstr (inbuff.p, request.boundary, d1 + 1, d2);
request.procint = d2;
} // if post get boundary


/*

{err, action, file, edit, upload, config, root} mode;
};
*/

return (request);
} // process_request
