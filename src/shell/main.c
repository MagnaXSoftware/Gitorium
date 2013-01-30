#include "main.h"

static void get_line(char **linep)
{
    char *line = malloc(LINE_BUFFER_SIZE);
    int len = LINE_BUFFER_SIZE, c;
    *linep = line;

    if(line == NULL)
        return;

    for(;;)
    {
        c = fgetc(stdin);
        if(c == EOF || c == '\n')
            break;

        if(--len == 0)
        {
            char *linen = realloc(*linep, sizeof *linep + LINE_BUFFER_SIZE);
            if(linen == NULL)
                exit(-2);

            len = LINE_BUFFER_SIZE;
            line = linen + (line - *linep);
            *linep = linen;
        }

        *line++ = c;
    }

    *line = '\0';
}

static void split_args(char ***args, char *str)
{
    char **res = NULL, *p = strtok(str, " ");
    int n_spaces = 0;

    while (p)
    {
        res = realloc(res, sizeof (char*) * ++n_spaces);

        if (res == NULL)
            exit(-2);

        res[n_spaces-1] = p;
        p = strtok(NULL, " ");
    }

    res = realloc(res, sizeof(char*) * (n_spaces+1));
    res[n_spaces] = 0;
    *args = res;
}

static struct git_exec
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

    char *mName = malloc(sizeof(char) * (strlen(args[1]) + 1)), *imName = mName;
    strcpy(mName, args[1]);

    config_t cfg;
    config_setting_t *setting;

    config_lookup_string(&aCfg, "repositories", (const char **)&rPath);

    config_init(&cfg);
    if (gitorium__repo_config_load(&cfg))
    {
        config_destroy(&cfg);
        return EXIT_FAILURE;
    }

    if ((setting = config_lookup(&cfg, "repositories")) == NULL)
    {
        fatal("could not load the repository list")
        config_destroy(&cfg);
        return EXIT_FAILURE;
    }

    if ('\'' == mName[0])
        mName++;

    if ('\'' == mName[strlen(mName)-1])
        mName[strlen(mName)-1] = 0;

    if ('/' == mName[strlen(mName)-1])
        mName[strlen(mName)-1] = 0;

    if (!strcmp(".git", &mName[strlen(mName)-4]))
        mName[strlen(mName)-4] = 0;

    for (struct git_exec *cmd = cmd_list; cmd->name ; cmd++)
    {
        if (strcmp(cmd->name, args[0]))
            continue;

        int count = config_setting_length(setting);

        for (int i = 0; i < count; i++)
        {
            config_setting_t *repo = config_setting_get_elem(setting, i);
            char *name;

            config_setting_lookup_string(repo, "name", (const char **) &name);

            if (strcmp(name, mName))
                continue;

            if (perms_check(config_setting_get_member(repo, "perms"), cmd->perms, (const char *) user, config_lookup(&cfg, "groups")))
            {
                config_destroy(&cfg);
                free(imName);
                fatal("insufficient permissions")
                return EXIT_FAILURE;
            }
        }

        pid_t pID = fork();
        if (pID == 0)                // child
        {
            char *rFullpath = malloc(sizeof(char) * (strlen(rPath) + strlen(mName) + 4 + 1));
            strcat(strcat(strcpy(rFullpath, rPath), mName), ".git");
            setenv("GITORIUM_USER", user, 1); // THat way we know who is accessing the shell
            execlp(cmd->name, cmd->name, rFullpath, (char *) NULL);
            _exit(1);
        }
        else if (pID < 0)            // failed to fork
        {
            fatalf("failed to launch %s", cmd->name)
            free(imName);
            config_destroy(&cfg);
            return EXIT_FAILURE;
        }

        config_destroy(&cfg);

        waitpid(pID, NULL, 0);
        free(imName);
        return 0;
    }

    return EXIT_FAILURE;
}

static struct shell_commands
{
    const char *cmd;
    int (*fn)(char *, char **);
    int (*help_fn)(char *, char **);
} commands[] =
{
    {"list",   cmd_int_list,  cmd_int_list_help},
    {NULL, NULL, NULL}
};

static int (*is_command_valid(char *argv[]))(char *, char **)
{
    for (unsigned int i = 0; i < ARRAY_SIZE(commands); i++)
    {
        if (!strcmp(argv[0], "help"))
        {
            if(!strcmp(commands[i].cmd, argv[1]))
                return commands[i].help_fn;
        }

        if (!strcmp(commands[i].cmd, argv[0]))
            return commands[i].fn;
    }
    return NULL;
}

static int run_interactive(char *user)
{
    int done = 0;

    do
    {
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
        {
            done = 1;
        } else if ((fn = is_command_valid(args)) != NULL) {
            fn(user, args);
        }
        else
            fprintf(stderr, "The command does not exist.\n");

        free(line);
        free(args);
    }
    while (!done);

    return 0;
}

int main(int argc, char **argv)
{
    // We remove the name of the executable from the list
    argv++;
    argc--;

    int exit = EXIT_FAILURE;

    if (argc < 1)
    {
        // Someone tried to call us directly
        fprintf(stderr, "You must call this from a shell.\n");
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
