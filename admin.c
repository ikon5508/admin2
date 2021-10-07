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



int get_file (const struct args_data args, const struct request_data request)
{
char outbuffer [string_sz];
struct buffer_data outbuff;
outbuff.p = outbuffer;
outbuff.len = 0;
outbuff.max = string_sz;

struct stat finfo;
stat(request.fullpath.p, &finfo);

int dot = getlast (request.path.p, '.', request.path.len);

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

outbuff.len = sprintf (outbuff.p, "%s%s%s%d\n\n", hthead, mime_txt, contlen, finfo.st_size);

sock_write (request.fd, outbuff.p, outbuff.len);

sendfile (request.fullpath.p, request.fd);

} //get_file


struct request_data process_request (const int fd, const struct args_data args, const struct buffer_data inbuff);
int get_edit_file (const struct args_data args, const struct request_data request);
int post_edit (const struct buffer_data initbuff, const struct request_data request);

int main (int argc, char **argv)
{
struct args_data args;
memset (&args, 0, sizeof (args));

loggingf (1, "admin load\n");

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

if (args.backdoor)
    init_sockbackdoor ("bd.txt");

init_log ("log.txt");

struct sockaddr_in address;
socklen_t addrlen = sizeof(address);

int servfd = prepsocket (args.port);
int loop = 1;

char inbuffer [string_sz * 2];
struct buffer_data inbuff;
inbuff.p = inbuffer;
inbuff.max = (string_sz * 2);

struct request_data request;

while (loop)
{
int connfd = accept(servfd, (struct sockaddr *)&address, (socklen_t*)&addrlen);
sock_setnonblock (connfd);

inbuff.len = sock_read (connfd, inbuff.p, inbuff.max);

request = process_request (connfd, args, inbuff);

if (request.method == 'G' && request.mode == edit)
    get_edit_file (args, request);


if (request.method == 'G' && request.mode == file)
    get_file (args, request);


if (request.method == 'P' && request.mode == edit)
    post_edit (inbuff, request);

shutdown (connfd,  SHUT_WR);

//wait for client to close connection
char tbuff [100];
int a;

while (1) 
{
a=read(connfd, tbuff, 100);

if (a > 0)
loggingf (1, "reading \"success\" in kill mode");

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
loggingf (1, "Connection closed by client");
break;
} // if socket non blocked
} // for wait for close bit.

close(connfd);


} // main loop
} // main




int post_edit (const struct buffer_data inbuff, const struct request_data request)
{
int d1, d2;
int startdata;
int fbound, rbound;

fbound = search (inbuff.p, request.boundary, request.code, inbuff.len);
if (fbound == -1)
    {send_txt (request.fd, "error locating first boundary", 0); return (-1);}

// get past name data, since only one object is sent
d1 = getnext (inbuff.p, (char) 10, fbound, inbuff.len);
d2 = getnext (inbuff.p, (char) 10, d1 + 1, inbuff.len);
startdata = d2;
while (inbuff.p [startdata] == 10 || inbuff.p [startdata] == 13 || inbuff.p [startdata] == '.')
    ++startdata;

rbound = search (inbuff.p, request.boundary, fbound + 1, inbuff.len);

//rear boundary found in first transmission
if (rbound > 0)
{
rbound -= request.boundlen;

while (inbuff.p [rbound] == 10 || inbuff.p [rbound] == 13 || inbuff.p [rbound] == '-')
    --rbound;

++rbound;

struct string_data filebuffer;
filebuffer.len = 0;

for (int i = startdata; i < rbound; ++i)
{
filebuffer.p [filebuffer.len] = inbuff.p [i];
++filebuffer.len;
} // for

//send_txt (request.fd, filebuffer.p, filebuffer.len);
send_txt (request.fd, "File recieved, single reciever", 0);

int localfd = open (request.fullpath.p, O_WRONLY | O_TRUNC| O_CREAT, S_IRUSR | S_IWUSR);
write (localfd, filebuffer.p, filebuffer.len);
close (localfd);

return 1;
} // if rear bound found

// no rear boundary (multi-part reciever
if (rbound == -1)
{

char tempfile [string_sz];
strcpy (tempfile, request.fullpath.p);
strcat (tempfile, ".new");

int localfd = open (tempfile, O_WRONLY | O_TRUNC| O_CREAT, S_IRUSR | S_IWUSR);

char mnb [string_sz * 2];
struct buffer_data filebuffer;
filebuffer.p = mnb;
filebuffer.max = string_sz * 2;
filebuffer.len = 0;

for (int i = startdata; i < inbuff.len; ++i)
{
filebuffer.p [filebuffer.len] = inbuff.p [i];
++filebuffer.len;
} // for

write (localfd, filebuffer.p, filebuffer.len);

while (rbound == -1)
{
filebuffer.len = sock_read (request.fd, filebuffer.p, filebuffer.max);

rbound = search (filebuffer.p, request.boundary, (filebuffer.len > 100)? filebuffer.len - 100: 0, filebuffer.len);

if (rbound == -1)
    write (localfd, filebuffer.p, filebuffer.len);


if (rbound > 0)
{
filebuffer.len -= request.boundlen;

while (filebuffer.p [filebuffer.len] == 10 || filebuffer.p [filebuffer.len] == 13 || filebuffer.p [filebuffer.len] == '-')
--filebuffer.len;

++filebuffer.len;
write (localfd, filebuffer.p, filebuffer.len);

} // if



//if (rbound > 0)
  //  {send_txt (request.fd, "rear boundary found!!!", 0); exit (0);}


} // while


close (localfd);
rename (tempfile, request.fullpath.p);

send_txt (request.fd, "rear boundary found, file completed", 0); exit (0);


} // if multipart reciever


} // post edit





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
//printf ("procint: %d\nlen: %d\n", editorbuffer.procint, editorbuffer.len);


for (int i = 0; i < (editorbuffer.procint - 8); ++i)
{
outbuff.p [outbuff.len] = editorbuffer.p [i];
++outbuff.len;
}

for (int i = 0; i < filebuffer.len; ++i)
{
outbuff.p [outbuff.len] = filebuffer.p [i];
++outbuff.len;
}

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


} // edit

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
request.code = d2;
} // if post get boundary


/*

{err, action, file, edit, upload, config, root} mode;
};
*/

return (request);
} // process_request
