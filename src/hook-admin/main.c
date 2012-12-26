#include "main.h"

int main(int argc, char **argv)
{
    if (!strcmp("hooks/post-update", argv[0]))
    {
        puts("post_update mode!");
        return ssh_setup();
    }

    for (int i = 0; i < argc; i++)
    {
        puts(argv[i]);
    }
    return 0;
}
