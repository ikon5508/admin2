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


/*
searchMP is search multi-part
starts searching the minor string at offset
if partial minor string is found searchMP will return -2 and offset will be set to # of chars found
 */

int searchM  (const char *main, const char *minor, int *offset,  int start, int end)
{

int multi = 0;
int smallcount = *offset;

if (*offset) multi = 1;


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
{
	
	smallcount = 0;

} // if !==
 
 ++start;    

} // end while

//printf ("smallcount: %d\n", smallcount);



if (multi)
	return -1;

if (smallcount)
{*offset = smallcount; return -2;}



return -1;
}


/*
int main ()
{
	int offset = 0;
char *b1 = "12345we6789webform";
char *b2 = "kit123456789";

char *minor1 = "webformkitty";

char *minor2 = "webformkit";


char *minor3 = "webform";

char *minor4 = "web";
int rtn;


printf ("major1: %s\nmajor2: %s\n", b1, b2);


printf ("\n\nsearch 1, webformkitty\n");

offset = 0;
rtn = searchM (b1, minor1, &offset, 0, 0);

	printf ("result: %d, %d\n", rtn, offset);

rtn = searchM (b2, minor1, &offset, 0, 0);

	printf ("result: %d, %d\n", rtn, offset);

printf ("\n\nsearch 2 webformkit\n");
offset = 0;
rtn = searchM (b1, minor2, &offset, 0, 0);

	printf ("result: %d, %d\n", rtn, offset);

rtn = searchM (b2, minor2, &offset, 0, 0);

	printf ("result: %d, %d\n", rtn, offset);


printf ("\n\nsearch 3 webform\n");
offset = 0;
rtn = searchM (b1, minor3, &offset, 0, 0);

	printf ("result: %d, %d\n", rtn, offset);

rtn = searchM (b2, minor3, &offset, 0, 0);

	printf ("result: %d, %d\n", rtn, offset);


printf ("\n\nsearch 4 web\n");
offset = 0;
rtn = searchM (b1, minor4, &offset, 0, 0);

	printf ("result: %d, %d\n", rtn, offset);

rtn = searchM (b2, minor4, &offset, 0, 0);

	printf ("result: %d, %d\n", rtn, offset);



}
*/
