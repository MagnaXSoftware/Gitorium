#include "setup.h"

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
        "   *admins = [\"",
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
        " *             };\n"
        " *         }\n"
        " *     )\n"
        " */\n"
        "repositories:\n"
        "(\n"
        "   {\n"
        "       name = \"gitorium-admin\";\n"
        "       perms = {\n"
        "           ",
        " = \"RW\"\n"
        "       };\n"
        "   }\n"
        ");"
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

static int setup__admin_repo(char *pubkey)
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

    strcpy(user, pubkey);
    if (user[0] == '*')
    {
        PRINT_ERROR("Users cannot begin with *. Please rename the file.")
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
            PRINT_ERROR("Could initialize a new administration repository.")
            fclose(pFile);
            free(conf);
            return EXIT_FAILURE;
        }

        if (git_treebuilder_create(&rcBuilder, NULL))
        {
            PRINT_ERROR("Could not create keys tree builder.")
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
            PRINT_ERROR("Could not copy the public key.")
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
            PRINT_ERROR("Could not copy the public key.")
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
            PRINT_ERROR("Could not create the administrator's public key.")
            free(buffer);
            git_treebuilder_free(rcBuilder);
            git_repository_free(repo);
            free(conf);
            return EXIT_FAILURE;
        }

        free(buffer);

        if (git_treebuilder_insert(NULL, rcBuilder, pubkey, &iOid, 0100644))
        {
            PRINT_ERROR("Could not insert the administrator's public key.")
            git_treebuilder_free(rcBuilder);
            git_repository_free(repo);
            free(conf);
            return EXIT_FAILURE;
        }

        if (git_treebuilder_write(&iOid, repo, rcBuilder))
        {
            PRINT_ERROR("Could not write the tree to index.")
            git_treebuilder_free(rcBuilder);
            git_repository_free(repo);
            free(conf);
            return EXIT_FAILURE;
        }

        git_treebuilder_free(rcBuilder);

        if (git_treebuilder_create(&rBuilder, NULL))
        {
            PRINT_ERROR("Could not create a tree builder.")
            git_treebuilder_free(rBuilder);
            git_repository_free(repo);
            free(conf);
            return EXIT_FAILURE;
        }

        if (git_treebuilder_insert(NULL, rBuilder, "keys", &iOid, 040000))
        {
            PRINT_ERROR("Could not insert the keys tree.")
            git_treebuilder_free(rBuilder);
            git_repository_free(repo);
            free(conf);
            return EXIT_FAILURE;
        }

        if (git_blob_create_frombuffer(&iOid, repo, conf, sizeof(char)*strlen(conf)))
        {
            PRINT_ERROR("Could not create the administration file.")
            git_treebuilder_free(rBuilder);
            git_repository_free(repo);
            free(conf);
            return EXIT_FAILURE;
        }

        if (git_treebuilder_insert(NULL, rBuilder, "gitorium.conf", &iOid, 0100644))
        {
            PRINT_ERROR("Could not insert the administration file.")
            git_treebuilder_free(rBuilder);
            git_repository_free(repo);
            free(conf);
            return EXIT_FAILURE;
        }

        free(conf);

        if (git_treebuilder_write(&iOid, repo, rBuilder))
        {
            PRINT_ERROR("Could not write the tree to index.")
            git_treebuilder_free(rBuilder);
            git_repository_free(repo);
            return EXIT_FAILURE;
        }

        git_treebuilder_free(rBuilder);

        if (git_tree_lookup(&rTree, repo, &iOid))
        {
            PRINT_ERROR("Could not write the tree to index.")
            git_tree_free(rTree);
            git_repository_free(repo);
            return EXIT_FAILURE;
        }

        if (git_signature_now(&rAuthor, "Gitorium", "Gitorium@local"))
        {
            PRINT_ERROR("Could not create a commit author.")
            git_tree_free(rTree);
            git_repository_free(repo);
            return EXIT_FAILURE;
        }

        if (git_commit_create(&iOid, repo, "HEAD", rAuthor, rAuthor, NULL, "Initial Configuration", rTree, 0, NULL))
        {
            PRINT_ERROR("Could not create the commit.")
            git_signature_free(rAuthor);
            git_tree_free(rTree);
            git_repository_free(repo);
            return EXIT_FAILURE;
        }

        git_signature_free(rAuthor);
        git_tree_free(rTree);

        //push the commit to origin

        config_lookup_string(&aCfg, "repositories", &rPath);

        rFullpath = malloc(sizeof("gitorium-admin.git") + sizeof(char)*(strlen(rPath)+1));
        strcat(strcpy(rFullpath, rPath), "gitorium-admin.git");

        if (git_repository_init(&bRepo, rFullpath, 1))
        {
            PRINT_ERROR("Could not initialize the remote admin repository.")
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
            PRINT_ERROR("Could not add the bare repository as remote.")
            git_remote_free(rRemote);
            free(rUrl);
            git_repository_free(repo);
            return EXIT_FAILURE;
        }

        free(rUrl);

        #ifdef PUSH
        #ifndef _NO_GIT2_PUSH
        if (git_remote_connect(rRemote, GIT_DIR_PUSH))
        {
            PRINT_ERROR("Could not propagate the repository.")
            const git_error *err = giterr_last();
            fprintf(stderr, "%s\n", err->message);
            git_remote_disconnect(rRemote);
            git_remote_free(rRemote);
            git_repository_free(repo);
            return EXIT_FAILURE;
        }
        #else
        chdir(".gitorium-admin");
        system("git push origin master");
        chdir("..");
        #endif

        system("rm -rf .gitorium-admin/");
        #endif

        git_remote_disconnect(rRemote);
        git_remote_free(rRemote);
        git_repository_free(repo);
        return 0;
    }
    else
    {
        PRINTF_ERROR("The public key file '%s' doesn't exist.", pubkey)
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

    if (!setup__admin_repo(argv[0]))
    {
        return 0;
    }
    else
        PRINT_ERROR("Could not initialize the admin repository.");

    return EXIT_FAILURE;
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
