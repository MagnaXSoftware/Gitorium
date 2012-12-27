#include "main.h"

int main(int argc, char **argv)
{
    if (!strcmp("hooks/post-update", argv[0]))
    {
        // No error handling here because we must run the whole thing no matter what
        gitorium_config_init();
        ssh_setup();
        repo_update();
        gitorium_config_close();

        return 0;
    }
    else if (!strcmp("hooks/update", argv[0]))
    {
        return 0;
    }

    return 0;
}
