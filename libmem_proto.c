// inlcuded already

#include <sys/types.h>
//#include <fcntl.h>

// included already

char filepath[500];



int load_set (char **obj, char *fpath)
{

int fd = open (fpath, O_RDONLY);
   if (fd < 0)
       return 0;

struct stat finfo;
fstat (fd, &finfo);  

*obj = (char *) malloc (finfo.st_size + 1);

if (*obj == NULL)
    return 0;

int readlen = read (fd, *obj, finfo.st_size);

//printf ("%s\n", *obj);

close (fd);


return 1;
} // load_set


int get_set (char *obj, char *cat, char *name, char *value)
{
    
    int cat_delim = instr (obj, cat, 0, 10000);
   int name_delim = instr (obj, name, cat_delim, 10000);
   
   int end_line = getnext(obj, (char) 10, name_delim, 10000);
   
   midstr (obj, value, name_delim, end_line);
    
}


int bool_set (char *obj, char *cat, char *value)
{
char temp[500];    
    int cat_delim = instr (obj, cat, 0, 10000);
   int valuecheck = instr (obj, value, cat_delim, 10000);
   
   
   if (valuecheck  > -1)
       return 1;
   
   return 0;
   
   
  // midstr (obj, temp, name_delim, end_line);

    //return chrcmp (temp, value);
        
    
    
    
    
}

int put_set (char *cat, char *name, char *value)
{
    

    
}
