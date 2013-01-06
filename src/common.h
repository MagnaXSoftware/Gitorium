#ifndef COMMON_H_INCLUDED
#define COMMON_H_INCLUDED

#define _GNU_SOURCE

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "config.h"
#include "specific.h"

#define ARRAY_SIZE(x) (sizeof(x)/sizeof(x[0]))

#define error(string)          errorf("%s", string)
#define errorf(format, ...)    fprintf(stderr, format"\n", __VA_ARGS__);

#define debug(string)          error("debug: "string)
#define debugf(format, ...)    errorf("debug: "format, __VA_ARGS__)
#define fatal(string)          error("fatal: "string)
#define fatalf(format, ...)    errorf("fatal: "format, __VA_ARGS__)

#define ADMIN_REPO "gitorium-admin.git"

#endif // COMMON_H_INCLUDED

