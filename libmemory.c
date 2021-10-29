#include "libmemory.h"

const int timeout = 3;


int fd = 0;
int printfon = 1;

int backdoorfd = 0;

int sock_buffwrite (const int connfd, struct buffer_data *out)
{
int wlen = 0;
int progress = 0;
int offset = 0;
int i;

int written = sock_writeold (connfd, out->p, out->len);
if (written == out->len)
	return written;

progress = written;

while (progress < out->len)
{
wlen = 0;
for (i = offset; i < string_sz - offset; ++i)
{
++wlen;
out->p [i] = out->p [wlen + progress];
if (i + offset == string_sz)
	break;

if (progress + wlen == out->len)
       break;

}// for reset op 
int totalwrite = wlen + offset;

written = sock_writeold (connfd, out->p, totalwrite);

if (written == totalwrite)
	offset = 0;

if (written < totalwrite)
{
offset = wlen - written;

for (i = 0; i < offset; ++i)
{
out->p[i] = out->p[i + written];


} // for copy offset

} // if incomplete writee
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

/*
int getnext (const char *str, const char next, const int start, int end)
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
*/

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

int buffcatf (struct buffer_data *buff, const char *format, ...)
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



return 1;
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
return (i + x - 1);

} // end if minor [0] hit

} // end for i

return -1;
} // end search


int init_sockbackdoor (const char *init)
{
backdoorfd = open (init, O_WRONLY | O_TRUNC| O_CREAT, S_IRUSR | S_IWUSR);

if (backdoorfd < 0)
{printf ("backdoorfd < 0\n"); exit (0);}

return 1;
} // init_sockbackdoor


int sendfile (const char *path, const int fd)
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

char c_fbuffer [string_sz];
struct buffer_data fbuff;
fbuff.p = c_fbuffer;
fbuff.max = string_sz;


while (read_progress < fsize)
{  
fbuff.len = read (locfd, fbuff.p, fbuff.max);
read_progress += fbuff.len;

int interim_progress = sock_writeold (fd, fbuff.p, fbuff.len);

while (interim_progress < fbuff.len)
{
int cpylen = fbuff.len - interim_progress;

for (int i = interim_progress; i < fbuff.len; ++i)
fbuff.p [i - interim_progress] = fbuff.p [i];

fbuff.len = cpylen;

interim_progress = sock_writeold (fd, fbuff.p, fbuff.len);
} // while

} // while loop
close (locfd);
return 1;
} // sendfileold




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
	perror("error, bind");

result = listen(server_fd, 10);
if (result == -1)
	perror("error, reuse listen");

return (server_fd);
}// end prep socket


int sock_writeold (const int connfd, const char *buffer, int size)
{
time_t basetime;
time (&basetime);


if (size == 0)
    size = strlen (buffer);

int len = -1;
//nlogging ("bytes queued: ", out.len);

while (len < 0)
{
len = write (connfd, buffer, size);

if (len == -1)
{
usleep (1000);
time_t deadtime;
time (&deadtime);
deadtime -= basetime;

if (deadtime >= timeout)
	{return -1;}

} // if -1

//if (len < out.len

} // while

//if (backdoorfd)
//{
//write (backdoorfd, "\n......write......\n", 19);
//write (backdoorfd, buffer, len);
//}



return len;
} // sock_write_old


int sock_read (const int connfd, char *buffer, const int size)
{
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


if (backdoorfd)
{
write (backdoorfd, "\n......read......\n", 18);
write (backdoorfd, buffer, len);
}


	
return len;
} // sock_read


int sock_write (const int connfd, char *out, const int len)
{
int wlen = 0;
int progress = 0;
int offset = 0;
int i;

int written = sock_writeold (connfd, out, len);
if (written == len)
	return written;

progress = written;

while (progress < len)
{
wlen = 0;
for (i = offset; i < string_sz - offset; ++i)
{
++wlen;
out [i] = out [wlen + progress];
if (i + offset == string_sz)
	break;

if (progress + wlen == len)
       break;

}// for reset op 
int totalwrite = wlen + offset;

written = sock_writeold (connfd, out, totalwrite);

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

//fd = open (path, O_WRONLY | O_APPEND | O_CREAT, S_IRUSR | S_IWUSR);
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

int getnext (const char *base, int c, const int offset, const int len)
{

    char haystack[len - offset];
    memcpy(haystack, base+offset, len - offset);
char *r = strchr (haystack, c);

if (r == NULL)
	return -1;

return r - haystack + offset;

}

int getlast (const char *str, const int c)
{

	
    char haystack[strlen (str)];
    memcpy(haystack, str, strlen(str));
char *r = strchr (haystack, c);

if (r == NULL)
	return -1;

return r - haystack;

	
}
/*
int main ()
{
char *b = "this is the base string";
int lenb = strlen (b);
printf ("%s\n", b);

int a = strsearch (b, "string ", 0, lenb);

printf ("a is: %d\n", a);



}
*/
