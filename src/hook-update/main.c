#include "main.h"

int main(int argc, char **argv)
{
    /* This program will have to:
     *  * read main configuration
     *  * read repo configuration from admin repoS
     *  * run each hook for the current repo
     */
    argv++;
    argc--;

    int exit = EXIT_FAILURE;

    gitorium__config_init();

    // We receive arguments from git as such: name old_oid new_oid
    if (argc != 3)
    {
        PRINT_ERROR("We didn't start as an update hook!")
        gitorium__config_close();
        return EXIT_FAILURE;
    }

    gitorium__config_close();

    return exit;}
