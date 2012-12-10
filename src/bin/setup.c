#include "common.h"
#include "setup.h"

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
    if (!setup_admin_repo(argv[0]))
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

static int setup_admin_repo(char *pubkey)
{
    git_repository *repo, *repo_bare;
    FILE *pubkey_file, *file;
    char *pubkey_path, *user = malloc(sizeof pubkey + 1);

    strcpy(user, pubkey);
    if (user[0] == '@')
    {
        PRINT_ERROR("Users cannot begin with @. Please rename the file.")
        return EXIT_FAILURE;
    }
    user = strtok(user, ".");

    if ((pubkey_file = fopen(pubkey, "r")))
    {
        if (!git_repository_init(&repo, ".gitorium-admin", 0))
        {
            mkdir(".gitorium-admin/conf", S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
            mkdir(".gitorium-admin/keys", S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);

            pubkey_path = malloc(sizeof(pubkey) + sizeof(".gitorium-admin/keys/") + 1);
            strcpy(pubkey_path, ".gitorium-admin/keys/");
            strcat(pubkey_path, pubkey);

            if((file = fopen(pubkey_path, "w")))
            {
                fseek(pubkey_file , 0 , SEEK_END);
                int size = ftell(pubkey_file);
                rewind(pubkey_file);

                char *buffer = malloc(sizeof(char) * size);
                if (buffer == NULL)
                {
                    fclose(file);
                    fclose(pubkey_file);
                    free(pubkey_path);
                    git_repository_free(repo);
                    return EXIT_FAILURE;
                }

                int result = fread(buffer, sizeof(char), size, pubkey_file);
                if (result != size)
                {
                    fclose(file);
                    fclose(pubkey_file);
                    free(pubkey_path);
                    git_repository_free(repo);
                    return EXIT_FAILURE;
                }

                fwrite(buffer, sizeof(char), size, file);

                fclose(file);
                fclose(pubkey_file);
                free(pubkey_path);
            }
            else
            {
                free(pubkey_path);
                git_repository_free(repo);
                fclose(pubkey_file);
                return EXIT_FAILURE;
            }

            if ((file = fopen(".gitorium-admin/conf/repos.cfg", "w")))
            {
                fprintf(file, "users:\n"
                        "[\n"
                        "   \"%s\"\n"
                        "];\n"
                        "\n"
                        "/* Groups differ from users in that they must start by a '*' character\n"
                        "   \n"
                        "   The groups are defined as a key value pair, one per line.\n"
                        "   The key is the name of the group, while the value is a bracket enclosed\n"
                        "   comma separated list of users and/or groups.\n"
                        "   \n"
                        "       groups:\n"
                        "       {\n"
                        "           *group2 = [\"user1\", \"user2\", \"*group1\"]\n"
                        "       }\n"
                        " */\n"
                        "groups:\n"
                        "{\n"
                        "   @admins = [\"%s\"];\n"
                        "};\n"
                        "\n"
                        "/* Repositories \n"
                        " */\n"
                        "repositories:\n"
                        "(\n"
                        "   {\n"
                        "       name = \"gitorium-admin\";\n"
                        "       perms = {\n"
                        "           %s = \"RW\"\n"
                        "       };\n"
                        "   };\n"
                        ");", user, user, user);
                free(user);
                fclose(file);
            }

            git_repository_free(repo);
        }
        else
        {
            fclose(pubkey_file);
            return EXIT_FAILURE;
        }
    }
    else
    {
        PRINTF_ERROR("The public key file '%s' doesn't exist.", pubkey)
        return EXIT_FAILURE;
    }

    return 0;
}
