#ifndef COMMON_H_INCLUDED
#define COMMON_H_INCLUDED

#define _GNU_SOURCE

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "config.h"
#include "specific.h"

#define ARRAY_SIZE(x) (sizeof(x)/sizeof(x[0]))

#define PRINT_ERROR(string)             PRINTF_ERROR("%s", string)
#define PRINTF_ERROR(format, string)    fprintf(stderr, format"\n", string);

#define PRINT_DEBUG(string)             PRINT_ERROR("debug: "string)
#define PRINTF_DEBUG(format, string)    PRINTF_ERROR("debug: "format, string)
#define PRINT_FATAL(string)             PRINT_ERROR("fatal: "string)
#define PRINTF_FATAL(format, string)    PRINTF_ERROR("fatal: "format, string)

#define ADMIN_REPO "gitorium-admin.git"

#define PERM_NO_ACCESS  0
#define PERM_READ       (1 << 0)
#define PERM_WRITE      (1 << 1)
#define PERM_FORCE      (1 << 2)

#endif // COMMON_H_INCLUDED

