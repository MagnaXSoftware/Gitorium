#include "repo.h"

int repo_create(char *name)
{
    char *nFullpath, *rPath;
    struct stat rStat;
    int ret = EXIT_FAILURE;

    config_lookup_string(&aCfg, "repositories", (const char **)&rPath);

    nFullpath = malloc(sizeof(char) * (strlen(rPath) + strlen(name) + strlen(".git") + 1));
    strcat(strcat(strcpy(nFullpath, rPath), name), ".git");

    if (stat(nFullpath, &rStat))
    {
        git_repository *nRepo;

        printf("Creating repo: %s\n", name);
        fflush(stdout);

        ret = git_repository_init(&nRepo, nFullpath, 1);
        git_repository_free(nRepo);

        if (!ret)
        {
            char *hFullpath = malloc(sizeof(char) * (strlen(nFullpath) + strlen("/hooks/post-update") + 1));
            strcat(strcpy(hFullpath, nFullpath), "/hooks/post-update");
            symlink(CMAKE_INSTALL_PREFIX"/bin/gitorium-hook-post-update", hFullpath);

            hFullpath = realloc(hFullpath, sizeof(char) * (strlen(nFullpath) + strlen("/hooks/update") + 1));
            strcat(strcpy(hFullpath, nFullpath), "/hooks/update");
            symlink(CMAKE_INSTALL_PREFIX"/bin/gitorium-hook-update", hFullpath);

            free(hFullpath);
        }
    }
    else
        ret = 0;

    free(nFullpath);

    return ret;
}
