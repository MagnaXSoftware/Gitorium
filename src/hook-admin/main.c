#include "main.h"

static int hook_admin__repo_update(void)
{
    config_t cfg;
    config_setting_t *setting;

    config_init(&cfg);
    if (gitorium__repo_config_load(&cfg))
    {
        error("Could not get repo configuration.");
        config_destroy(&cfg);
        return EXIT_FAILURE;
    }

    if ((setting = config_lookup(&cfg, "repositories")) == NULL)
    {
        error("Could not load the repositories.");
        config_destroy(&cfg);
        return EXIT_FAILURE;
    }

    int count = config_setting_length(setting);

    for (int i = 0; i < count; i++)
    {
        config_setting_t *repo;
        char *name;

        repo = config_setting_get_elem(setting, i);
        config_setting_lookup_string(repo, "name", (const char**) &name);

        if ('~' == name[0]) //we ignore personal repos
            continue;

        repo_create(name);
    }

    config_destroy(&cfg);

    return 0;
}

static int check_ref(char *ref, char *newid)
{
    if (strcmp("refs/heads/master", ref))
        return 0; // We don't care about refs that aren't the master.

    if (!strcmp("0000000000000000000000000000000000000000", newid))
        return EXIT_FAILURE; // No one can delete the master. Ever.

    return 0;
}

int main(int argc, char **argv)
{
    int exit = EXIT_FAILURE;

    gitorium__config_init();

    if (!strcmp("hooks/post-update", argv[0]))
    {
        // No error handling here because we must run the whole thing no matter what
        for (int i = 1; i < argc; i++)
        {
            if (!strcmp("refs/heads/master", argv[i]))
            {
                ssh_setup();
                hook_admin__repo_update();
            }
        }
        exit = 0;
    }
    else if (!strcmp("hooks/update", argv[0]))
    {
        exit = check_ref(argv[1], argv[3]);
    }

    gitorium__config_close();

    return exit;
}
