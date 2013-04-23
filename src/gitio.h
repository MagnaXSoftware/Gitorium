#ifndef _GITIO_H_INCLUDED
#define _GITIO_H_INCLUDED

#include "common.h"

// Someone really should sit down and fix FCGI
#ifndef _USE_FCGI_STDIO
    #define _ATTRIBUTE_FORMAT(m,n) __attribute__((format (printf,m,n)))
#else
    #define _ATTRIBUTE_FORMAT(m,n) /* nothing */
#endif // _USE_FCGI_STDIO

unsigned int  gitio_sformat   (char **out, const char *format, ...) _ATTRIBUTE_FORMAT(2,3);
void          gitio_fwrite    (FILE *stream, const char *format, ...) _ATTRIBUTE_FORMAT(2,3);
void          gitio_fflush    (FILE *stream);
void          gitio_write     (const char *format, ...) _ATTRIBUTE_FORMAT(1,2);

int           gitio_fread     (FILE *stream, char *buffer, size_t size);
char         *gitio_fread_line(FILE *stream);
void          gitio_truncate  (char *buffer, size_t size);

#endif // _GITIO_H_INCLUDED
