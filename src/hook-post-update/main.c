#include "main.h"

int main(int argc, char **argv)
{
    /* This program will have to:
        * read main configuration
        * read repo configuration from admin repo
        * run each hook for the current repo
     */
    argv++;
    argc--;

    if (gitorium_config_init())
    {
        PRINT_ERROR("Could not initialize the configuration.")
        return EXIT_FAILURE;
    }

    gitorium_config_close();

    return 0;
}
