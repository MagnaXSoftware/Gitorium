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

static int compare_perms(const int required, const char *givenPerms)
{
    int permCode = PERM_NO_ACCESS;
    for (unsigned int i = 0; i < strlen(givenPerms); i++)
    {
        if ('R' == givenPerms[i])
            permCode |= PERM_READ;
        if ('W' == givenPerms[i])
            permCode |= PERM_WRITE;
        if ('F' == givenPerms[i])
            permCode |= PERM_FORCE;
    }
    return !((permCode & required) == required);
}

static int check_perms(const config_setting_t *perms, const int required, const char *user, const config_setting_t *groups)
{
    char *givenPerms;

    if (CONFIG_TRUE == config_setting_lookup_string(perms, user, (const char **) &givenPerms))
        return compare_perms(required, (const char *) givenPerms);

    for (int i = 0; i < config_setting_length(perms); i++)
    {
        char *group = (char *) config_setting_name(config_setting_get_elem(perms, i));

        if ('*' != group[0])
            continue;

        if (!strcmp(group, "*all"))
            continue;

        config_setting_t *members = config_setting_get_member(groups, (const char*) group);
        int out = 1;

        for (int j = 0; j < config_setting_length(members); j++)
        {
            if (!strcmp(config_setting_get_string_elem(members, j), user))
            {
                out = 0;
                break;
            }
        }

        if (out)
        	continue;

        if (CONFIG_TRUE == config_setting_lookup_string(perms, group, (const char **) &givenPerms))
            return compare_perms(required, (const char *) givenPerms);
    }

    if (CONFIG_TRUE == config_setting_lookup_string(perms, "*all", (const char **) &givenPerms))
        return compare_perms(required, (const char *) givenPerms);

    return EXIT_FAILURE;
}

static struct commands
{
    const char *name;
    const int perms;
} cmd_list[] =
{
    { "git-receive-pack", PERM_WRITE },
    { "git-upload-pack", PERM_READ },
    { "git-upload-archive", PERM_READ },
    { NULL },
};

static int run_git_cmd(char *user, char *orig)
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
        PRINT_FATAL("could not load the repository list")
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

    for (struct commands *cmd = cmd_list; cmd->name ; cmd++)
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

            if (check_perms(config_setting_get_member(repo, "perms"), cmd->perms, (const char *) user, config_lookup(&cfg, "groups")))
            {
                config_destroy(&cfg);
                free(imName);
                PRINT_FATAL("insufficient permissions")
                return EXIT_FAILURE;
            }
        }

        pid_t pID = fork();
        if (pID == 0)                // child
        {
            char *rFullpath = malloc(sizeof(char) * (strlen(rPath) + strlen(mName) + 4 + 1));
            strcat(strcat(strcpy(rFullpath, rPath), mName), ".git");
            execlp(cmd->name, cmd->name, rFullpath, (char *) NULL);
            _exit(1);
        }
        else if (pID < 0)            // failed to fork
        {
            PRINTF_FATAL("failed to launch %s", cmd->name)
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

static int run_shell(char *user)
{
    int done = 0;

    do
    {
        char *line, **args;

        fprintf(stderr, "gitorium (%s)> ", user);
        get_line(&line);

        if (line[0] == '\0')
        {
            free(line);
            break;
        }

        split_args(&args, line);

        if (!strcmp(args[0], "quit") || !strcmp(args[0], "exit"))
        {
            done = 1;
//        } else if (is_remote_command_valid(args[0])) {
//            call_remote_command(user, args);
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
            exit = run_shell(argv[0]);
        }
        else
        {
            // Running non-interactive (only support git commands ATM)
            exit = run_git_cmd(argv[0], soc);
        }

        gitorium__config_close();
    }

    return exit;
}
