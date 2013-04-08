#ifndef REPO_H_INCLUDED
#define REPO_H_INCLUDED

#include "common.h"

#include <git2.h>
#include <unistd.h>

#include "gitio.h"

int     repo_create     (char *name);
char   *repo_massage    (char *orig);
void    repo_list_refs  (git_repository **repo);

#endif // REPO_H_INCLUDED
