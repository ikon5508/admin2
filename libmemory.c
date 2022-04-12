#include "libmemory.h"
#include <poll.h>
const int timeout = 3;

int bdcount = 1;
int fd = 0;
int printfon = 1;

int backdoor = 0;
int backdoorfd = 0;
char bd[nameholder];


void send_kill (const int port)
{
loggingf ("port busy, killing previous process\n");

char buffer [nameholder];

int sockfd, portno, n;

struct sockaddr_in serv_addr;
struct hostent *server;

sockfd = socket(AF_INET, SOCK_STREAM, 0);
if (sockfd < 0) 
    {printf ("ERROR client socket\n"); exit (0);}
server = gethostbyname("localhost"); // get hostbyname

if (server == NULL) {
        fprintf(stderr,"ERROR, no such host\n");
        exit(0);
}

bzero((char *) &serv_addr, sizeof(serv_addr));
serv_addr.sin_family = AF_INET;

bcopy((char *)server->h_addr, (char *)&serv_addr.sin_addr.s_addr, server->h_length);
    
serv_addr.sin_port = htons(port);

if (connect(sockfd,(struct sockaddr *)&serv_addr,sizeof(serv_addr)) < 0) 
        {printf ("ERROR connecting"); exit (0);}

strcpy (buffer, "----kill----");
write(sockfd, buffer, strlen(buffer));


n = read(sockfd, buffer, nameholder);

//if (!strcmp (buffer, "dead"))
}

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

int init_buffer (struct buffer_data *buffer, const int size)
{
buffer->p = (char *) malloc (size);
if (buffer->p == NULL)
    {buffer->max = 0; return 0;}
    
    buffer->max = size;

    return size;
} // init_buffer


int ugetnext (const unsigned char *str, const int next, const int start, int end)
{
const int debug = 0;

//if (end == 0)
//end = strlen (str);

for (int i = start; i < end; ++i)
{
    
if (debug)
printf ("c: %c - %c\n", str[i], next);
        
if (str[i] == next)
	return i;
} // for

    return  -1;
} // ugetnext

int getnext (const char *str, const int next, const int start, int end)
{
const int debug = 0;

if (end == 0)
end = strlen (str);

for (int i = start; i < end; ++i)
{
    
if (debug)
printf ("c: %c - %c\n", str[i], next);
        
if (str[i] == next)
	return i;
} // for

    return  -1;
} // getnext


int midstr(const char *major, char *minor, int start, const int end)
{
int count = 0;
while (start < end)
{
minor[count] = major[start];
++start;
++count;
}
minor[count] = 0;
return (count);
} // end midstr

int umidstr(const unsigned char *major, char *minor, int start, const int end)
{
int count = 0;
while (start < end)
{
minor[count] = major[start];
++start;
++count;
}
minor[count] = 0;
return (count);
} // end umidstr

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


int search (const char *main, const char *minor, const int start, const int end)
{
int minorlen = strlen (minor) -1;

int i, x;
for (i = start; i < end; ++i)
{
if (main[i] == minor[0])
{
for (x = 1; x < minorlen; ++x)
{
	
if (main [i + x] != minor [x])
 break; // if minor mismatch

} // for x
if (x == minorlen)
return (i + x + 1);

} // end if minor [0] hit

} // end for i

return -1;
} // end search

int usearch (const unsigned char *main, const char *minor, const int start, const int end)
{
int minorlen = strlen (minor) -1;

int i, x;
for (i = start; i < end; ++i)
{
if (main[i] == minor[0])
{
for (x = 1; x < minorlen; ++x)
{
	
if (main [i + x] != minor [x])
 break; // if minor mismatch

} // for x
if (x == minorlen)
return (i + x + 1);

} // end if minor [0] hit

} // end for i

return -1;
} // end usearch

int init_sockbackdoor (const int type, const char *init)
{
backdoor = type;
string path;

sprintf (path.p, "sandbox/%s.txt", init);
strcpy (bd, init);

if (type == 1 || type == 2)
{

backdoorfd = open (path.p, O_WRONLY | O_TRUNC| O_CREAT, S_IRUSR | S_IWUSR);
if (backdoorfd == -1) {printf ("error libmem 280\n");}
    
    
} // if
loggingf ("socket backdoor init\n");

return 1;

} // init_sockbackdoor


int sendfileold (const char *path, const int fd)
{
int locfd = open (path, O_RDONLY);
if (locfd < -1)
    return -1;

struct stat finfo;
fstat (locfd, &finfo);

size_t read_progress = 0;

size_t write_progress = 0;

size_t fsize = finfo.st_size;
//loggingf (200, "file size: %d\n", fsize);

char c_fbuffer [500];
struct buffer_data fbuff;
fbuff.p = c_fbuffer;
fbuff.max = 500;


while (read_progress < fsize)
{  
fbuff.len = read (locfd, fbuff.p, fbuff.max);
read_progress += fbuff.len;

int interim_progress = sock_writeold (fd, fbuff.p, fbuff.len);
if (interim_progress == -1)
	return -1;
while (interim_progress < fbuff.len)
{
int cpylen = fbuff.len - interim_progress;

for (int i = interim_progress; i < fbuff.len; ++i)
fbuff.p [i - interim_progress] = fbuff.p [i];

fbuff.len = cpylen;

interim_progress = sock_writeold (fd, fbuff.p, fbuff.len);
if (interim_progress == -1)
	return -1;
} // while

} // while loop
close (locfd);
return 1;
} // sendfileold

int sendfileM (const char *path, const int fd)
{
int locfd = open (path, O_RDONLY);
if (locfd < -1)
    return -1;

struct stat finfo;
fstat (locfd, &finfo);

size_t read_progress = 0;

size_t write_progress = 0;

size_t fsize = finfo.st_size;
//loggingf (200, "file size: %d\n", fsize);

char c_fbuffer [500];
struct buffer_data fbuff;
fbuff.p = c_fbuffer;
fbuff.max = 500;


while (read_progress < fsize)
{  
fbuff.len = read (locfd, fbuff.p, fbuff.max);
read_progress += fbuff.len;

int interim_progress = sock_writeold (fd, fbuff.p, fbuff.len);
if (interim_progress == -1)
	return -1;

while (interim_progress < fbuff.len)
{
int cpylen = fbuff.len - interim_progress;
//for (int i = interim_progress; i < fbuff.len; ++i)
//fbuff.p [i - interim_progress] = fbuff.p [i];
//fbuff.p += interim_progress; // doesnt work
memmove (fbuff.p, fbuff.p + interim_progress, cpylen);
fbuff.len = cpylen;

interim_progress = sock_writeold (fd, fbuff.p, fbuff.len);
if (interim_progress == -1)
	return -1;

} // while

} // while loop
close (locfd);
return 1;
} // sendfileM

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
//loggingf (200, "file size: %d\n", fsize);

char c_fbuffer [sendfileunit];
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
	//int cpylen = fbuff.len - interim_progress;
//for (int i = interim_progress; i < fbuff.len; ++i)
//fbuff.p [i - interim_progress] = fbuff.p [i];
//fbuff.p += interim_progress; // doesnt work
//memmove (fbuff.p, fbuff.p + interim_progress, cpylen);
//fbuff.len = cpylen;

interim_progress = sock_writeold (fd, fbuff.p + cpylen, fbuff.len - cpylen);
if (interim_progress == -1)
	return -1;
cpylen += interim_progress;
} // while

} // while loop
close (locfd);
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

if (backdoor ==3) return;

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


int sock_writeold (const int connfd, const char *buffer, int size)
{ // bm sock writeold

if (size == 0)
    size = strlen (buffer);

if (backdoor == 3)
    return size;

struct pollfd ev;
ev.fd = connfd;
ev.events = POLLOUT;

int prtn = poll (&ev, 1, timeout * 1000);
if (prtn < 1) return -1;

return write (connfd, buffer, size);

} // sock_write_old



int usock_read (const int connfd, unsigned char *buffer, const int size)
{
if (backdoor ==3)
{
loggingf ("sb fetch: %s-%d\n", bd, bdcount);
string temp;
sprintf (temp.p, "sandbox/%s-%d.txt", bd, bdcount);

int tfd = open (temp.p, O_RDONLY);
if (tfd == -1) {printf ("592 end of sandbox files: %s\n", temp.p); exit(0);}
int rlen = read (tfd, buffer, size);

close (tfd);
++bdcount;
return rlen;
    
} // if sandbox mode



time_t basetime;
time (&basetime);

int len = -1;
//read (connfd, inb->data, in_size);
while (len < 0)
{
len = read (connfd, buffer, size);

if (len == -1)
{
usleep (1000);
time_t deadtime;
time (&deadtime);

deadtime -= basetime;
if (deadtime >= timeout)
	{return -1;}

} // if -1

	
	
} // while


if (backdoor == 2)
{
string temp;
sprintf (temp.p, "sandbox/%s-%d.txt", bd, bdcount);

int tfd = open (temp.p, O_WRONLY | O_TRUNC | O_CREAT, S_IRUSR | S_IWUSR);
if (tfd == -1) {printf ("error opening log file 585\n"); return len;}
write (tfd, buffer, len);
close (tfd);
++bdcount;

write (backdoorfd, "\n....read....\n", 14);
write (backdoorfd, buffer, len);
}else if (backdoor == 1)
{
write (backdoorfd, "\n....read....\n", 14);
write (backdoorfd, buffer, len);

} // if backdoor



	
return len;
} // sock_read



int sock_read (const int connfd, char *buffer, const int size)
{
if (backdoor ==3)
{
string temp;
sprintf (temp.p, "sandbox/%s-%d.txt", bd, bdcount);

loggingf ("sb fetch: %s-%d\n", bd, bdcount);

int tfd = open (temp.p, O_RDONLY);
if (tfd == -1) {printf ("592 end of sandbox files: %s\n", temp.p); exit(0);}
int rlen = read (tfd, buffer, size);

close (tfd);
++bdcount;
return rlen;
    
} // if sandbox mode



time_t basetime;
time (&basetime);

int len = -1;
//read (connfd, inb->data, in_size);
while (len < 0)
{
len = read (connfd, buffer, size);

if (len == -1)
{
usleep (1000);
time_t deadtime;
time (&deadtime);

deadtime -= basetime;
if (deadtime >= timeout)
	{return -1;}

} // if -1

	
	
} // while


if (backdoor == 2)
{
string temp;
sprintf (temp.p, "sandbox/%s-%d.txt", bd, bdcount);

int tfd = open (temp.p, O_WRONLY | O_TRUNC | O_CREAT, S_IRUSR | S_IWUSR);
if (tfd == -1) {printf ("error opening log file 585\n"); return len;}
write (tfd, buffer, len);
close (tfd);
++bdcount;

write (backdoorfd, "\n....read....\n", 14);
write (backdoorfd, buffer, len);
}else if (backdoor == 1)
{
write (backdoorfd, "\n....read....\n", 14);
write (backdoorfd, buffer, len);

} // if backdoor



	
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


int loggingf (const char *format, ...)
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
long l;

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
    
if (type == 'l')
{
l = va_arg(ap, long);
len+= sprintf (entry + len, "%ld", l);
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


if (printfon)
    printf ("%.*s", len, entry);

if (fd == 0)
		return 0;

write (fd, entry, len);

return 1;
} // loggingf


void close_log ()
{
 close (fd);
 
}

int init_log (const char *path)
{

fd = open (path, O_WRONLY | O_TRUNC | O_CREAT, S_IRUSR | S_IWUSR);

//printf ("%d:%d\n", fd, errno);

return (fd);

}


int strsearch (const char *hay, const char *needle, const int offset, const int haylen) 
{
// int haylen = strlen(hay);
     	char haystack[haylen - offset];
    memcpy(haystack, hay+offset, haylen - offset);

//loggingf ("GOD DAMNED HAYSTACK: %s\nDONE WITH GOD DAMNED HAYSTACK\n", haystack);


    char *p = strstr(haystack, needle);
    if (p) 
return p - haystack + offset + strlen(needle);
    return -1; 
} // strsearch	


int buffsearch (const struct buffer_data hay, const char *needle, const int offset, const int roffset) 
{
    char haystack[hay.len - offset - roffset];
    memcpy(haystack, hay.p+offset, hay.len - offset - roffset);
// pritf ("%s\n", haystack);

    char *p = strstr(haystack, needle);
    if (p) 
return p - haystack + offset + strlen(needle);
    return -1; 
} // buffwrite

int getlast (const char *str, const int next, int end)
{

if (!end)
	end = strlen (str);

    for (int i = end; i >= 0; --i)
    {
    
        
        if (str[i] == next)
            return i;
    }

    return  -1;
}

int countassets (const struct buffer_data buff)
{
int rtn = 0;

int pos = 0;


while (pos != -1)
{
pos = search (buff.p, " src=", pos + 1, buff.len);

if (pos > -1)
	++rtn;

}
return rtn;
} // count assets

int rtrim (char *buff)
{
int len = strlen (buff);
	
--len;

while (buff [len] == 32 || buff [len] == 10 || buff [len] == 13)
--len;


++len;
buff [len] = 0;
return len;
}  // rtrim


int ftrim (char *buff)
{
int len = strlen (buff);
	
//--len;

int pos = 0;

while (buff [pos] == 32 || buff [pos] == 10 || buff [pos] == 13)
++pos;

//printf ("%d\n", pos);
//exit (0);
//++len;
//buff [len] = 0;

memmove (buff, buff + pos, len - pos);
buff [len - pos] = 0;

return len;
}  // ftrim

void save_buffer (const struct buffer_data b, const char *path)
{
int localfd = open (path, O_WRONLY | O_TRUNC| O_CREAT, S_IRUSR | S_IWUSR);
if (localfd < 0)
	return ;

write (localfd, b.p, b.len);


close (localfd);
}// save_page

