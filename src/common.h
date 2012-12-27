#ifndef COMMON_H_INCLUDED
#define COMMON_H_INCLUDED

#define _GNU_SOURCE

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "config.h"
#include "specific.h"

#define ARRAY_SIZE(x) (sizeof(x)/sizeof(x[0]))
#define PRINT_ERROR(s) fprintf(stderr, s"\n");
#define PRINTF_ERROR(s, f) fprintf(stderr, s"\n", f);

#define ADMIN_REPO "gitorium-admin.git"

#endif // COMMON_H_INCLUDED

