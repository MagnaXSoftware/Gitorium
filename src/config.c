#include "config.h"

int gitorium_config_close(void)
{
    config_destroy(&aCfg);
    return 0;
}

int gitorium_config_save(void)
{
    char *rcfile = malloc(sizeof(RC_FILE) + sizeof(char) * (strlen(getenv("HOME")) + 2));
    strcat(strcpy(rcfile, getenv("HOME")), "/"RC_FILE);

    if(!config_write_file(&aCfg, rcfile))
    {
        PRINT_ERROR("Could not save configuration file.")
        gitorium_config_close();
        free(rcfile);
        return EXIT_FAILURE;
    }

    free(rcfile);
    return 0;
}

int gitorium_config_init(void)
{
    char *rcfile = malloc(sizeof(RC_FILE) + sizeof(char) * (strlen(getenv("HOME")) + 2));
    strcat(strcpy(rcfile, getenv("HOME")), "/"RC_FILE);

    config_init(&aCfg);

    if(!config_read_file(&aCfg, rcfile))
    {
        config_setting_t *root, *setting;

        free(rcfile);

        root = config_root_setting(&aCfg);

        setting = config_setting_add(root, "repositories", CONFIG_TYPE_STRING);
        config_setting_set_string(setting, "/var/repositories/");

        if(!gitorium_config_save())
            return EXIT_FAILURE;
    }

    free(rcfile);
    return 0;
}
