#include "repo.h"

static void repo__create(config_setting_t *repo)
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
    }

    free(nFullpath);
}

static int repo__load_config(char **confOut)
{
    git_repository *bRepo;
    git_reference *bHead, *bRealHead;
    git_commit *hCommit;
    git_tree *hTree;
    git_tree_entry *conf;
    git_blob *blob;

    char *bFullpath, *rPath;

    config_lookup_string(&aCfg, "repositories", (const char **)&rPath);

    bFullpath = malloc(sizeof(ADMIN_REPO) + sizeof(char)*(strlen(rPath)+1));
    strcat(strcpy(bFullpath, rPath), ADMIN_REPO);

    if (git_repository_open(&bRepo, bFullpath))
    {
        PRINT_ERROR("Could not open the admin repository.")
        free(bFullpath);
        return EXIT_FAILURE;
    }

    free(bFullpath);

    if (git_repository_head(&bHead, bRepo))
    {
        PRINT_ERROR("Could not load the HEAD.")
        git_repository_free(bRepo);
        return EXIT_FAILURE;
    }

    if (git_reference_resolve(&bRealHead, bHead))
    {
        PRINT_ERROR("Could not resolve the HEAD.")
        git_reference_free(bHead);
        git_repository_free(bRepo);
        return EXIT_FAILURE;
    }

    git_reference_free(bHead);

    if (git_commit_lookup(&hCommit, bRepo, git_reference_oid(bRealHead)))
    {
        PRINT_ERROR("Could not load the commit.")
        git_repository_free(bRepo);
        return EXIT_FAILURE;
    }

    git_reference_free(bRealHead);

    if (git_commit_tree(&hTree, hCommit))
    {
        PRINT_ERROR("Could not load the main tree.")
        git_commit_free(hCommit);
        git_repository_free(bRepo);
        return EXIT_FAILURE;
    }

    git_commit_free(hCommit);

    if ((conf = git_tree_entry_byname(hTree, "gitorium.conf")) == NULL)
    {
        PRINT_ERROR("Could not load the repo configuration.")
        git_tree_free(hTree);
        git_repository_free(bRepo);
        return EXIT_FAILURE;
    }

    git_tree_free(hTree);

    if (git_blob_lookup(&blob, bRepo, git_tree_entry_id((const git_tree_entry*) conf)))
    {
        PRINT_ERROR("Could not load the repo configuration.")
        git_repository_free(bRepo);
        return EXIT_FAILURE;
    }

    *confOut = git_blob_rawcontent(blob);

    git_blob_free(blob);
    git_repository_free(bRepo);

    return 0;
}

int repo_update(void)
{
    config_t cfg;
    config_setting_t *setting;
    char *confData;
    FILE *temp;

    if (repo__load_config(&confData))
    {
        PRINT_ERROR("Could not get repo configuration.")
        return EXIT_FAILURE;
    }

    config_init(&cfg);

    // config_read_string is not in the version of libconfig available on ubuntu 12.04 LTS, so we emulate
    temp = fmemopen(confData, strlen(confData), "r");
    if (!config_read(&cfg, temp))
    {
        PRINT_ERROR("Could not read the repo configuration.")
        config_destroy(&cfg);
        return EXIT_FAILURE;
    }

    fclose(temp);

    if ((setting = config_lookup(&cfg, "repositories")) == NULL)
    {
        PRINT_ERROR("Could not load the repositories.")
        config_destroy(&cfg);
        return EXIT_FAILURE;
    }

    int count = config_setting_length(setting);

    for (int i = 0; i < count; i++)
    {
        repo__create(config_setting_get_elem(setting, i));
    }

    config_destroy(&cfg);

    return 0;
}
