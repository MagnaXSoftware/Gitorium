#include "config.h"

void gitorium__config_close(void)
{
    config_destroy(&aCfg);
}

void gitorium__config_init(void)
{
    config_init(&aCfg);

    if(!config_read_file(&aCfg, RC_FILE))
    {
        config_setting_t *root, *setting;

        root = config_root_setting(&aCfg);

        setting = config_setting_add(root, "repositories", CONFIG_TYPE_STRING);
        config_setting_set_string(setting, "/var/repositories/");
    }
}

int gitorium__repo_config_load(config_t *cfg)
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

    if ((conf = (git_tree_entry *) git_tree_entry_byname(hTree, "gitorium.conf")) == NULL)
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

    if (!config_read_string(cfg, (const char *) git_blob_rawcontent(blob)))
    {
        PRINT_ERROR("Could not read the repo configuration.")
        PRINTF_FATAL("%s on line %i of file gitorium.conf", config_error_text(cfg), config_error_line(cfg))
        git_blob_free(blob);
        git_repository_free(bRepo);
        return EXIT_FAILURE;
    }

    git_blob_free(blob);
    git_repository_free(bRepo);

    return 0;
}
