#include "main.h"

static int check_ref(char *ref, char *newid)
{
    if (strcmp("refs/heads/master", ref))
        return 0;

    if (!strcmp("0000000000000000000000000000000000000000", newid))
        return EXIT_FAILURE;

    return 0;
}

int main(int argc, char **argv)
{
    int exit = EXIT_FAILURE;

    gitorium_config_init();

    if (!strcmp("hooks/post-update", argv[0]))
    {
        // No error handling here because we must run the whole thing no matter what
        for (int i = 1; i < argc; i++)
        {
            if (!strcmp("refs/heads/master", argv[i]))
            {
                ssh_setup();
                repo_update();
            }
        }
        exit = 0;
    }
    else if (!strcmp("hooks/update", argv[0]))
    {
        exit = check_ref(argv[1], argv[3]);
    }

    gitorium_config_close();

    return exit;
}
