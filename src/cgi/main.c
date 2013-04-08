#include "main.h"

static void http_header(const char *field, const char *value)
{
	printf("%s: %s\n", field, value);
}

static void http_status(const int status, const char *desc)
{
	printf("Status: %i %s\n", status, desc);
}

static void http_date(const char *name, const unsigned long offset)
{
	time_t now = time(NULL);
	now += offset;
	struct tm *timer = gmtime(&now);
	char value[29];
	strftime(value, sizeof(char) * strlen(value), "%a, %d %b %Y %H:%M:%S %Z", timer);
	http_header(name, value);
}

static inline void http_cache_none(void)
{
	http_header("Expires", "Fri, 01 Jan 1980 00:00:00 GMT");
	http_header("Pragma", "no-cache");
	http_header("Cache-Control", "no-cache, max-age=0, must-revalidate");
}

static inline void http_cache_forever(void)
{
	http_date("Expires", 31536000);
	http_header("Cache-Control", "public, max-age=31536000");
}

static inline void http_end_headers(void)
{
	printf("\n");
}

static void exec__redirect_stdio(void *payload)
{
	dup2(fileno(stdin), 0);
	dup2(fileno(stdout), 1);
	dup2(fileno(stderr), 2);
}

static void get_info_refs(const char *loc)
{
	http_status(200, "OK");
	http_header("Content-Type", "application/x-git-upload-pack-advertisement");
	http_cache_none();
	http_end_headers();

	gitio_fwrite(stdout, "# service=git-upload-pack\n");
	gitio_fflush(stdout);

	gitorium_execlp(&exec__redirect_stdio, NULL, "git-upload-pack", "--stateless-rpc", "--advertise-refs", loc, (char *) NULL);
}

static void post_git_upload_pack(const char *loc)
{
	http_status(200, "OK");
	http_header("Content-Type", "application/x-git-upload-pack-result");
	http_cache_none();
	http_end_headers();

	gitorium_execlp(&exec__redirect_stdio, NULL, "git-upload-pack", "--stateless-rpc", loc, (char *) NULL);
}

static struct cmd_service {
	const char *method;
	const char *exp;
	void (*fn)(const char *);
} services[] = {
	{"GET",     "/info/refs$",          &get_info_refs},
	{"POST",    "/git-upload-pack$",    &post_git_upload_pack},
	{NULL}
};

int main(void)
{
	gitorium__config_init();

	mainloop:
	while (FCGI_Accept() >= 0)   {
		char *method = getenv("REQUEST_METHOD");
		char *doc_uri = getenv("DOCUMENT_URI");
		struct stat rStat;

		if (!method || !doc_uri)
		{
			http_status(500, "Internal Server Error");
			http_end_headers();
			fatal("No REQUEST_METHOD or DOCUMENT_URI was given.\n"
				"Check your server config.");
			continue;
		}
		if (!strcmp(method, "HEAD"))
			method = "GET";

		for (struct cmd_service *s = services; s->method; s++)
		{
			regex_t r;
			regmatch_t out[1];

			if (strcmp(method, s->method))
				continue;

			if (regcomp(&r, s->exp, REG_EXTENDED))
			{
				http_status(500, "Internal Server Error");
				http_end_headers();
				break;
			}
			if (regexec(&r, doc_uri, 1, out, 0))
				continue;

			char *rel = malloc(sizeof(char) * (out[0].rm_so));
			//@todo check malloc
			strncpy(rel, doc_uri, (out[0].rm_so));
			rel[out[0].rm_so] = '\0';
			regfree(&r);

			char *rPath;
			config_lookup_string(&aCfg, "repositories", (const char **)&rPath);

			char *loc = malloc(sizeof(char) * (strlen(rPath) + strlen(rel+1) + 1));
			//@todo check malloc
			strcat(strcpy(loc, rPath), rel+1);
			free(rel);

// we won't allow users to create repositories over http,
// so we'll 404 went the repository doesn't exist.
			if (stat(loc, &rStat))
			{
				http_status(404, "Not Found");
				http_end_headers();
				fatal("The specified repository does not exist.");
				break;
			}

			s->fn(loc);

			free(loc);
			goto mainloop;
		}

		http_status(404, "Not Found");
		http_end_headers();
		fatal("The given method and URL do not match anything known.");
	}

	gitorium__config_close();

	return 0;
}
