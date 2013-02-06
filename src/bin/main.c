#include "main.h"

typedef struct
{
    const char *cmd;
    int (*fn)(int, char **);
    int (*help_fn)(int, char **);
} cmd_t;

static cmd_t commands[] =
{
    {"setup",   cmd_setup,  cmd_setup_help},
    {"config",  cmd_config, cmd_config_help},
    {NULL, NULL, NULL}
};

static int main_help(int argc, char **argv)
{
    puts("gitorium <command> [<args>]\n"
         "\n"
         "For more information on a particular command, please run gitorium help <command>\n"
         "\n"
         "Available commands:");
    for (unsigned int i = 0; i < ARRAY_SIZE(commands)-1; i++)
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

    for (unsigned int i = 0; i < ARRAY_SIZE(commands); i++)
    {
        cmd_t *p = commands+i;
        if (strcmp(p->cmd, cmd))
            continue;

        if (!strcmp("help", argv[0]))
            return run_cmd(p->help_fn, argc, argv);
        else
            return run_cmd(p->fn, argc, argv);
    }

    errorf("The command '%s' does not exist.", cmd);
    main_help(argc, argv);

    return EXIT_FAILURE;
}

int main(int argc, char **argv)
{
    // We remove the name of the executable from the list
    argv++;
    argc--;

    int exit = EXIT_FAILURE;

    gitorium__config_init();

    if ((argc == 0) || (argc == 1 && !strcmp("help", argv[0])))
    {
        exit = run_cmd(main_help, argc, argv);
    }
    else
    {
        exit = handle_command(argc, argv);
    }

    gitorium__config_close();

    return exit;
}
