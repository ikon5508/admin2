#include "libmemory.h"
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>

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
 */
struct search_data searchM  (const char *main, const char *minor, int offset, int start, int end)
{
struct search_data rtn = {0, 0};

int minorlen = strlen (minor);
int osearch = offset;

if (end == 0)
    end = strlen (main);

while (start < end)
{

if (minor[offset] == 0)
{

rtn.rtn = start - 1;
rtn.offset = 0;
return rtn;
} // if
	
	
if (main[start] == minor[offset])
{
//printf("main: %c, minor: %c\n", main[lc], minor[sc]);
++offset;
++start;
continue;
}else{
offset = 0;      
++start;

if (osearch)
	{rtn.rtn = -1; return rtn;}

}	// end if

} // end while


//if (start == end)
//{
if (offset)
{

if (minorlen > offset)
{
//printf ("000major with midsearch to second step000\n");
//start = -2;
rtn.offset = offset;
rtn.rtn = -2; 
return rtn;
}else{
} 
rtn.rtn = start -1;
rtn.offset = 0;
return rtn;
} 



if (!offset)
{rtn.rtn = -1; return rtn;}
	//printf ("000major end NO OFFSET000\n");
//}


}//searchM


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


/*
int main ()
{

	
	char *b = "this bs is a big test";

	printf ("%s, %d\n", b, strlen(b));

struct search_data rtnm = searchM (b, "this", 0, 0, 0);

int r = search (b, "this", 0, 0);

printf ("%d, %d\n", rtnm.rtn, r);

rtnm = searchM (b, "bs", 0, 0, 0);

r = search (b, "bs", 0, 0);

printf ("%d, %d\n", rtnm.rtn, r);

rtnm = searchM (b, "big", 0, 0, 0);

r = search (b, "big", 0, 0);

printf ("%d, %d\n", rtnm.rtn, r);

rtnm = searchM (b, "test", 0, 0, 0);

r = search (b, "test", 0, 0);

printf ("%d, %d\n", rtnm.rtn, r);

}
*/
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

struct search_data rtn = {0,0};

printf ("major1: %s\nmajor2: %s\n", b1, b2);


printf ("\n\nsearch 1, webformkitty\n");

//offset = 0;
rtn = searchM (b1, minor1, rtn.offset, 0, 0);

	printf ("result: %d, %d\n", rtn.rtn, rtn.offset);

rtn = searchM (b2, minor1, rtn.offset, 0, 0);

	printf ("result: %d, %d\n", rtn.rtn, rtn.offset);


printf ("\n\nsearch 2 webformkit\n");
rtn.offset = 0;
rtn = searchM (b1, minor2, rtn.offset, 0, 0);

	printf ("result: %d, %d\n", rtn.rtn, rtn.offset);

rtn = searchM (b2, minor2, rtn.offset, 0, 0);

	printf ("result: %d, %d\n", rtn.rtn, rtn.offset);

	

printf ("\n\nsearch 3 webform\n");
rtn.offset = 0;
rtn = searchM (b1, minor3, rtn.offset, 0, 0);

	printf ("result: %d, %d\n", rtn.rtn, rtn.offset);

rtn = searchM (b2, minor3, 0, 0, 0);

	printf ("result: %d, %d\n", rtn.rtn, rtn.offset);


printf ("\n\nsearch 4 web\n");
rtn.offset = 0;
rtn = searchM (b1, minor4, rtn.offset, 0, 0);

	printf ("result: %d, %d\n", rtn.rtn, rtn.offset);

rtn = searchM (b2, minor4, 0, 0, 0);

	printf ("result: %d, %d\n", rtn.rtn, rtn.offset);


rtn = searchM ("the big test", "abc", 0, 0, 0);

	printf ("abc big test result: %d, %d\n", rtn.rtn, rtn.offset);




}
*/
