#ifndef _GITIO_H_INCLUDED
#define _GITIO_H_INCLUDED

#define _GNU_SOURCE

#include <string.h>
#include <stdarg.h>
#include <stdlib.h>

#ifdef _USE_FCGI_STDIO
#include <fcgi_stdio.h>
#else // _USE_FCGI_STDIO
#include <stdio.h>
#endif // _USE_FCGI_STDIO

void git_sformat(char **out, const char *format, ...);

#endif // _GITIO_H_INCLUDED
