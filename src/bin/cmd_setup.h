#ifndef CMD_SETUP_H_INCLUDED
#define CMD_SETUP_H_INCLUDED

#define _NO_GIT2_PUSH

#include "../common.h"

#include <git2.h>
#include <libconfig.h>

int cmd_setup(int argc, char **argv);
int cmd_setup_help(int argc, char **argv);

#endif // CMD_SETUP_H_INCLUDED
