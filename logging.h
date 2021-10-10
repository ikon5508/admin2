

#ifndef logging_h
#define logging_h

int init_log (const char *path);
int loggingf (const char *format, ...);
void close_log ();

#define remove_old 1

#endif
