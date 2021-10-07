#include <stdio.h>

#include <fcntl.h>

#include <unistd.h>
#include <stdlib.h>

int main (int argc, char **argv)
{

int fd = open ("sample.txt", O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);

if (fd < 0)
	return 0;
	
int mrun = atoi (argv [1]);

for (int i = 0; i < mrun; ++i)
{
char temp [1000];

int len = sprintf (temp, "%d, this is a test file, %d\n", i, i);

write (fd, temp, len);

}


}
