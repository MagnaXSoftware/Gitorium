#ifndef SSH_H_INCLUDED
#define SSH_H_INCLUDED

#include <string.h>
#include <libconfig.h>
#include <git2.h>
#include <sys/stat.h>

#include "../common.h"

int ssh_setup(void);

#endif // SSH_H_INCLUDED
