#include "main.h"

int main(int argc, char **argv)
{
    if (!strcmp("hooks/post-update", argv[0]))
    {
        puts("post_update mode!");

        gitorium_config_init();
        if (ssh_setup())
            return EXIT_FAILURE;

        return 0;
    }
    else if (!strcmp("hooks/update", argv[0]))
    {
        puts("update mode");
        return 0;
    }

    return 0;
}
