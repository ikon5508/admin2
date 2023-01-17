#include "shared.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>


// must search for multiple instances of delim
// no worry about going in step, will largely be futile.
void build_template (buffer_t *buff, const int ents, ...)
{
va_list ap;
va_start (ap, ents);

int d;
char c;
char *s;

struct ent_table {
char *delim;
int pos;
char *rep;
int replen;

struct ent_table *next;
}entries[ents];

int req_len = buff->len;

printf ("%s\n", buff->p);

for (int i = 0; i < ents; ++i)
{
entries[i].delim = va_arg(ap, char *);
entries[i].rep = va_arg(ap, char *);
entries[i].replen = strlen (entries[i].rep);
int dlen = strlen (entries[i].delim);

char *ret = (char *) memmem (buff->p, buff->len, entries[i].delim, dlen);
if (ret != NULL) {
entries[i].pos = ret - buff->p;
req_len -= dlen;
req_len += entries[i].replen;
}else {entries[i].pos = -1;}


} // for


for (int i = 0; i < ents; ++i)
{
printf ("%d: delim: %s, pos: %d, rep: %s, replen: %d\n", i, entries[i].delim, entries[i].pos, entries[i].rep, entries[i].replen);
}


} // build_template


buffer_t JSON_decode (const buffer_t in)
{
int tail = 0;
int req_len = in.len - 2;

const int increment = 200;
int inst_max = increment;
int inst_count = 0;

struct codeco *inst = (struct codeco *)  malloc (inst_max * sizeof (struct codeco));
if (inst == NULL) killme ("no malloc");

//for (int i = 0; i < 100; ++i)
while (1)
{
char *p1 = (char *) memchr (in.p + tail,  92, in.len - tail);
if (p1 == NULL) break;

if (inst_count == inst_max)
{
inst_max += increment;
printf ("resizing instruction array\n");
inst = realloc (inst, sizeof (struct codeco) * inst_max);
if (inst == NULL) killme ("no realloc");
} //if

int d1 = p1 - in.p + 1;
char dchar = in.p[d1];


//printf ("%d pos:%d dchar: %c\n", inst_count ,d1 , dchar);

switch (dchar) {
case 'n':
inst[inst_count].dchar = dchar;
inst[inst_count].pos = d1;
req_len -= 1;

break;
case '\"':
inst[inst_count].dchar = dchar;
inst[inst_count].pos = d1;
req_len -= 1;

break;
case 92:
inst[inst_count].dchar = dchar;
inst[inst_count].pos = d1;
req_len -= 1;

break;
case 't':
inst[inst_count].dchar = dchar;
inst[inst_count].pos = d1;
req_len -= 1;

break;
case '\'':
inst[inst_count].dchar = dchar;
inst[inst_count].pos = d1;
req_len -= 1;

break;
default:
printf ("unhandled escape delimeter char: [%d]%c\n", d1, dchar);
char temp [smbuff_sz];
memset (temp, 0, default_sz);
memcpy (temp, in.p + d1 - 80,160);
printf ("samp [%s]\n", temp);
exit (0);
} // end switch

tail = d1 + 1;
++inst_count;
} //loop

//printf ("in.len: %d| req_len: %d\n", in.len, req_len);

buffer_t out = init_buffer (req_len);
memset (out.p, 0, req_len);
int i = 0;
tail = 0;
for (i = 0; i < inst_count; ++i)
{

int len = inst[i].pos - tail - 2;
memcpy (out.p + out.len, in.p + tail + 1, len);
out.len += len;
tail = inst[i].pos;

switch (inst[i].dchar) {

case 'n':
	out.p[out.len] = 10;

break;
case 92:
	out.p[out.len] = 92;

break;
case  '\"':
	out.p[out.len] = '\"';

break;
case '\'':
	out.p[out.len] = '\'';

break;
case  't':
	out.p[out.len] = 9;

break;
default: 
killme ("unhandled instruction");

}// switch

++out.len;
} // loop

///printf ("req_len: %d, ou.len: %d\n", req_len, out.len);

int len = req_len - out.len;
if (len > 0) {
memcpy (out.p + out.len, in.p + tail +1, len);
out.len += len;
printf ("added!!\n");
}else{

switch (inst[inst_count -1].dchar) {

case 'n':
	out.p[out.len] = 10;

break;
case 92:
	out.p[out.len] = 92;

break;
case  '\"':
	out.p[out.len] = '\"';

break;
case '\'':
	out.p[out.len] = '\'';

break;
case  't':
	out.p[out.len] = 9;
}
printf ("len is 0last dchar %d[%c]\n", inst[inst_count -1].dchar, inst[inst_count -1].dchar);

++out.len;
}




//printf ("len: %d ...last char: %d[%c]\n", out.len, out.p[out.len], out.p[out.len]);

/*
char temp [default_sz];
memset (temp, 0, default_sz);
memcpy (temp, in.p + tail +1, len);
printf ("adding [%s]\n", temp);
exit (0);
*/

free (inst);
return out;
} // JSON decode


buffer_t HTML_encode (const buffer_t in, const int level)
{
//const buffer_t in = *((buffer_t *) buff);

int d1 = 0;
int d2 = 0;
char lastc = 0;

char searchstr[10];
if (level == 1)
{
lastc = 0;
strcpy (searchstr, "\'\"&<>");

}else if  (level == 2) {
lastc = '\n';
strcpy (searchstr, "\"\'&<>\n");
} // if


//printf ("%s\n\n", in.p);
const int increment = 500;
int inst_max = increment;
int inst_count = 0;

struct codeco *inst = (struct codeco *)  malloc (inst_max * sizeof (struct codeco));
if (inst == NULL) killme ("no malloc");
int req_len = in.len;
printf ("in.len: %d\n", in.len);
while (1)
//for (int q = 0; q < 10; ++q)
{

char *p1 = strpbrk (in.p + d1, searchstr);
//char *p1 = memchr (in.p + d1, (int) '<', in.len);
if (p1 == NULL) break ;

if (inst_count == inst_max)
{
inst_max += increment;
printf ("resizing instruction array\n");
inst = realloc (inst, sizeof (struct codeco) * inst_max);
if (inst == NULL) killme ("no realloc");
} //if

d2 = p1 - in.p;
char dchar = in.p[d2];
switch (dchar) {
// 34 = " 39 = '
case 34:
inst [inst_count].pos = d2;
inst [inst_count].dchar = in.p[d2];
++inst_count;
req_len += 5;

break;
case 39:
inst [inst_count].pos = d2;
inst [inst_count].dchar = in.p[d2];
++inst_count;
req_len += 5;

break;
case '<':
inst [inst_count].pos = d2;
inst [inst_count].dchar = in.p[d2];
++inst_count;
req_len += 4;
break;

case '>':
inst [inst_count].pos = d2;
inst [inst_count].dchar = in.p[d2];
++inst_count;
req_len += 4;

break;
case '&':
inst [inst_count].pos = d2;
inst [inst_count].dchar = in.p[d2];
++inst_count;
req_len += 5;

break;
default :

if (lastc == dchar) {
inst [inst_count].pos = d2;
inst [inst_count].dchar = in.p[d2];
++inst_count;
req_len += 4;
}
}// switch

d1 = d2 + 1;
} // first loop

//printf ("inst_cout: %d, in_len: %d, req_len: %d\n", inst_count, in.len, req_len);
buffer_t out;
out.p = malloc (req_len);
if (out.p == NULL) killme ("malloc");
out.len = 0;
out.max = req_len;
/*
if (out->max == 0)
{
//printf ("buffer not allocated\n");
out->p = malloc (req_len);
if (out->p == NULL) killme ("no malloc");
}else if (out->max < req_len) {

//printf ("buffer too small\n");
out->p = realloc (out->p, req_len);
if (out->p == NULL) killme ("no realloc");
}
*/
int pos = -1;
for (int i = 0; i < inst_count; ++i)
{
//printf ("#d %d: pos %d: dchar: %c\n", i, inst[i].pos, inst[i].dchar);


int len = inst[i].pos - pos - 1;
//memcpy (temp, in.p + pos + 1, len);
memcpy (out.p + out.len, in.p + pos + 1, len);
out.len += len;

char dchar = inst[i].dchar;
switch (dchar) {
case 34: // = "
//&#34;
out.p[out.len] = '&';
out.p[out.len + 1] = '#';
out.p[out.len + 2] = '3';
out.p[out.len + 3] = '4';
out.p[out.len + 4] = ';';
out.len += 5;

break;
case 39: // = '
//&#39;
out.p[out.len] = '&';
out.p[out.len + 1] = '#';
out.p[out.len + 2] = '3';
out.p[out.len + 3] = '9';
out.p[out.len + 4] = ';';
out.len += 5;

break;
case '&':
out.p[out.len] = '&';
out.p[out.len + 1] = 'a';
out.p[out.len + 2] = 'm';
out.p[out.len + 3] = 'p';
out.p[out.len + 4] = ';';
out.len += 5;

break;
case '<':
out.p[out.len] = '&';
out.p[out.len + 1] = 'l';
out.p[out.len + 2] = 't';
out.p[out.len + 3] = ';';
out.len += 4;
break;

case '>':
out.p[out.len] = '&';
out.p[out.len + 1] = 'g';
out.p[out.len + 2] = 't';
out.p[out.len + 3] = ';';
out.len += 4;
break;
default:


if (lastc == dchar) {
out.p[out.len] = '<';
out.p[out.len + 1] = 'b';
out.p[out.len + 2] = 'r';
out.p[out.len + 3] = '>';
out.len += 4;

}

} // switch
pos = inst[i].pos;

} //for


int len = in.len - pos - 1;
if (len > 0) {
memcpy (out.p + out.len, in.p + pos + 1, len);
out.len += len;
}
//printf ("out-> [%.*s]\n", out->len, out->p);
free (inst);
//free (in.p);

//const buffer_t rtn = *((buffer_t *) out);
return out;

} // end HTML encode



int URL_decode (const char *in, char *out)
{ // bm URL_decode
int outlen = 0;
int tail = 0;
while (1)
{
char *p1 = strchr (in + tail, (int) '%');
if (p1 == NULL) break;
int offset = p1 - in;

//copy everything up to offset
int len = offset - tail;
memcpy (out + outlen, in + tail, len);
outlen += len;
//printf ("progress: %.*s\n", outlen, out); 

char code [4]; 
memcpy (code, in + offset + 1, 2);
code [2] = 0;
 
if (!strcmp (code, "20"))
	out [outlen] = ' ';
	++outlen;

tail = offset + 3;
} //loop

int in_len = strlen (in);
int remainder = in_len - tail;
memcpy (out + outlen, in + tail, remainder);
outlen += remainder;

out [outlen] = 0;



return outlen;
} // URL decode

void buffer_sanity (buffer_t *buff, const int req, const int inc)
{
const int diff = buff->max - buff->len;
//printf ("bufflen: %d, buffmax: %d\n", buff->len, buff->max);

if (diff < req) {
//printf ("resizing buffer\n");
buff->p = (char *) realloc (buff->p, buff->max + inc);
	if (buff->p == NULL) killme ("no realloc");
buff->max += inc;

} // if resize needed

} // buffer_sanity

int FAR (buffer_t *base, const char *delim, const buffer_t rep)
{
char *p1 = (char *) memmem (base->p, base->len, delim, strlen (delim));
if (p1 == NULL) return 0;

int d1 = p1 - base->p;
int d2 = d1 + strlen (delim);

int templen = base->len - d2;
int bparse = base->len - templen;

char *temp = (char *) malloc (templen);
if (temp == NULL) killme ("no malloc");
memcpy (temp, base->p + bparse, templen);

/*printf ("base: %d:  [%.*s]\n", base->len, base->len,  base->p);
printf ("rep %s\n", rep.p);
printf ("temp %d [%.*s]\n", templen, templen, temp);
*/
// do overflow protection
int req_len = d1 + templen + rep.len;
if (req_len > base->max) {
//printf ("req len: %d, base->max: %d-resizing buffer\n", req_len, base->max);
int diff = req_len - base->max;
int needed = base->max + req_len;

base->p = (char *) realloc (base->p, needed);
if (base->p == NULL) killme ("no realloc");
base->max = needed;
} // if base resize needed

memcpy (base->p + d1, rep.p, rep.len);
base->len = rep.len + d1;


//printf ("base: %d:  [%.*s]\n", base->len, base->len,  base->p);
memcpy (base->p + base->len, temp, templen);
base->len += templen;
free (temp);
//printf ("base: %d:  [%.*s]\n", base->len, base->len,  base->p);
return 1;
} // FAR

// no overflow protection
void buffcatf (struct buffer_data *buff, const char *format, ...)
{
va_list ap;
va_start (ap, format);

int formatlen = strlen (format);
//char type;
//int specify_len = 0;
//char entry [lgbuff_sz] = "";

int len = 0;
int d;
char c;
char *s;

char *delim;
int offset = 0;
int pos = 0;

while (1)
{
delim = strchr (format + offset, '%');
if (delim == NULL) break;

offset = delim - format + 1;
char dchar = format [offset];
int len = offset - pos - 1;
// do realloc check here
//add formatted string here before dchar
buffer_sanity (buff, len, len);
memcpy (buff->p + buff->len, format + pos, len);
buff->len += len;

switch (dchar)
//if (dchar == 's')
{
case 's':
s = va_arg(ap, char *);
int slen = strlen (s);
buffer_sanity (buff, slen, slen);
buff->len += sprintf (buff->p + buff->len, "%s", s);
break;

case 'd':
d = va_arg(ap, int);
char temp [100];
int tlen = sprintf (temp, "%d", d);
buffer_sanity (buff, tlen, tlen);

buff->len += sprintf (buff->p + buff->len, "%d", d);

//strcat (buff->p, temp);
//buff->len += tlen;

//buff->len +=break;
} // switch
pos = offset + 1;

} // while

len = formatlen - pos;
buffer_sanity (buff, len, len);
memcpy (buff->p + buff->len, format + pos, len);
buff->len += len;
buff->p[buff->len] = 0;


/*

if (type == 'c')
{
c = (char) va_arg(ap, int);
entry [len] = c;
++len;
entry [len] = 0;
++fplace;    
} // if c
*/

} // buffcatf


int strsearch (const char *main, const char *minor, const int start)
{
char *p = strstr (main + start, minor);
if (p == NULL) return -1;

return (p - main + strlen (minor));

} // end search

int midstr(const char *major, char *minor, int start, const int end)
{
int len = end - start;

memcpy (minor, major + start, len);
minor [len] = 0;
return len;

} // end midstr

int getlast (const char *str, const int next, int end)
{
char *p = (char *) memrchr (str, next, end);
if (p == NULL) return -1;

return (p - str);
}

int getnext (const char *str, const int next, const int start, int end)
{

char *p = (char *) memchr (str + start, next, end - start);
if (p == NULL) return -1;

return (p - str);
} // getnext

// unfinished and untested

int extract_SC (const buffer_t *src, char *ex, const int exmax, const char *d1, const char d2)
{ // bm buffer_data extract string-char

int d1len = strlen (d1);

char *p1 = (char *) memmem (src->p, src->len, d1, d1len);
if (p1 == NULL) return 0;

int o1 = p1 - src->p + d1len;

char *p2 = (char *) memchr (src->p + o1 + 1, (int) d2, src->len);
if (p2 == NULL) return 0;
int o2 = p2 - src->p;

int exlen = o2 - o1;
if (exlen > exmax) return 0;

memcpy (ex, src->p + o1, exlen);

ex[exlen] = 0;

return (exlen);
} // bm extract char char


int extract_CC (const buffer_t src, char *ex, const int exmax, const char d1, const char d2)
{ // bm buffer_data extract char-char
char *p1 = (char *) memchr (src.p, (int) d1, src.len);
if (p1 == NULL) return 0;

int o1 = p1 - src.p;


char *p2 = (char *) memchr (src.p + o1 + 1, (int) d2, src.len);
if (p2 == NULL) return 0;
int o2 = p2 - src.p;

int exlen = o2 - o1 - 1;
if (exlen > exmax) return 0;

memcpy (ex, src.p + o1 + 1, exlen);

ex[exlen] = 0;

return (exlen);
} // bm extract char char

struct buffer_data init_buffer (const int sz)
{ // bm init_buffer
struct buffer_data buffer; 
if (sz == 0)
{memset (&buffer, 0, sizeof (struct buffer_data)); return buffer;}

buffer.p = (char *) malloc (sz + 1);
if (buffer.p == NULL) killme ("error malloc");

buffer.max = sz;
buffer.len = 0;
buffer.procint = 0;

return buffer;
   
} // init buffer



/*
timer_c::timer_c ()
{ // bm timer constructor
clock_gettime (CLOCK_MONOTONIC, &start);
}// timer constructor

void timer_c::reset ()
{ // bm timer reset
clock_gettime (CLOCK_MONOTONIC, &start);
} // timer

void timer_c::stop () 
{ //bm timer end
clock_gettime (CLOCK_MONOTONIC, &end);
} //timer end

void timer_c::result (int *sec, int *ms)
{ // bm timer get diff
struct timespec temp;
//if ((end.tv_nsec-start.tv_nsec)<0)
if (end.tv_nsec < start.tv_nsec)
{ 
temp.tv_sec = end.tv_sec-start.tv_sec-1;
temp.tv_nsec = 1000000000+end.tv_nsec-start.tv_nsec; 
} else { 
temp.tv_sec = end.tv_sec-start.tv_sec;
temp.tv_nsec = end.tv_nsec-start.tv_nsec;
} // if

int millisec = temp.tv_nsec / 1000000;

*sec = temp.tv_sec;
*ms = millisec;

}// timer get diff
*/

void err_ctl (const int rslt, const char *msg) {
if (rslt < 0) {
printf ("%s\n", msg);
exit(0);
}
} //err_ctl


void killme (const char *msg) {
printf ("%s\n", msg);
exit(0);
} //killme


char *parse_line (char *dest, const char *src)
{ // bm parse_line 
char *rtn = (char *) strchr (src, 10);
if (rtn == NULL) return NULL;
int len = rtn - src;
memcpy (dest, src, len);
dest [len] = 0;
return rtn +1;
 
} // parse_line

int split_value (char *src, const char d, char *value)
{ // bm split_value
char *p1 = strchr (src, (int) d);
if (p1 == NULL) return 0;

int len = strlen (src);


int d1 = p1 - src;
//memset (value, 0, valsz);
memcpy (value, src + d1 +1, len - d1 -1);
value [len - d1 -1] = 0;
src[d1] = 0;



return 1;
} // parse value


int trim (char *totrim)
{ // bm void trim
int len = strlen (totrim);

//trim beginning
int count = 0;
for (int i = 0; i !=len; ++i)
{
int trigger = 0;
switch (totrim[i])
{
case 10: 
++count;
break;
case 13:
++count;
break;
case 32:
++count;
break;
  
default:
goto fin1;
  
} // switch
} // for
    
fin1:;
if (count)
{
memmove (totrim, totrim + count, len - count);
totrim [len - count] = 0;  // works partially
//memset (totrim + len - count, 0, count);
len -= count;
///printf ("trimming %d\n", count);
}

int endtrim = 0;
// trim end
for (int i = len -1; i > 0; --i)
{
// printf ("i: %d (%c - <%d>)\n", i, totrim[i], totrim[i]);
switch (totrim[i])
{
case 10: 
totrim[i] = 0;
++endtrim;
break;
case 13:
totrim[i] = 0;
++endtrim;
break;
case 32:
totrim[i] = 0;
++endtrim;
break;

default:
i = 0;
} // switch
} // for

len -= endtrim;
return len;
} // trim

void save_buffer (const struct buffer_data b, const char *path)
{ // bm save_buffer
int localfd = open (path, O_WRONLY | O_TRUNC| O_CREAT, S_IRUSR | S_IWUSR);
if (localfd < 0)
	return ;

write (localfd, b.p, b.len);


close (localfd);
}// save_page
