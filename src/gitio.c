#include "gitio.h"

static size_t gitio__fwrite(FILE *stream, const void *buf, size_t len, size_t size)
{
	size_t nn = len;
	while (len)
	{
		// FCGI_fwrite's first argument is not a const. It should be.
		size_t ret = fwrite((void *) buf, size, len, stream);
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

static char buffer_out[DEFAULT_PACKET_SIZE];

static unsigned int gitio__vformat(const char *format, va_list args)
{
	static char hexchar[] = "0123456789abcdef";
	int n;

	n = vsnprintf(buffer_out + 4, DEFAULT_PACKET_SIZE - 4, format, args);
	if (n >= DEFAULT_PACKET_SIZE - 4)
		return -1;

	n += 4;

	buffer_out[0] = hexchar[(n >> 12) & 15];
	buffer_out[1] = hexchar[(n >> 8) & 15];
	buffer_out[2] = hexchar[(n >> 4) & 15];
	buffer_out[3] = hexchar[n & 15];

	return n;
}

unsigned int gitio_sformat(char **out, const char *format, ...)
{
	va_list args;
	unsigned int n;

	va_start(args, format);
	n = gitio__vformat(format, args);
	va_end(args);

	*out = malloc(sizeof(char) * (strlen(buffer_out) + 1));
	strcpy(*out, buffer_out);
	return n;
}

void gitio_fwrite(FILE *stream, const char *format, ...)
{
	va_list args;
	unsigned int n;

	va_start(args, format);
	n = gitio__vformat(format, args);
	va_end(args);

	gitio__fwrite(stream, buffer_out, n, sizeof(char));
}

void gitio_fflush(FILE *stream)
{
	gitio__fwrite(stream, "0000", 4, sizeof(char));
}

void gitio_write(const char *format, ...)
{
	va_list args;
	unsigned int n;

	va_start(args, format);
	n = gitio__vformat(format, args);
	va_end(args);

	gitio__fwrite(stdout, buffer_out, n, sizeof(char));
}

int gitio_fread(FILE *stream, char *buffer, size_t size)
{
	// a uint16_t would be enough to store the maximum size allowed by git's
	// specifications, but we don't want to rush it.
	long int len;
	size_t ret;
	char linelen[5];

	fgets(linelen, 5, stream);

	linelen[4] = 0;

	len = strtol(linelen, NULL, 16);

	if (0 > len || (0 < len && len < 4))
	{
		errorf("protocol error: bad line length character: %.4s", linelen);
		return 0;
	}

	if (0 == len)
		return 0;

	len -= 4;

	if ((unsigned long int) len >= size)
	{
		errorf("protocol error: bad line length %li", len);
		return 0;
	}

	ret = fread(buffer, sizeof(char), len, stream);
	buffer[len] = 0;
	return ret;
}

static char buffer_in[LARGE_PACKET_SIZE];

char *gitio_fread_line(FILE *stream)
{
	int ret = gitio_fread(stream, buffer_in, LARGE_PACKET_SIZE);

	if (0 == ret)
		return NULL;
	else
		return buffer_in;
}

void gitio_truncate(char *buffer, size_t size)
{
	buffer[size-1] = 0;
}
