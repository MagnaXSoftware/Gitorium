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

int perms_check(const config_setting_t *perms, 
	const int required, 
	const char *user, 
	const config_setting_t *groups)
{
	char *givenPerms;

	// User is given specific permissions
	if (CONFIG_TRUE == config_setting_lookup_string(perms, user, (const char **) &givenPerms))
		return perms_compare(required, (const char *) givenPerms);

	// Check all groups
	for (int i = 0; i < config_setting_length(perms); i++)
	{
		const char *group = (const char *) config_setting_name(config_setting_get_elem(perms, i));

		if ('*' != group[0])
			continue;

		if (!strcmp(group, "*all")) // this is a special case
			continue;

		config_setting_t *members = config_setting_get_member(groups, group);

		// Check all members of the group
		for (int j = 0; j < config_setting_length(members); j++)
		{
			// User is a member of the group
			if (!strcmp(config_setting_get_string_elem(members, j), user))
			{
				if (CONFIG_TRUE == config_setting_lookup_string(perms, group, (const char **) &givenPerms))
					return perms_compare(required, (const char *) givenPerms);
			}
		}
	}

	// Is there a catch-all group?
	if (CONFIG_TRUE == config_setting_lookup_string(perms, "*all", (const char **) &givenPerms))
		return perms_compare(required, (const char *) givenPerms);

	return GITORIUM_ERROR;
}
