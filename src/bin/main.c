#include <string.h>
#include <libconfig.h>
#include "common.h"

typedef struct
{
    const char *cmd;
    int (*fn)(int, char **);
    int (*help_fn)(int, char **);
} cmd_t;


static cmd_t commands[] =
{
    {"setup", cmd_setup, cmd_setup_help}
};

static int main_help(int argc, char **argv)
{
    puts("gitorium <command> [<args>]\n"
         "\n"
         "For more information on a particular command, please run gitorium help <command>\n"
         "\n"
         "Available commands:");
    for (int i = 0; i < ARRAY_SIZE(commands); i++)
    {
        cmd_t *p = commands+i;
        printf("\t%s\n", p->cmd);
    }
    return 0;
}

static int run_cmd(int (*fn)(int, char **), int argc, char **argv)
{
    return fn(argc, argv);
}

static int handle_command(int argc, char **argv)
{
    const char *cmd;

    // Turn "cmd --help" into "help cmd"
    if (argc > 1 && !strcmp(argv[1], "--help"))
    {
        argv[1] = argv[0];
        argv[0] = "help";
    }

    if (!strcmp("help", argv[0]))
    {
        cmd = argv[1];
    }
    else
    {
        cmd = argv[0];
    }

    for (int i = 0; i < ARRAY_SIZE(commands); i++)
    {
        cmd_t *p = commands+i;
        if (strcmp(p->cmd, cmd))
            continue;

        if (!strcmp("help", argv[0]))
            return run_cmd(p->help_fn, argc, argv);
        else
            return run_cmd(p->fn, argc, argv);
    }

    PRINTF_ERROR("The command '%s' does not exist.", cmd)
    main_help(argc, argv);

    return EXIT_FAILURE;
}

static void init(void)
{
    config_t cfg;
    FILE *cFile;

    config_init(&cfg);

    if ((cFile = fopen(RC_FILE, "r")))
    {
        if(!config_read(&cfg, cFile))
        {
            PRINT_ERROR("Could not read configuration")
            fprintf(stderr, RC_FILE":%d - %s\n", config_error_line(&cfg),
                    config_error_text(&cfg));
            config_destroy(&cfg);
            exit(EXIT_FAILURE);
        }
    }
    else
    {
        config_setting_t *root, *setting;

        root = config_root_setting(&cfg);

        setting = config_setting_add(root, "repositories", CONFIG_TYPE_STRING);
        config_setting_set_string(setting, "/var/repositories");

        if(!config_write_file(&cfg, RC_FILE))
        {
            PRINT_ERROR("Error while writing configuration.")
            config_destroy(&cfg);
            exit(EXIT_FAILURE);
        }
    }

    config_destroy(&cfg);
}

int main(int argc, char **argv)
{
    // We remove the name of the executable from the list
    argv++;
    argc--;

    init();

    if ((argc == 0) || (argc == 1 && !strcmp("help", argv[0])))
    {
        return run_cmd(main_help, argc, argv);
    }
    else
    {
        return handle_command(argc, argv);
    }

    return EXIT_FAILURE;
}
