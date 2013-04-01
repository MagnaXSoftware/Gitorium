#include "main.h"

static int split_args(char ***args, char *str)
{
	char **res = NULL, *p = strtok(str, " ");
	int n_spaces = 0, i = 0;

	while (p)
	{
		res = realloc(res, sizeof (char*) * ++n_spaces);

		if (res == NULL)
			return GITORIUM_MEM_ALLOC;

		res[n_spaces-1] = p;
		i++;
		p = strtok(NULL, " ");
	}

	res = realloc(res, sizeof(char*) * (n_spaces+1));
	if (res == NULL)
		return GITORIUM_MEM_ALLOC;

	res[n_spaces] = 0;
	*args = res;

	return i;
}

static void exec__setup_interactive(void *payload)
{
	setenv("GITORIUM_USER", (char *)payload, 1);
}


static struct non_interactive_cmd
{
	const char *name;
	const int perms;
	const char *dir;
} cmd_list[] =
{
	{ "git-receive-pack",   PERM_WRITE, "push" },
	{ "git-upload-pack",    PERM_READ , "pull" },
	{ "git-upload-archive", PERM_READ , "pull" },
	{ NULL },
};

static int run_non_interactive(const char *user, char *orig)
{
	char **args, *rPath;

	split_args(&args, orig);

	char *repoName = malloc(sizeof(char) * (strlen(args[1]) + 1)), *irepoName = repoName;
	if (NULL == repoName)
		return GITORIUM_MEM_ALLOC;

	strcpy(repoName, args[1]);
	repoName = repo_massage(repoName);

	config_lookup_string(&aCfg, "repositories", (const char **)&rPath);

	for (struct non_interactive_cmd *cmd = cmd_list; cmd->name ; cmd++)
	{
		if (strcmp(cmd->name, args[0]))
			continue;

		char *rFullpath = malloc(sizeof(char) * (strlen(rPath) + strlen(repoName) + 4 + 1));
		if (NULL == rFullpath)
		{
			free(irepoName);
			return GITORIUM_MEM_ALLOC;
		}
		strcat(strcat(strcpy(rFullpath, rPath), repoName), ".git");

		if ('~' == repoName[0]) //user dir
		{
			if (strprecmp(&repoName[1], user))
			{
				fatal("insufficient permissions");
				free(rFullpath);
				free(irepoName);
				return GITORIUM_ERROR;
			}

			struct stat rStat;

			if (stat(rFullpath, &rStat))
			{
				char *rPartpath = malloc(sizeof(char) * (strlen(rPath) + 1 + strlen(user) + 1));
				if (NULL == rPartpath)
				{
					free(rFullpath);
					free(irepoName);
					return GITORIUM_MEM_ALLOC;
				}
				strcat(strcat(strcpy(rPartpath, rPath), "~"), user);

				if (stat(rPartpath, &rStat))
					mkdir(rPartpath, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);

				free(rPartpath);
				repo_create(repoName);
			}
		}
		else
		{
			config_t cfg;
			config_setting_t *setting;

			config_init(&cfg);
			if (gitorium__repo_config_load(&cfg))
			{
				config_destroy(&cfg);
				free(rFullpath);
				free(irepoName);
				return GITORIUM_ERROR;
			}

			if (NULL == (setting = config_lookup(&cfg, "repositories")))
			{
				fatal("could not load the repository list");
				config_destroy(&cfg);
				free(rFullpath);
				free(irepoName);
				return GITORIUM_ERROR;
			}

			for (int i = 0; i < config_setting_length(setting); i++)
			{
				config_setting_t *repo = config_setting_get_elem(setting, i);
				char *name;

				config_setting_lookup_string(repo, "name", (const char **) &name);

				if (strcmp(name, repoName))
					continue;

				if (perms_check(config_setting_get_member(repo, "perms"), cmd->perms, (const char *) user, config_lookup(&cfg, "groups")))
				{
					fatal("insufficient permissions");
					config_destroy(&cfg);
					free(rFullpath);
					free(irepoName);
					return GITORIUM_ERROR;
				}
				else
					break;
			}

			config_destroy(&cfg);
		}

		free(irepoName);

		if (gitorium_execlp(&exec__setup_interactive, (void *) user, cmd->name, rFullpath, (char *) NULL))
		{
			fatalf("failed to launch %s", cmd->name);
			free(rFullpath);
			return GITORIUM_ERROR;
		}

		return 0;
	}

	return GITORIUM_ERROR;
}

static int get_line(char **linep)
{
	char *line = malloc(LINE_BUFFER_SIZE);
	int len = LINE_BUFFER_SIZE, c;
	*linep = line;

	if(line == NULL)
		return GITORIUM_MEM_ALLOC;

	for(;;)
	{
		c = fgetc(stdin);
		if(c == EOF || c == '\n')
			break;

		if(--len == 0)
		{
			char *linen = realloc(*linep, sizeof *linep + LINE_BUFFER_SIZE);
			if(linen == NULL)
				return GITORIUM_MEM_ALLOC;

			len = LINE_BUFFER_SIZE;
			line = linen + (line - *linep);
			*linep = linen;
		}

		*line++ = c;
	}

	*line = 0;
	return 0;
}

static int interactive_help(char *user, char *argv[])
{
	return 0;
}

static struct interactive_cmd
{
	const char *name;
	int (*fn)(char *, char **);
	int (*help_fn)(char *, char **);
} int_cmds[] =
{
	{"list",   &cmd_int_list,   &cmd_int_list_help},
	{"fsck",   &cmd_int_fsck,   &cmd_int_fsck_help},
	{NULL}
};

static int (*is_command_valid(char *argv[]))(char *, char **)
{
	if (NULL == argv[0] ||
		(!strcmp("help", argv[0]) && NULL == argv[1]))
		return interactive_help;

	for (struct interactive_cmd *cmd = int_cmds; cmd->name ; cmd++)
	{
		if (!strcmp(argv[0], "help"))
		{
			if(!strcmp(cmd->name, argv[1]))
				return cmd->help_fn;
		}

		if (!strcmp(cmd->name, argv[0]))
			return cmd->fn;
	}
	return NULL;
}
/*
static void trap_sigint(int sig)
{
	//@todo I want SIGINT to clear the current line and start a new one.
}*/

static int run_interactive(char *user)
{
	//signal(SIGINT, trap_sigint);

	int done = 0;

	do
	{
		/*
		I would need to do something line
		start_line()
		parse_line()
		end_line()

		This way, I can call end_line() prematurely
		*/
		char *line, **args;
		int (*fn)(char *, char **);

		fprintf(stderr, "gitorium (%s)> ", user);
		get_line(&line);

		if (line[0] == '\0')
		{
			free(line);
			break;
		}

		split_args(&args, line);

		if (!strcmp(args[0], "quit") || !strcmp(args[0], "exit") ||
			!strcmp(args[0], "logout") || !strcmp(args[0], "bye"))
			done = 1;
		else if ((fn = is_command_valid(args)) != NULL)
			fn(user, args);
		else
			error("The command does not exist.");

		free(line);
		free(args);
	}
	while (!done);

	return 0;
}

int main(int argc, char **argv)
{
	argv++;
	argc--;

	int exit = GITORIUM_ERROR;

	if (argc < 1)
	{
		error("You cannot call the shell directly.");
		error("Please use 'gitorium' instead.");
	}
	else
	{
		char *soc = getenv("SSH_ORIGINAL_COMMAND");

		gitorium__config_init();

		if (NULL == soc)
		{
			// Running interactive (first argument is the user's name)
			exit = run_interactive(argv[0]);
		}
		else
		{
			// Running non-interactive (only support git commands ATM)
			exit = run_non_interactive(argv[0], soc);
		}

		gitorium__config_close();
	}

	return exit;
}
