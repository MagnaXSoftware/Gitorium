#ifndef REPO_H_INCLUDED
#define REPO_H_INCLUDED

#include "common.h"

#include <git2.h>
#include <unistd.h>

int repo_create(char *name);

#endif // REPO_H_INCLUDED
