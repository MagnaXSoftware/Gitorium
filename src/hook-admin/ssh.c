#include "ssh.h"

static int ssh__reset(void)
{
    FILE *file;
    char *path;

    config_lookup_string(&aCfg, "keyfile", (const char **)&path);

    if ((file = fopen(path, "w")) == NULL)
    {
        PRINT_ERROR("Could not reset authorized keys.")
        return EXIT_FAILURE;
    }

    fclose(file);
    return 0;
}

static int ssh__add(const char *root, git_tree_entry *entry, void *payload)
{
    if (strcmp("keys/", root))
        return 0; // For some reason libgit2 walks from the root tree instead of our subtree

    FILE *auth;
    git_blob *blob;
    char *path, *name = (char *) git_tree_entry_name(entry);
    name = strtok(name, ".");

    config_lookup_string(&aCfg, "keyfile", (const char **)&path);

    printf("Adding user %s\n", name);

    if (git_blob_lookup(&blob, payload, git_tree_entry_id((const git_tree_entry*) entry)))
    {
        PRINT_ERROR("Could not load the key.")
        return 0;
    }

    if ((auth = fopen(path, "a")) == NULL)
    {
        PRINT_ERROR("Could not open authorized keys file.")
        return 0;
    }

    fprintf(auth, "command=\""CMAKE_INSTALL_PREFIX"/bin/gitorium-shell %s\",no-port-forwarding,no-X11-forwarding,no-agent-forwarding %s\n", name, (char *) git_blob_rawcontent(blob));
    fclose(auth);

    git_blob_free(blob);

    return 0;
}

int ssh_setup(void)
{
    git_repository *bRepo;
    git_reference *bHead, *bRealHead;
    git_commit *hCommit;
    git_tree *hTree, *kTree;

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

    if (git_tree_get_subtree(&kTree, hTree, "keys"))
    {
        PRINT_ERROR("Could not find the \"keys\" subtree in the main tree.")
        git_tree_free(hTree);
        git_commit_free(hCommit);
        git_repository_free(bRepo);
        return EXIT_FAILURE;
    }

    git_tree_free(hTree);
    git_commit_free(hCommit);

    if (ssh__reset())
    {
        git_tree_free(kTree);
        git_tree_free(hTree);
        git_commit_free(hCommit);
        git_repository_free(bRepo);
        return EXIT_FAILURE;
    }

    git_tree_walk(kTree, ssh__add, GIT_TREEWALK_POST, bRepo);

    git_tree_free(kTree);
    git_repository_free(bRepo);

    return 0;
}
