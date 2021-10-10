//#include "logging.h"
#include "libmemory.h"
#include <fcntl.h>
#include <errno.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <unistd.h>



int fd = 0;
int printfon = 1;


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
