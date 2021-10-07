

#ifndef logging_h
#define logging_h

int init_log (const char *path);
int loggingf (const int level, const char *format, ...);
void close_log ();

#define remove_old 1

#endif
