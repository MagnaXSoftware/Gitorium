#include "gitio.h"

static ssize_t gitio__fwrite(FILE *stream, const void *buf, ssize_t len, ssize_t size)
{
	ssize_t nn = len;
	while (len)
	{
		// FCGI_fwrite's first argument is not a const. It should be.
		int ret = fwrite((void *) buf, size, len, stream);
		if (ret > 0)
		{
			buf = (char *) buf + ret;
			len -= ret;
			continue;
		}
		if (!ret)
			return -1;
	}
	return nn;
}

#define _FORMAT_SIZE 1000 //max length of a pkt-line
static char buffer[_FORMAT_SIZE];

static unsigned int gitio__vformat(const char *format, va_list args)
{
	static char hexchar[] = "0123456789abcdef";
	int n;

	n = vsnprintf(buffer + 4, _FORMAT_SIZE - 4, format, args);
	if (n >= _FORMAT_SIZE - 4)
		return -1;

	n += 4;

	buffer[0] = hexchar[(n >> 12) & 15];
	buffer[1] = hexchar[(n >> 8) & 15];
	buffer[2] = hexchar[(n >> 4) & 15];
	buffer[3] = hexchar[n & 15];

	return n;
}

void gitio_fflush(FILE *stream)
{
	gitio__fwrite(stream, "0000", 4, sizeof(char));
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

void gitio_fwrite(FILE *stream, const char *format, ...)
{
	va_list args;
	unsigned int n;

	va_start(args, format);
	n = gitio__vformat(format, args);
	va_end(args);

	gitio__fwrite(stream, buffer, n, sizeof(char));
}

void gitio_write(const char *format, ...)
{
	va_list args;
	unsigned int n;

	va_start(args, format);
	n = gitio__vformat(format, args);
	va_end(args);

	gitio__fwrite(stdout, buffer, n, sizeof(char));
}

/*
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
}*/

void gitio_truncate(char *buffer, ssize_t size)
{
	buffer[size-1] = 0;
}
