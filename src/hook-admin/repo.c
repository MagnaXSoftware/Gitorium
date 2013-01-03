#include "repo.h"

static void hook_admin__repo_create(config_setting_t *repo)
{
    char *name, *nFullpath, *rPath;
    struct stat rStat;

    config_setting_lookup_string(repo, "name", (const char**) &name);

    config_lookup_string(&aCfg, "repositories", (const char **)&rPath);

    nFullpath = malloc(sizeof(char) * (strlen(rPath) + strlen(name) + strlen(".git") + 1));
    strcat(strcat(strcpy(nFullpath, rPath), name), ".git");

    if (stat(nFullpath, &rStat))
    {
        git_repository *nRepo;

        printf("Creating repo: %s\n", name);

        git_repository_init(&nRepo, nFullpath, 1);
        git_repository_free(nRepo);

        char *hFullpath = malloc(sizeof(char) * (strlen(nFullpath) + strlen("/hooks/post-update") + 1));
        strcat(strcpy(hFullpath, nFullpath), "/hooks/post-update");
        symlink(CMAKE_INSTALL_PREFIX"/bin/gitorium-hook-post-update", hFullpath);

        hFullpath = realloc(hFullpath, sizeof(char) * (strlen(nFullpath) + strlen("/hooks/update") + 1));
        strcat(strcpy(hFullpath, nFullpath), "/hooks/update");
        symlink(CMAKE_INSTALL_PREFIX"/bin/gitorium-hook-update", hFullpath);

        free(hFullpath);
    }

    free(nFullpath);
}

int repo_update(void)
{
    config_t cfg;
    config_setting_t *setting;

    config_init(&cfg);
    if (gitorium__repo_config_load(&cfg))
    {
        PRINT_ERROR("Could not get repo configuration.")
        config_destroy(&cfg);
        return EXIT_FAILURE;
    }

    if ((setting = config_lookup(&cfg, "repositories")) == NULL)
    {
        PRINT_ERROR("Could not load the repositories.")
        config_destroy(&cfg);
        return EXIT_FAILURE;
    }

    int count = config_setting_length(setting);

    for (int i = 0; i < count; i++)
    {
        hook_admin__repo_create(config_setting_get_elem(setting, i));
    }

    config_destroy(&cfg);

    return 0;
}
