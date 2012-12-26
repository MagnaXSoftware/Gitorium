#include "helper_ssh.h"

static char** list_keys()
{
    return NULL;
}

int cmd_helper_ssh(int argc, char **argv)
{
    // We remove the name of the executable from the list
    argv++;
    argc--;

    if (argc == 0)
    {
        cmd_helper_ssh_help(argc, argv);
        return EXIT_FAILURE;
    }

    return EXIT_FAILURE;
}

int cmd_helper_ssh_help(int argc, char **argv)
{
    puts("gitorium helper-ssh\n"
         "\n"
         "Generates a authorized_keys files from the commited keys.");
    return 0;
}
