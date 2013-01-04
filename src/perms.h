#ifndef PERMS_H_INCLUDED
#define PERMS_H_INCLUDED

#include "../common.h"

#define PERM_NO_ACCESS  0
#define PERM_READ       (1 << 0)
#define PERM_WRITE      (1 << 1)
#define PERM_FORCE      (1 << 2)

int perms_compare(const int required, const char *givenPerms);
int perms_check(const config_setting_t *perms, const int required, const char *user, const config_setting_t *groups);

#endif // PERMS_H_INCLUDED
