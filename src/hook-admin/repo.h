#ifndef REPO_H_INCLUDED
#define REPO_H_INCLUDED

#include "../common.h"

#include <libconfig.h>
#include <git2.h>
#include <sys/stat.h>

int repo_update(void);

#endif // REPO_H_INCLUDED
