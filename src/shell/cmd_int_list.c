#include "cmd_int_list.h"

static int int_list_repos(void)
{
	config_t cfg;
	config_setting_t *setting;

	config_init(&cfg);
	if (gitorium__repo_config_load(&cfg))
	{
		config_destroy(&cfg);
		return GITORIUM_ERROR;
	}

	if (NULL == (setting = config_lookup(&cfg, "repositories")))
	{
		fatal("could not load the repository list");
		config_destroy(&cfg);
		return GITORIUM_ERROR;
	}

	puts("Here are the known repositories.");

	for (int i = 0; i < config_setting_length(setting); i++)
	{
		config_setting_t *elem = config_setting_get_elem(setting, i);
		char *name;

		config_setting_lookup_string(elem, "name", (const char **) &name);

		printf("\t%s\n", name);
	}

	return 0;
}

static int int_list_groups(void)
{
	config_t cfg;
	config_setting_t *setting;

	config_init(&cfg);
	if (gitorium__repo_config_load(&cfg))
	{
		config_destroy(&cfg);
		return GITORIUM_ERROR;
	}

	if (NULL == (setting = config_lookup(&cfg, "groups")))
	{
		fatal("could not load the group list");
		config_destroy(&cfg);
		return GITORIUM_ERROR;
	}

	puts("Here are the known groups.");

	for (int i = 0; i < config_setting_length(setting); i++)
	{
		config_setting_t *elem = config_setting_get_elem(setting, i);

		printf("\t%s\n\t", config_setting_name(elem));

		for (int j = 0; j < config_setting_length(elem); j++)
		{
			printf("%s ", config_setting_get_string_elem(elem, j));
		}
		puts("");
	}

	return 0;
}

int cmd_int_list(char *user, char *args[])
{
	char *opt;

	if (NULL == args[1])
		opt = "repos";
	else
		opt = args[1];

	switch (opt[0])
	{
		case 'r':
			return int_list_repos();
		case 'g':
			return int_list_groups();
		case 'u':
			return GITORIUM_ERROR;
		default:
			errorf("%s is not a known object", opt);
			cmd_int_list_help(user, args);
			return GITORIUM_ERROR;
	}
}

int cmd_int_list_help(char *user, char *args[])
{
	puts("list [repos|groups|users]");
	puts("");
	puts("Lists the known objects.");
	puts("If none is given, defaults to repositories.");
	return 0;
}
