#include <time.h>
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

int prepsocket (const int PORT);
int sock_setnonblock (const int fd);

int sock_writeold (const int connfd, const char *buffer, const int size);
int sock_read (const int connfd, char *buffer, int size);

int sendfile (const char *path, const int fd);
int init_sockbackdoor (const char *init);

extern const int timeout;
