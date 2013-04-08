#ifndef MAIN_H_INCLUDED
#define MAIN_H_INCLUDED

#define _GNU_SOURCE

#include "../common.h"

#include <regex.h>
#include <time.h>

#include "../gitio.h"
#include "../repo.h"

#define has_param(param) ((NULL == strstr(getenv("QUERY_STRING"), param)) ? 1 : 0)

#endif // MAIN_H_INCLUDED

