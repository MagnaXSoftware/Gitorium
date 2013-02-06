#include "gitio.h"

int strprecmp(const char *str, const char *prefix)
{
	for (;;str++, prefix++)
        if (!*prefix)
            return 0;
        else if (*str != *prefix)
            return (unsigned char)*prefix - (unsigned char)*str;
}

static ssize_t gitio__write(int fd, const void *buffer, ssize_t n)
{
    ssize_t nn = n;
    while (n)
    {
        int ret = write(fd, buffer, n);
        if (ret > 0)
        {
            buffer = (char *) buffer + ret;
            n -= ret;
            continue;
        }
        if (!ret)
            return -1;
    }
    return nn;
}

#define _FORMAT_SIZE 1004 //max length of a pkt-line
static char buffer[_FORMAT_SIZE];
static unsigned int gitio__vformat(const char *format, va_list args)
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

void gitio_flush(void)
{
    gitio__write(fileno(stdout), "0000", 4);
}

unsigned int gitio_sformat(char **out, const char *format, ...)
{
    va_list args;
    unsigned int n;

    va_start(args, format);
    n = gitio__vformat(format, args);
    va_end(args);

    *out = malloc(sizeof(char) * (strlen(buffer) + 1));
    strcpy(*out, buffer);
    return n;
}

void gitio_write(int fd, const char *format, ...)
{
    va_list args;
    unsigned int n;

    va_start(args, format);
    n = gitio__vformat(format, args);
    va_end(args);

    gitio__write(fd, buffer, n);
}

int gitio_read(int fd, char *buffer, ssize_t size)
{
    int len, ret;
    char linelen[5];

    read(fd, linelen, 4);

    linelen[4] = 0;

    len = strtoll(linelen, NULL, 16);

    if (len < 0)
    {
        errorf("protocol error: bad line length character: %.4s", linelen);
        return 0;
    }

    if (len == 0)
        return 0;

    len -= 4;

    if (len >= size)
    {
        errorf("protocol error: bad line length %i", len);
        return 0;
    }

    ret = read(fd, buffer, len);
    buffer[len] = 0;
    return ret;
}

void gitio_truncate(char *buffer, ssize_t size)
{
    buffer[size-1] = 0;
}
