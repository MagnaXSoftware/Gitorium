#include "gitio.h"

#define _FORMAT_SIZE 1000
static char buffer[_FORMAT_SIZE];
static int git_vformat(const char *format, va_list args)
{
    static char hexchar[] = "0123456789abcdef";
    int n;

    n = vsnprintf(buffer + 4, _FORMAT_SIZE - 4, format, args);
    if (n >= (signed int) sizeof(buffer)-4)
        return -1;

    n += 4;

    buffer[0] = hexchar[(n >> 12) & 15];
    buffer[1] = hexchar[(n >> 8) & 15];
    buffer[2] = hexchar[(n >> 4) & 15];
    buffer[3] = hexchar[n & 15];

    return n;
}

void git_sformat(char **out, const char *format, ...)
{
    va_list args;
    va_start(args, format);
    git_vformat(format, args);
    va_end(args);
    *out = malloc(sizeof(char) * (strlen(buffer) + 1));
    strcpy(*out, buffer);
}
