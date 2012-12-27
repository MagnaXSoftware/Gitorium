#ifndef SETUP_H_INCLUDED
#define SETUP_H_INCLUDED

#define _NO_GIT2_PUSH

#include "../common.h"

#include <git2.h>
#include <libconfig.h>
#include <sys/stat.h>
#include <unistd.h>

int cmd_setup(int argc, char **argv);
int cmd_setup_help(int argc, char **argv);

#endif // SETUP_H_INCLUDED
