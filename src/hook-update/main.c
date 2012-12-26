#include "main.h"

int main(int argc, char **argv)
{
    /* This program will have to:
     *  * read main configuration
     *  * read repo configuration from admin repo
     *  * determine if user can push to this repo
     *  * run each hook for the current repo
     */

    // We remove the name of the executable from the list
    argv++;
    argc--;

    if (gitorium_config_init())
    {
        PRINT_ERROR("Could not initialize the configuration.")
        return EXIT_FAILURE;
    }

    // We receive arguments from git as such: name old_oid new_oid
    if (argc != 3)
    {
        PRINT_ERROR("We didn't start as an update hook!")
        return EXIT_FAILURE;
    }

    return 0;
}
