#ifndef COMMON_H_INCLUDED
#define COMMON_H_INCLUDED

#include <stdio.h>

#include "config.h"

#define ARRAY_SIZE(x) (sizeof(x)/sizeof(x[0]))
#define PRINT_ERROR(s) fprintf(stderr, s"\n");
#define PRINTF_ERROR(s, f) fprintf(stderr, s"\n", f);

#define RC_FILE "/etc/default/config.cfg"

#endif // COMMON_H_INCLUDED

