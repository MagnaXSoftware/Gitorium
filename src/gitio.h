#ifndef _GITIO_H_INCLUDED
#define _GITIO_H_INCLUDED

#include "common.h"

void            gitio_fflush    (FILE *stream);

unsigned int    gitio_sformat   (char **out, const char *format, ...);
void            gitio_fwrite    (FILE *stream, const char *format, ...);
int             gitio_read      (int fd, char *buffer, ssize_t size);
void            gitio_truncate  (char *buffer, ssize_t size);

#endif // _GITIO_H_INCLUDED
