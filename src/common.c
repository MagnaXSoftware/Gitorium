#include "common.h"

#define MAX_ARGS 32

int gitorium_execvp(void (*cb)(void *), void *payload, const char **argv)
{
	pid_t pID = fork();
	if (pID == 0)                // child
	{
		if (NULL != cb)
			(*cb)(payload);
		execvp(argv[0], (char * const *) argv);
		_exit(1);
	}
	else if (pID < 0)            // failed to fork
	{
		return GITORIUM_ERROR;
	}

	waitpid(pID, NULL, 0);

	return 0;
}

int gitorium_execlp(void (*cb)(void *), void *payload, const char *file, const char *arg0, ...)
{
	va_list param;
	int argc;
	const char *argv[MAX_ARGS + 1];

	va_start(param, arg0);
	argv[0] = file;
	argv[1] = arg0;
	argc = 2;
	while (argc < MAX_ARGS) {
		if (NULL == (argv[argc++] = va_arg(param, char *)))
			break;
	}
	va_end(param);

	argv[argc] = NULL; // this seems redundant

	return gitorium_execvp(cb, payload, argv);
}

int rrmdir(const char *dir)
{
	return gitorium_execlp(NULL, NULL, "rm", "-rf", dir, (char *) NULL);
}

int strprecmp(const char *str, const char *prefix)
{
	for (;;str++, prefix++)
	{
		if (!*prefix)
			return 0;
		else if (*str != *prefix)
			return (unsigned char)*prefix - (unsigned char)*str;
	}
}