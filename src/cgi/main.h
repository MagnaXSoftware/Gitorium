#ifndef MAIN_H_INCLUDED
#define MAIN_H_INCLUDED

#define _GNU_SOURCE

#include <fcgi_stdio.h>

#define _USE_FCGI_STDIO

#include "../common.h"

#include <regex.h>
#include <sys/wait.h>
#include <unistd.h>
#include <time.h>

#include "../gitio.h"

// errors must go to stdout to be displayed
#undef errorf
#define errorf(format, ...)    fprintf(stdout, format"\n", __VA_ARGS__);

#endif // MAIN_H_INCLUDED

