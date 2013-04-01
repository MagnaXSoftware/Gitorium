#ifndef COMMON_H_INCLUDED
#define COMMON_H_INCLUDED

#define _GNU_SOURCE

#include <string.h>
#include <stdlib.h>
#include <git2.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdarg.h>

#ifdef _USE_FCGI_STDIO
    #include <fcgi_stdio.h>
#else // _USE_FCGI_STDIO
    #include <stdio.h>
#endif // _USE_FCGI_STDIO

#include "config.h"
#include "specific.h"
#include "error.h"

#define sizeofa(x) (sizeof(x)/sizeof(x[0]))
#define ARRAY_SIZE sizeofa

#define error(string)          errorf("%s", string)
#define errorf(format, ...)    fprintf(stderr, format"\n", __VA_ARGS__)

#ifdef _USE_FCGI_STDIO
    // we redirect errors to stdout when using fcgi
    #undef errorf
    #define errorf(format, ...)    fprintf(stdout, format"\n", __VA_ARGS__)
#endif // _USE_FCGI_STDIO

#define debug(string)          error("debug: "string)
#define debugf(format, ...)    errorf("debug: "format, __VA_ARGS__)
#define fatal(string)          error("fatal: "string)
#define fatalf(format, ...)    errorf("fatal: "format, __VA_ARGS__)

int gitorium_execvp(const char **argv);
int gitorium_execlp(const char *file, const char *arg, ...);
int rrmdir(const char *dir);
int strprecmp(const char *str, const char *prefix);

#endif // COMMON_H_INCLUDED

