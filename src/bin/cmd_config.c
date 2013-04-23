#include "cmd_config.h"

static int config__list_keys(void)
{
	config_setting_t *root = config_root_setting(&aCfg), *setting;
	int type;

	for (int i = 0; i < config_setting_length(root); i++)
	{
		setting = config_setting_get_elem((const config_setting_t *) root, (unsigned int) i);
		type = config_setting_type((const config_setting_t *) setting);

		switch (type)
		{
			case CONFIG_TYPE_INT:
				printf("%s: %i\n",
					config_setting_name((const config_setting_t *) setting),
					config_setting_get_int((const config_setting_t *) setting));
			break;
			case CONFIG_TYPE_INT64:
				printf("%s: %lli\n",
					config_setting_name((const config_setting_t *) setting),
					config_setting_get_int64((const config_setting_t *) setting));
			break;
			case CONFIG_TYPE_FLOAT:
				printf("%s: %f\n",
					config_setting_name((const config_setting_t *) setting),
					config_setting_get_float((const config_setting_t *) setting));
			break;
			case CONFIG_TYPE_STRING:
				printf("%s: %s\n",
					config_setting_name((const config_setting_t *) setting),
					config_setting_get_string((const config_setting_t *) setting));
			break;
			case CONFIG_TYPE_BOOL:
				printf("%s: %s\n",
					config_setting_name((const config_setting_t *) setting),
					(0 == config_setting_get_bool((const config_setting_t *) setting)) ? "false" : "true");
			break;
			case CONFIG_TYPE_ARRAY:
			case CONFIG_TYPE_LIST:
			case CONFIG_TYPE_GROUP:
			default:
				continue;
		}

	}

	return 0;
}

static int config__get_key(const char *key)
{
	config_setting_t *setting;
	int type;

	if ((setting = config_lookup(&aCfg, key)) == NULL)
	{
		error("No configuration found under the specified key");
		return EXIT_FAILURE;
	}

	type = config_setting_type((const config_setting_t *) setting);

	switch (type)
	{
		case CONFIG_TYPE_INT:
		printf("%s: %i\n",
			config_setting_name((const config_setting_t *) setting),
			config_setting_get_int((const config_setting_t *) setting));
		return 0;
		case CONFIG_TYPE_INT64:
		printf("%s: %lli\n",
			config_setting_name((const config_setting_t *) setting),
			config_setting_get_int64((const config_setting_t *) setting));
		return 0;
		case CONFIG_TYPE_FLOAT:
		printf("%s: %f\n",
			config_setting_name((const config_setting_t *) setting),
			config_setting_get_float((const config_setting_t *) setting));
		return 0;
		case CONFIG_TYPE_STRING:
		printf("%s: %s\n",
			config_setting_name((const config_setting_t *) setting),
			config_setting_get_string((const config_setting_t *) setting));
		return 0;
		case CONFIG_TYPE_BOOL:
		printf("%s: %s\n",
			config_setting_name((const config_setting_t *) setting),
			(0 == config_setting_get_bool((const config_setting_t *) setting)) ? "false" : "true");
		return 0;
		case CONFIG_TYPE_ARRAY:
		case CONFIG_TYPE_LIST:
		case CONFIG_TYPE_GROUP:
		default:
		error("I cannot display configuration keys of this type.");
		return EXIT_FAILURE;
	}

	return EXIT_FAILURE;
}

int cmd_config(int argc, char **argv)
{
// We remove the name of the executable from the list
	argv++;
	argc--;

	if (argc == 0)
	{
		cmd_config_help(argc, argv);
		return EXIT_FAILURE;
	}

	if (!strcmp("list", argv[0]))
		return config__list_keys();
	else if (!strcmp("get", argv[0]) && 2 == argc)
		return config__get_key(argv[1]);
	else
		cmd_config_help(argc, argv);

	return EXIT_FAILURE;
}

int cmd_config_help(int argc, char **argv)
{
	UNUSED(argc);
	UNUSED(argv);
	puts("gitorium config [ list | get <key> ]\n"
		"\n"
		"Retrieves configuration values.\n"
			"\tlist\n"
				"\t\tList all the keys in the configuration file.\n"
			"get <key>\n"
				"\t\tReturns the value of a key in the configuration file.\n");
	return 0;
}
