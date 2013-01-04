#include "perms.h"

int perms_compare(const int required, const char *givenPerms)
{
    int permCode = PERM_NO_ACCESS;
    for (unsigned int i = 0; i < strlen(givenPerms); i++)
    {
        if ('R' == givenPerms[i])
            permCode |= PERM_READ;
        if ('W' == givenPerms[i])
            permCode |= PERM_WRITE;
        if ('F' == givenPerms[i])
            permCode |= PERM_FORCE;
    }
    return !((permCode & required) == required);
}

int perms_check(const config_setting_t *perms, const int required, const char *user, const config_setting_t *groups)
{
    char *givenPerms;

    if (CONFIG_TRUE == config_setting_lookup_string(perms, user, (const char **) &givenPerms))
        return perms_compare(required, (const char *) givenPerms);

    for (int i = 0; i < config_setting_length(perms); i++)
    {
        char *group = (char *) config_setting_name(config_setting_get_elem(perms, i));

        if ('*' != group[0])
            continue;

        if (!strcmp(group, "*all"))
            continue;

        config_setting_t *members = config_setting_get_member(groups, (const char*) group);
        int out = 1;

        for (int j = 0; j < config_setting_length(members); j++)
        {
            if (!strcmp(config_setting_get_string_elem(members, j), user))
            {
                out = 0;
                break;
            }
        }

        if (out)
        	continue;

        if (CONFIG_TRUE == config_setting_lookup_string(perms, group, (const char **) &givenPerms))
            return perms_compare(required, (const char *) givenPerms);
    }

    if (CONFIG_TRUE == config_setting_lookup_string(perms, "*all", (const char **) &givenPerms))
        return perms_compare(required, (const char *) givenPerms);

    return EXIT_FAILURE;
}
