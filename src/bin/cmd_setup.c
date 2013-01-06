#include "cmd_setup.h"

static void setup__generate_conf(char **fullconf, char *user)
{
    char *parts[3] =
    {
        "/* Groups must start by a '*' character\n"
        " * \n"
        " * The groups are defined as a key value pair, one per line.\n"
        " * The key is the name of the group, while the value is a bracket enclosed\n"
        " * comma separated list of users and/or groups.\n"
        " * \n"
        " *     groups:\n"
        " *     {\n"
        " *         *group2 = [\"user1\", \"user2\", \"*group1\"]\n"
        " *     }\n"
        " */\n"
        "groups:\n"
        "{\n"
        "    *admins = [\"",
        "\"];\n"
        "};\n"
        "\n"
        "/* Repositories \n"
        " * \n"
        " * Repositories are defined inside a {} pair.\n"
        " * Each repository must have at least a name key. If there is no name key,\n"
        " * that repository entry is considered non-valid.\n"
        " * Repositories do not need to contain a perms key, but if they don't, then\n"
        " * no one will have access to that repository.\n"
        " * \n"
        " *     repositories:\n"
        " *     (\n"
        " *         {\n"
        " *             name = \"repo1\";\n"
        " *             perms = {\n"
        " *                 user1   = \"RW\"\n"
        " *                 *group1 = \"R\"\n"
        " *             }\n"
        " *         },\n"
        " *         {\n"
        " *             name = \"repo2\";\n"
        " *             perms = {\n"
        " *                 user1   = \"R\"\n"
        " *                 *group2 = \"RW\"\n"
        " *             }\n"
        " *         }\n"
        " *     )\n"
        " */\n"
        "repositories:\n"
        "(\n"
        "    {\n"
        "        name = \"gitorium-admin\";\n"
        "        perms = {\n"
        "            ",
        " = \"RW\"\n"
        "            *admins = \"RW\"\n"
        "        }\n"
        "    }\n"
        ");\n"
    };

    *fullconf = malloc(sizeof(char)*(strlen(parts[0])+1));
    strcpy(*fullconf, parts[0]);

    for(unsigned int i = 1; i < ARRAY_SIZE(parts); i++)
    {
        *fullconf = realloc(*fullconf, sizeof(char)*(strlen(*fullconf)+strlen(user)+strlen(parts[i])+1));
        strcat(*fullconf, user);
        strcat(*fullconf, parts[i]);
    }
}

static int setup__admin_repo(char *pubkey, int force)
{
    git_repository *repo, *bRepo;
    git_remote *rRemote;
    git_signature *rAuthor;
    git_treebuilder *rBuilder, *rcBuilder;
    git_oid iOid;
    git_tree *rTree;

    FILE *pFile;
    char *buffer, *rFullpath, *rUrl, *conf,
        *user = malloc(sizeof(char)*(strlen(pubkey)+1));
    const char *rPath;
    int size, result;
    struct stat rStat;
    pid_t pID;

    config_lookup_string(&aCfg, "repositories", &rPath);

    rFullpath = malloc(sizeof(ADMIN_REPO) + sizeof(char)*(strlen(rPath)+1));
    strcat(strcpy(rFullpath, rPath), ADMIN_REPO);

    if (!stat(rFullpath, &rStat))
    {
        // admin repo exists
        if (!force)
        {
            error("Administration repo already exists. Use --force to overwrite it.")
            return EXIT_FAILURE;
        }
    }

    strcpy(user, pubkey);
    if (user[0] == '*')
    {
        error("Users cannot begin with *. Please rename the file.")
        return EXIT_FAILURE;
    }
    user = strtok(user, ".");
    setup__generate_conf(&conf, user);
    free(user);

    if ((pFile = fopen(pubkey, "r")))
    {
        if (stat(".gitorium-admin", &rStat))
            remove(".gitorium-admin");

        if (git_repository_init(&repo, ".gitorium-admin", 0))
        {
            error("Could initialize a new administration repository.")
            fclose(pFile);
            free(conf);
            return EXIT_FAILURE;
        }

        if (git_treebuilder_create(&rcBuilder, NULL))
        {
            error("Could not create keys tree builder.")
            git_treebuilder_free(rcBuilder);
            git_repository_free(repo);
            fclose(pFile);
            free(conf);
            return EXIT_FAILURE;
        }

        // Copying the key
        fseek(pFile , 0 , SEEK_END);
        size = ftell(pFile);
        rewind(pFile);

        buffer = malloc(sizeof(char) * size);
        if (buffer == NULL)
        {
            error("Could not copy the public key.")
            free(buffer);
            git_treebuilder_free(rcBuilder);
            git_repository_free(repo);
            fclose(pFile);
            free(conf);
            return EXIT_FAILURE;
        }

        result = fread(buffer, sizeof(char), size, pFile);
        if (result != size)
        {
            error("Could not copy the public key.")
            free(buffer);
            git_treebuilder_free(rcBuilder);
            git_repository_free(repo);
            fclose(pFile);
            free(conf);
            return EXIT_FAILURE;
        }

        fclose(pFile);

        if (git_blob_create_frombuffer(&iOid, repo, buffer, sizeof(char)*size))
        {
            error("Could not create the administrator's public key.")
            free(buffer);
            git_treebuilder_free(rcBuilder);
            git_repository_free(repo);
            free(conf);
            return EXIT_FAILURE;
        }

        free(buffer);

        if (git_treebuilder_insert(NULL, rcBuilder, pubkey, &iOid, 0100644))
        {
            error("Could not insert the administrator's public key.")
            git_treebuilder_free(rcBuilder);
            git_repository_free(repo);
            free(conf);
            return EXIT_FAILURE;
        }

        if (git_treebuilder_write(&iOid, repo, rcBuilder))
        {
            error("Could not write the tree to index.")
            git_treebuilder_free(rcBuilder);
            git_repository_free(repo);
            free(conf);
            return EXIT_FAILURE;
        }

        git_treebuilder_free(rcBuilder);

        if (git_treebuilder_create(&rBuilder, NULL))
        {
            error("Could not create a tree builder.")
            git_treebuilder_free(rBuilder);
            git_repository_free(repo);
            free(conf);
            return EXIT_FAILURE;
        }

        if (git_treebuilder_insert(NULL, rBuilder, "keys", &iOid, 040000))
        {
            error("Could not insert the keys tree.")
            git_treebuilder_free(rBuilder);
            git_repository_free(repo);
            free(conf);
            return EXIT_FAILURE;
        }

        if (git_blob_create_frombuffer(&iOid, repo, conf, sizeof(char)*strlen(conf)))
        {
            error("Could not create the administration file.")
            git_treebuilder_free(rBuilder);
            git_repository_free(repo);
            free(conf);
            return EXIT_FAILURE;
        }

        if (git_treebuilder_insert(NULL, rBuilder, "gitorium.conf", &iOid, 0100644))
        {
            error("Could not insert the administration file.")
            git_treebuilder_free(rBuilder);
            git_repository_free(repo);
            free(conf);
            return EXIT_FAILURE;
        }

        free(conf);

        if (git_treebuilder_write(&iOid, repo, rBuilder))
        {
            error("Could not write the tree to index.")
            git_treebuilder_free(rBuilder);
            git_repository_free(repo);
            return EXIT_FAILURE;
        }

        git_treebuilder_free(rBuilder);

        if (git_tree_lookup(&rTree, repo, &iOid))
        {
            error("Could not write the tree to index.")
            git_tree_free(rTree);
            git_repository_free(repo);
            return EXIT_FAILURE;
        }

        if (git_signature_now(&rAuthor, "Gitorium", "Gitorium@local"))
        {
            error("Could not create a commit author.")
            git_tree_free(rTree);
            git_repository_free(repo);
            return EXIT_FAILURE;
        }

        if (git_commit_create(&iOid, repo, "HEAD", rAuthor, rAuthor, NULL, "Initial Configuration", rTree, 0, NULL))
        {
            error("Could not create the commit.")
            git_signature_free(rAuthor);
            git_tree_free(rTree);
            git_repository_free(repo);
            return EXIT_FAILURE;
        }

        git_signature_free(rAuthor);
        git_tree_free(rTree);

        //push the commit to origin

        if (!stat(rFullpath, &rStat))
        {
            pID = fork();
            if (pID == 0)                // child
            {
                execlp("rm", "rm", "-rf", rFullpath);
                _exit(1);
            }
            else if (pID < 0)            // failed to fork
            {
                error("Failed to launch external program 'rm'.")
                free(rFullpath);
                git_repository_free(repo);
                return EXIT_FAILURE;
            }

            waitpid(pID, NULL, 0);
        }

        if (git_repository_init(&bRepo, rFullpath, 1))
        {
            error("Could not initialize the remote admin repository.")
            free(rFullpath);
            git_repository_free(repo);
            return EXIT_FAILURE;
        }

        git_repository_free(bRepo);

        char *hFullpath = malloc(sizeof(char) * (strlen(rFullpath) + strlen("/hooks/post-update") + 1));
        strcat(strcpy(hFullpath, rFullpath), "/hooks/post-update");
        symlink(CMAKE_INSTALL_PREFIX"/bin/gitorium-hook-admin", hFullpath);

        hFullpath = realloc(hFullpath, sizeof(char) * (strlen(rFullpath) + strlen("/hooks/update") + 1));
        strcat(strcpy(hFullpath, rFullpath), "/hooks/update");
        symlink(CMAKE_INSTALL_PREFIX"/bin/gitorium-hook-admin", hFullpath);

        free(hFullpath);

        rUrl = malloc(sizeof("file://") + sizeof(char)*(strlen(rFullpath) + 1));
        strcat(strcpy(rUrl, "file://"), rFullpath);
        free(rFullpath);

        if (git_remote_add(&rRemote, repo, "origin", rUrl))
        {
            error("Could not add the bare repository as remote.")
            git_remote_free(rRemote);
            free(rUrl);
            git_repository_free(repo);
            return EXIT_FAILURE;
        }

        free(rUrl);

        #ifndef _NO_GIT2_PUSH  // libgit2 cannot push repositories ATM, so we must fork and call git itself.
        if (git_remote_connect(rRemote, GIT_DIR_PUSH))
        {
            error("Could not push the repository.")
            git_remote_disconnect(rRemote);
            git_remote_free(rRemote);
            git_repository_free(repo);
            return EXIT_FAILURE;
        }

        git_remote_disconnect(rRemote);
        #else
        pID = fork();
        if (pID == 0)                // child
        {
            chdir(".gitorium-admin");
            execlp("git", "git", "push", "origin", "master");
            _exit(1);
        }
        else if (pID < 0)            // failed to fork
        {
            error("Failed to launch external program 'git push'.")
            git_remote_free(rRemote);
            git_repository_free(repo);
            return EXIT_FAILURE;
        }

        waitpid(pID, NULL, 0);
        #endif

        pID = fork();
        if (pID == 0)                // child
        {
            execlp("rm", "rm", "-rf", ".gitorium-admin");
            _exit(1);
        }
        else if (pID < 0)            // failed to fork
        {
            error("Failed to launch external program 'rm'.")
            error("Please remove the .gitorium-admin directory manually.")
            git_remote_free(rRemote);
            git_repository_free(repo);
            return EXIT_FAILURE;
        }

        waitpid(pID, NULL, 0);

        git_remote_free(rRemote);
        git_repository_free(repo);
        return 0;
    }
    else
    {
        errorf("The public key file '%s' doesn't exist.", pubkey)
        return EXIT_FAILURE;
    }

    return 0;
}

int cmd_setup(int argc, char **argv)
{
    // We remove the name of the executable from the list
    argv++;
    argc--;

    if (argc == 0)
    {
        cmd_setup_help(argc, argv);
        return EXIT_FAILURE;
    }

    int result;

    if (!strcmp("--force", argv[0]))
    {
        result = setup__admin_repo(argv[1], 1);
    }
    else
    {
        result = setup__admin_repo(argv[0], 0);
    }

    return result;
}

int cmd_setup_help(int argc, char **argv)
{
    puts("gitorium setup <pubkey>\n"
         "\n"
         "Sets up gitorium for the current user.\n"
         "\n"
         "<pubkey> is the path to the administrator's public key. It should be "
         "named after the user (user admin has a key named admin.pub). That "
         "user will be set up as the initial administrator.");
    return 0;
}