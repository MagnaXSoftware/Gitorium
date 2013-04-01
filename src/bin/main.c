#include "main.h"

static struct bin_cmd
{
	const char *cmd;
	int (*fn)(int, char **);
	int (*help_fn)(int, char **);
	const char *desc;
} bin_cmds[] =
{
	{"setup",   cmd_setup,  cmd_setup_help,		"Initialize (or re-initialize) Gitorium."},
	{"config",  cmd_config, cmd_config_help,	"Retrieve or set Gitorium configuration."},
	{NULL}
};

static int main_help(int argc, char **argv)
{
	puts("gitorium <command> [<args>]\n"
		"\n"
		"For more information on a particular command, please run gitorium help <command>\n"
		"\n"
		"Available commands:");
	for (struct bin_cmd *p = bin_cmds; p->cmd; p++)
	{
		printf("\t%s: %s\n", p->cmd, p->desc);
	}
	return 0;
}

static int opt_version(void)
{
	puts("Gitorium "GITORIUM_VERSION
		"\n"
		"Copyright (c) MagnaX Software 2013");
	return 0;
}

static int handle_command(int argc, char **argv)
{
	if (0 == argc ||
		(1 == argc && !strcmp("help", argv[0])) ||
		!strcmp("--help", argv[0]) ||
		!strcmp("-h", argv[0]))
		return main_help(argc, argv);

	if (!strcmp("--version", argv[0]) ||
		!strcmp("-v", argv[0]))
		return opt_version();

	if (!strcmp("help", argv[0]))
	{
		argv[0] = argv[1];
		argv[1] = "help";
	}

	for (struct bin_cmd *p = bin_cmds; p->cmd; p++)
	{
		if (strcmp(p->cmd, argv[0]))
			continue;

		if (1 < argc &&
			(!strcmp("help", argv[1]) ||
				!strcmp("--help", argv[1]) ||
				!strcmp("-h", argv[1])))
			return p->help_fn(argc, argv);

		return p->fn(argc, argv);
	}

	errorf("The command '%s' does not exist.", argv[0]);
	main_help(argc, argv);

	return GITORIUM_ERROR;
}

int main(int argc, char **argv)
{
	argv++;
	argc--;

	int exit = GITORIUM_ERROR;

	gitorium__config_init();

	exit = handle_command(argc, argv);

	gitorium__config_close();

	return exit;
}
