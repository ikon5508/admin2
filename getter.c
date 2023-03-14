#define _GNU_SOURCE
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>

#include <openssl/ssl.h>
#include <openssl/err.h>

//#include <sys/types.h>
       #include <sys/stat.h>
       #include <fcntl.h>

struct io_control {
SSL *ssl;
int connfd;
};

typedef struct io_control io_t;
struct buffer_data
{
char *p;
int len;
int max;
};
typedef struct buffer_data buffer_t;

bool tls = true;

int sock_write1 (const io_t, const char *, const int);
int sock_write (const io_t, const char *, const int);
int sock_read (const io_t, char *, const int);
int sock_read1 (const io_t, char *, const int);
int sock_send_file (const io_t, const char *);

// bm func def
int tls_read1 (const io_t, char *, const int);
int tls_read (const io_t, char *, const int);
int tls_write (const io_t, const char *, const int);
int tls_write1 (const io_t, const char *, const int);
int tls_send_file (const io_t, const char *);

int (*io_read1) (const io_t, char *, const int) = tls_read1;
int (*io_read) (const io_t, char *, const int) = tls_read;
int (*io_write) (const io_t, const char *, const int) = tls_write;
int (*io_write1) (const io_t, const char *, const int) = tls_write1;
int (*send_file) (const io_t, const char *) = tls_send_file;

void error(char *msg)
{
    perror(msg);
    exit(0);
}

SSL_CTX *create_context()
{
    const SSL_METHOD *method;
    SSL_CTX *ctx;

    method = TLS_server_method();

    ctx = SSL_CTX_new(method);
    if (!ctx) {
        perror("Unable to create SSL context");
        ERR_print_errors_fp(stderr);
        exit(EXIT_FAILURE);
    }

    return ctx;
}

void configure_context(SSL_CTX *ctx)
{
    /* Set the key and cert */
    if (SSL_CTX_use_certificate_file(ctx, "server.crt", SSL_FILETYPE_PEM) <= 0) {
        ERR_print_errors_fp(stderr);
        exit(EXIT_FAILURE);
    }

    if (SSL_CTX_use_PrivateKey_file(ctx, "server.key", SSL_FILETYPE_PEM) <= 0 ) {
        ERR_print_errors_fp(stderr);
        exit(EXIT_FAILURE);
    }
}

int tls_accept (io_t *io)
{ // bm tls accept

while (1)
{
struct pollfd pfd;
pfd.fd = io->connfd;
pfd.events = POLLIN | POLLOUT;
int rtn = poll (&pfd, 1, timeout);
if (rtn < 1) {printf ("tls accept failure 754\n"); return -1;} 
int rt = SSL_accept(io->ssl);

if (rt > 0)
    return 1;

else if (rt <= 0)
{
int rt2 = SSL_get_error(io->ssl, rt);

if (rt2 == SSL_ERROR_WANT_WRITE || rt2 == SSL_ERROR_WANT_READ) {
//printf ("want read / write\n");
continue;

} else if (rt2 == SSL_ERROR_WANT_CONNECT || rt2 == SSL_ERROR_WANT_ACCEPT){
//printf ("want connect / accept\n");
continue;
    
} else {
printf ("non recoverable error\n");
   return -1;
} //if rt2

} // if rt-1

} // while

return -1;
}

int main(int argc, char *argv[])
{
const int buff_sz = 1000;
char cbuffer[buff_sz];
buffer_t buffer;
buffer.p = cbuffer;
buffer.max = buff_sz;

SSL_CTX *ctx;

if (argc == 2)
{
printf("connecting localhost, request: %s\n", argv[1]);
}else{
printf("undefined\n");
return 0;
}

if (tls == true)
{ctx = create_context(); configure_context(ctx);}


    int sockfd, portno, n;

    struct sockaddr_in serv_addr;
    struct hostent *server;

    portno = 9999;
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) 
        error("ERROR opening socket");
    server = gethostbyname("localhost"); // get hostbyname
    if (server == NULL) {
        fprintf(stderr,"ERROR, no such host\n");
        exit(0);
    }

 bzero((char *) &serv_addr, sizeof(serv_addr));
 serv_addr.sin_family = AF_INET;

 bcopy((char *)server->h_addr, (char *)&serv_addr.sin_addr.s_addr, server->h_length);
    
 serv_addr.sin_port = htons(portno);


    if (connect(sockfd,(struct sockaddr *)&serv_addr,sizeof(serv_addr)) < 0) 
        error("ERROR connecting");


buffer.len = sprintf(buffer.p, "GET %s HTTP/1.1 OK\n", argv[1]);

    n = write(sockfd, buffer.p, buffer.len);
    if (n < 0) 
         error("ERROR writing to socket");

buffer.len = read(sockfd,buffer.p, buffer.max);
if (buffer.len < 0) 
  error("ERROR reading from socket");

buffer.p[buffer.len] = 0;

//printf("%s\n", buffer.p);
//get content len

char *p1 = strcasestr (buffer.p, "Content-Length: ");
if (p1 != 0)
{
//printf ("it is found:!\n");
char strnum [100];
memset (strnum, 0, 100);

int d1 = p1 - buffer.p + 16;

char *p2 = memchr (buffer.p + d1, 10, buffer.len - d1);
 int d2 = p2 - buffer.p;
int strnumlen = d2 - d1;
memcpy (strnum, buffer.p + d1, strnumlen);

//printf ("d1: %d, d2: %d\n", d1, d2);

printf ("the length is: [%s]\n", strnum);
int content_len = atoi (strnum);
int read_prog = 0;


//int fd = open ("output", O_CREAT | O_WRONLY);
int fd = open ("output", O_WRONLY | O_TRUNC| O_CREAT, S_IRUSR | S_IWUSR);
while (read_prog < content_len)
{
int rtn = read (sockfd, buffer.p, buffer.max);
if (rtn <= 0) {printf ("0 rtn\n"); break;}

write (fd, buffer.p, rtn);
read_prog += rtn;
printf ("%d / %d\n", read_prog, content_len);
} // read loop
close (fd);

} // if content len




//    return 0;


} // main

int tls_read1 (const io_t io, char *buffer, const int max)
{ // bm tls_read1
  struct pollfd ev;
ev.fd = io.connfd;
ev.events = POLLIN;

int r = poll (&ev, 1, timeout * 1000);
if (r == 0) return -1;

int len = SSL_read (io.ssl, buffer, max);
	
return len;  
}

int sock_read1 (const io_t io, char *buffer, const int max)
{ // bm sock_read1
  struct pollfd ev;
ev.fd = io.connfd;
ev.events = POLLIN;

int r = poll (&ev, 1, timeout * 1000);
if (r == 0) return -1;

int len = read (io.connfd, buffer, max);
	
return len;  
}

int tls_read (const io_t io, char *buffer, const int max)
{ // bm tls_read
  struct pollfd ev;
ev.fd = io.connfd;
ev.events = POLLIN;

int r = poll (&ev, 1, timeout * 1000);
if (r == 0) return -1;

int len = SSL_read (io.ssl, buffer, max);
	
return len;  
}

int sock_read (const io_t io, char *buffer, const int max)
{ // bm sock_read
  struct pollfd ev;
ev.fd = io.connfd;
ev.events = POLLIN;

int r = poll (&ev, 1, timeout * 1000);
if (r == 0) return -1;

int len = read (io.connfd, buffer, max);
	
return len;  
}

int tls_write (const io_t io, const char *buffer, const int size)
{// tls_write
printf ("tls writing\n");
struct pollfd ev;
ev.fd = io.connfd;
ev.events = POLLOUT;

int prtn = poll (&ev, 1, timeout * 1000);
if (prtn < 1) return -1;

int rtn = SSL_write (io.ssl, buffer, size);
int accrue = rtn;
while (accrue < size)
{

prtn = poll (&ev, 1, timeout * 1000);
if (prtn < 1) return -1;
   
rtn = SSL_write (io.ssl, buffer + accrue, size - accrue);
if (rtn <= 0) return 1;
accrue += rtn;
}
return accrue;

} // tls write


int sock_write (const io_t io, const char *buffer, const int size)
{// bm sock write
//printf ("sock writing\n");
 struct pollfd ev;
ev.fd = io.connfd;
ev.events = POLLOUT;

int prtn = poll (&ev, 1, timeout * 1000);
if (prtn < 1) return -1;

int rtn = write (io.connfd, buffer, size);
int accrue = rtn;
while (accrue < size)
{

prtn = poll (&ev, 1, timeout * 1000);
if (prtn < 1) return -1;
   
rtn = write (io.connfd, buffer + accrue, size - accrue);
if (rtn <= 0) return 1;
accrue += rtn;
}
return accrue;
} // sock write


int sock_write1 (const io_t io, const char *buffer, const int size)
{// bm sock write1
 struct pollfd ev;
ev.fd = io.connfd;
ev.events = POLLOUT;

int prtn = poll (&ev, 1, timeout * 1000);
if (prtn < 1) return -1;

return write (io.connfd, buffer, size);
return -1;
}

int tls_write1 (const io_t io, const char *buffer, const int size)
{ // bm tls_write1
struct pollfd ev;
ev.fd = io.connfd;
ev.events = POLLOUT;

int prtn = poll (&ev, 1, timeout * 1000);
if (prtn < 1) return -1;

return SSL_write (io.ssl, buffer, size);

}
