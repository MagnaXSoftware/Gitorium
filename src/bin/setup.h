#ifndef SETUP_H_INCLUDED
#define SETUP_H_INCLUDED

#include <git2.h>
#include <libconfig.h>
#include <sys/stat.h>
#include <string.h>

int cmd_setup(int argc, char **argv);
int cmd_setup_help(int argc, char **argv);

static int setup_admin_repo(char *pubkey);

#endif // SETUP_H_INCLUDED
