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

    char *repoName = malloc(sizeof(char) * (strlen(args[1]) + 1)), *irepoName = repoName;
    strcpy(repoName, args[1]);

    config_lookup_string(&aCfg, "repositories", (const char **)&rPath);

    if ('\'' == repoName[0])
        repoName++;

    if ('\'' == repoName[strlen(repoName)-1])
        repoName[strlen(repoName)-1] = 0;

    if ('/' == repoName[strlen(repoName)-1])
        repoName[strlen(repoName)-1] = 0;

    if (!strcmp(".git", &repoName[strlen(repoName)-4]))
        repoName[strlen(repoName)-4] = 0;

    for (struct git_exec *cmd = cmd_list; cmd->name ; cmd++)
    {
        if (strcmp(cmd->name, args[0]))
            continue;

        char *rFullpath = malloc(sizeof(char) * (strlen(rPath) + strlen(repoName) + 4 + 1));
        strcat(strcat(strcpy(rFullpath, rPath), repoName), ".git");

        if ('~' == repoName[0])
        {
            if (strprecmp(&repoName[1], user))
            {
                fatal("insufficient permissions");
                free(rFullpath);
                free(irepoName);
                return EXIT_FAILURE;
            }

            struct stat rStat;

            if (stat(rFullpath, &rStat))
            {
                char *rPartpath = malloc(sizeof(char) * (strlen(rPath) + 1 + strlen(user) + 1));
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
                return EXIT_FAILURE;
            }

            if ((setting = config_lookup(&cfg, "repositories")) == NULL)
            {
                fatal("could not load the repository list");
                config_destroy(&cfg);
                free(rFullpath);
                free(irepoName);
                return EXIT_FAILURE;
            }

            int count = config_setting_length(setting);

            for (int i = 0; i < count; i++)
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
                    return EXIT_FAILURE;
                }
                else
                    break; //we have permission, we don't need to see the rest of the repos.
            }

            config_destroy(&cfg);
        }

        free(irepoName);

            pid_t pID = fork();
            if (pID == 0)                // child
            {
                setenv("GITORIUM_USER", user, 1); // THat way we know who is accessing the shell
                execlp(cmd->name, cmd->name, rFullpath, (char *) NULL);
                _exit(1);
            }
            else if (pID < 0)            // failed to fork
            {
                fatalf("failed to launch %s", cmd->name);
                free(rFullpath);
                return EXIT_FAILURE;
            }

            waitpid(pID, NULL, 0);


        free(rFullpath);
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
    // We remove the name of the executable from the list
    argv++;
    argc--;

    int exit = EXIT_FAILURE;

    if (argc < 1)
    {
        // Someone tried to call us directly
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
