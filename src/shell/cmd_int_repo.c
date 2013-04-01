#include "cmd_int_repo.h"

int cmd_int_fsck(char *user, char **args)
{
	error("THIS COMMAND HAS NOT YET BEEN IMPLEMENTED.");
	return -1;
}

int cmd_int_fsck_help(char *user, char **args)
{
	puts("fsck [--all|<repo> [...]]\n"
		"\n"
		"Runs a git fsck on the specified repositories or all.");
	return 0;
}
