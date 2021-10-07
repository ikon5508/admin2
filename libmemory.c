#include "libmemory.h"
#include <string.h>
#include <stdlib.h>

int getlast (const char *str, const char next, const int end)
{
const int debug = 0;

    for (int i = end; i >= 0; --i)
    {
    
        if (debug)
            printf ("c: %c - %c\n", str[i], next);
        
        if (str[i] == next)
            return i;
    }

    return  -1;
}

int init_buffer (struct buffer_data *buffer, const int size)
{
buffer->p = (char *) malloc (size);
if (buffer->p == NULL)
    {buffer->max = 0; return 0;}
    
    buffer->max = size;

    return size;
} // init_buffer

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

int search (const char *main, const char *minor, int start, int end)
{
int smallcount = 0;

if (end == 0)
    end = strlen (main);

while (start < end)

{
if (main[start] == minor[smallcount])
{
//printf("main: %c, minor: %c\n", main[lc], minor[sc]);
 
++smallcount;

} // end if

    
if (minor[smallcount] == 0)
return start;


if (!main[start] == minor[smallcount])
    smallcount = 0;
    
++start;    

} // end while
return -1;
} // end search
