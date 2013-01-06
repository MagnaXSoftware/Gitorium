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
        error("No configuration found under the specified key")
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
            error("I cannot display configuration keys of this type.")
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
    else if (!strcmp("get", argv[0]) && argc > 1)
        return config__get_key(argv[1]);
    else
        cmd_config_help(argc, argv);

    return EXIT_FAILURE;
}

int cmd_config_help(int argc, char **argv)
{
    puts("gitorium config [ list | get <key> | set <key> <value> ]\n"
         "\n"
         "Retrieves and set configuration values.\n"
         "\n"
         "list\n"
         "\tList all the keys in the configuration file.\n"
         "get <key>\n"
         "\tReturns the value of a key in the configuration file.\n"
         "set <key> <value>\n"
         "\tSets the value of <key> to <value>\n"
         "\tLegal values consist of a one character type followed by the string representation.\n"
         "\tValid types are:\n"
         "\t\ti for int\n"
         "\t\tl for long long (int64)\n"
         "\t\tf for double (float)\n"
         "\t\tb for boolean (only valid string representations are true and false)\n"
         "\t\ts for string\n");
    return 0;
}
