#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include <arpa/inet.h>
#include <netinet/tcp.h>

#include <fcntl.h>
//#include <signal.h>
//#include <sys/epoll.h>
//#include <sys/ioctl.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/stat.h>


#include "socket.h"

#include "libmemory.h"
#include "logging.h"

const int timeout = 3;


int backdoorfd = 0;

int init_sockbackdoor (const char *init)
{
backdoorfd = open (init, O_WRONLY | O_TRUNC| O_CREAT, S_IRUSR | S_IWUSR);

if (backdoorfd < 0)
{printf ("backdoorfd < 0\n"); exit (0);}


} // init_sockbackdoor

int sock_write (const int connfd, const char *buffer, const int size)
{
time_t basetime;
time (&basetime);

int writelen = -1;
int write_progress = 0;
  
while (writelen < 0)
{
writelen = write (connfd, buffer, size);

if (writelen == -1)
{
usleep (1000);
time_t deadtime;
time (&deadtime);
deadtime -= basetime;

if (deadtime >= timeout)
	{return -1;}
} // if -1
} // while

write_progress += writelen;
/*
if (backdoorfd)
{
write (backdoorfd, "\n......write......\n", 19);
write (backdoorfd, buffer, writelen);
} //if
*/
char smbuffer [string_sz];
struct buffer_data smbuff;
smbuff.p = smbuffer;
smbuff.max = string_sz;
smbuff.len = 0;


while (write_progress < size)
{
int cpylen = size - write_progress;
int minorpos;

for (int i = write_progress; i < size; ++i)
{
minorpos = i - write_progress;
smbuff.p [minorpos] = buffer [i];
if (minorpos == smbuff.max)
    break;
} //for
smbuff.len = minorpos;

writelen = -1;
time (&basetime);
while (writelen < 0)
{
writelen = write (connfd, smbuff.p, smbuff.len);

if (writelen == -1)
{
usleep (1000);
time_t deadtime;
time (&deadtime);
deadtime -= basetime;

if (deadtime >= timeout)
	{return -1;}
} // if -1
} // while write -1

write_progress += writelen;
/*
if (backdoorfd)
{
write (backdoorfd, "\n......write......\n", 19);
write (backdoorfd, smbuff.p, writelen);
} //if
*/
} // while write_progress < size

 
return write_progress;
} // sock_write

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
loggingf (200, "file size: %d\n", fsize);

char c_fbuffer [string_sz];
struct buffer_data fbuff;
fbuff.p = c_fbuffer;
fbuff.max = string_sz;


while (read_progress < fsize)
{  
fbuff.len = read (locfd, fbuff.p, fbuff.max);
read_progress += fbuff.len;

int interim_progress = sock_write (fd, fbuff.p, fbuff.len);

loggingf (200, "read progress: %d - written: %d\n", read_progress, interim_progress);


} // while loop

} // sendfile


/*
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
loggingf (200, "file size: %d\n", fsize);

char c_fbuffer [string_sz];
struct buffer_data fbuff;
fbuff.p = c_fbuffer;
fbuff.max = string_sz;


while (read_progress < fsize)
{  
fbuff.len = read (locfd, fbuff.p, fbuff.max);
read_progress += fbuff.len;

int interim_progress = sock_write (fd, fbuff.p, fbuff.len);

loggingf (200, "read progress: %d - written: %d\n", read_progress, interim_progress);

while (interim_progress < fbuff.len)
{
//loggingf (200, "additional logic required <int sendfile>\n"); 


int cpylen = fbuff.len - interim_progress;
loggingf (200, "incomplete writer leftover: %d\n", cpylen);

for (int i = interim_progress; i < fbuff.len; ++i)
fbuff.p [i - interim_progress] = fbuff.p [i];

fbuff.len = cpylen;

interim_progress = sock_write (fd, fbuff.p, fbuff.len);


loggingf (200, "(incomplete) progress: %d - written: %d\n", cpylen, interim_progress);
} // while


} // while loop

} // sendfile
*/

int getfile (const char *path, const int fd)
{

}// getfile



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

if (backdoorfd)
{
write (backdoorfd, "\n......write......\n", 19);
write (backdoorfd, buffer, len);
}



return len;
} // sock_write


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

